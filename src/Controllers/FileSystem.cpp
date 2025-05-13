#include <thread>
#include <cstring>
#include <span>
#include <fstream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <Controllers/FileSystem.hpp>
#include <Controllers/FindFile.hpp>
#include <Controllers/FileAttributes.hpp>
#include <Controllers/SetFileParameter.hpp>
#include <Controllers/ReadDirectory.hpp>
#include <Controllers/FSDeleteFile.hpp>
#include <Controllers/FSAccessFile.hpp>
#include <Exceptions/FSException.hpp>

using namespace std::chrono_literals;

namespace fusevfs {

    using Clock = std::chrono::system_clock;

    // std::filesystem::path TFileSystem::FifoPath = "";
    std::filesystem::path FileSystem::SocketPath = "";

    static constexpr std::string_view RootPath = "/";

    int FileSystem::Init(int argc, char *argv[]) {
        fuse_operations FSOps = {
            .getattr = GetAttr,
            .readlink = ReadLink,
            .mknod = MkNod,
            .mkdir = MkDir,
            .unlink = Unlink,
            .rmdir = RmDir,
            .symlink = SymLink,
            .chmod = ChMod,
            .truncate  = Truncate,
            .open = Open,
            .read = Read,
            .write = Write,
            .opendir = OpenDir,
            .readdir = ReadDir,
            .access = Access,
            .utimens = Utimens
        };

        int server_fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
        if (server_fd < 0) {
            perror("socket");
            return 1;
        }
        ::unlink(SocketPath.c_str());

        sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        std::strncpy(addr.sun_path, SocketPath.c_str(),
                     sizeof(addr.sun_path) - 1);
        if (bind(server_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
            perror("bind");
            return 1;
        }
        if (listen(server_fd, 5) < 0) {
            perror("listen");
            return 1;
        }
        // auto fifoCommunicationThread = std::jthread(FindByNameThread);
        std::thread(&FileSystem::ServerLoop, server_fd).detach();
        return fuse_main(argc, argv, &FSOps, nullptr);
    }

    void FileSystem::ServerLoop(int server_fd) {
        for (;;) {
            int client = ::accept(server_fd, nullptr, nullptr);
            if (client < 0) {
                std::this_thread::sleep_for(10ms);
                continue;
            }
            std::string name;
            char c;
            while (read(client, &c, 1) == 1 && c != '\n') {
                name.push_back(c);
            }
            std::string out;
            try {
                std::vector<std::filesystem::path> results;
                auto&& s = FindFile::FindByName(name);
                results.assign(s.begin(), s.end());
                for (auto& p : results)
                    out += p.native() + "\n";

            } catch(const FSException& ex) {
                out = "No files with such name\n";
            }
            ssize_t w = write(client, out.c_str(), out.size());
            if (w < 0) perror("write");
            close(client);
        }
    }

    int FileSystem::Utimens(const char*           path,
                             const struct timespec  tv[2],
                             struct fuse_file_info*)
    {
        try {
            auto var = FindFile::Find(path);  // FileObjectSharedVariant

            using clock = std::chrono::system_clock;
            auto now = clock::now();
            auto ts2tp = [&](const timespec& ts){
                return clock::from_time_t(ts.tv_sec)
                     + std::chrono::nanoseconds(ts.tv_nsec);
            };

            std::visit([&](auto& lock){
                auto wr = lock->Write();

                // ── atime ───────────────────────────
                if (tv[0].tv_nsec == UTIME_NOW) {
                    SetAccessedParameter{ now }(wr);
                }
                else if (tv[0].tv_nsec != UTIME_OMIT) {
                    SetAccessedParameter{ ts2tp(tv[0]) }(wr);
                }
                // ── mtime ───────────────────────────
                if (tv[1].tv_nsec == UTIME_NOW) {
                    SetModifiedParameter{ now }(wr);
                }
                else if (tv[1].tv_nsec != UTIME_OMIT) {
                    SetModifiedParameter{ ts2tp(tv[1]) }(wr);
                }
                // ── ctime ─────────────────────────── always
                SetChangedParameter{ clock::now() }(wr);
            }, var);

            return 0;
        }
        catch(const FSException& ex) {
            return ex.Type();
        }
    }


    int FileSystem::GetAttr(const char* path, struct stat* st, struct fuse_file_info* fi) {
        try {
            const auto result = FindFile::Find(path);
            FileAttributes::Get(result, st);
            return 0;
        } catch(const FSException& ex) {
            return ex.Type();
        }
    }

    int FileSystem::ReadLink(const char* path, char* buffer, size_t size) {
        try {
            const auto link = FindFile::FindLink(path);
            {
                auto wr = link->Write();
                SetAccessedParameter{ Clock::now() }( wr );
            }
            const auto linkRead = link->Read();
            const auto& pathView = linkRead->LinkTo.native();
            auto bufferSpan = std::span(buffer, size);
            std::fill(bufferSpan.begin(), bufferSpan.end(), 0);
            std::copy(pathView.begin(), pathView.end(), bufferSpan.begin());
            return 0;
        } catch(const FSException& ex) {
            return ex.Type();
        }
    }

    template<FileObjectConcept T, typename ...Args>
    int AddFile(const char* path, mode_t mode, Args&&... args) {
        const auto newPath = std::filesystem::path(path);
        const auto parentPath = newPath.parent_path();
        auto parentDir = FindFile::FindDir(parentPath);
        if(FSAccessFile::Access(parentDir, W_OK)==FileAccessType::Restricted) {
            return ExceptionTypeEnum::AccessNotPermitted;
        }
        T::New(newPath.filename(), mode, parentDir, args...);
        {
            auto pw  = parentDir->Write();                 // unique-lock
            auto now = std::chrono::system_clock::now();
            SetModifiedParameter{now}(pw);        // mtime
            SetChangedParameter{now}(pw);         // ctime
        }

        FindFile::AddNameToHash(newPath);
        return 0;
    }

    int FileSystem::MkNod(const char* path, mode_t mode, dev_t rdev) {
        try {
            return AddFile<RegularFile>(path, mode);
        } catch(const FSException& ex) {
            return ex.Type();
        }
    }

    int FileSystem::MkDir(const char* path, mode_t mode) {
        try {
            return AddFile<Directory>(path, mode);
        } catch(const FSException& ex) {
            return ex.Type();
        }
    }

    int FileSystem::Unlink(const char* path) {
        try {
            FSDeleteFile::Delete(path);
            auto parentDir = FindFile::FindDir(std::filesystem::path(path).parent_path());
            {
                auto pw  = parentDir->Write();                 // unique-lock
                auto now = std::chrono::system_clock::now();
                SetModifiedParameter{now}(pw);        // mtime
                SetChangedParameter{now}(pw);      // ctime
            }
            return 0;
        } catch(const FSException& ex) {
            return ex.Type();
        }
    }

    int FileSystem::RmDir(const char* path) {
        try {
            FSDeleteFile::Delete(path);
            auto parentDir = FindFile::FindDir(std::filesystem::path(path).parent_path());
            {
                auto pw  = parentDir->Write();                 // unique-lock
                auto now = std::chrono::system_clock::now();
                SetModifiedParameter{now}(pw);        // mtime
                SetChangedParameter{now}(pw);      // ctime
            }
            return 0;
        } catch(const FSException& ex) {
            return ex.Type();
        }
    }

    int FileSystem::SymLink(const char* target_path, const char* link_path) {
        try {
            return AddFile<Link>(link_path, 0777, target_path);

        } catch(const FSException& ex) {
            return ex.Type();
        }
    }

    int FileSystem::ChMod(const char* path, mode_t mode, struct fuse_file_info* fi) {
        try {
            auto var = FindFile::Find(path);

            mode_t raw_perms = mode & (S_IRWXU|S_IRWXG|S_IRWXO);

            SeModeParameter{ raw_perms }(var);
            SetChangedParameter{ Clock::now() }(var);

            return 0;
        } catch(const FSException& ex) {
            return ex.Type();
        }
    }

    int FileSystem::Open(const char* path, struct fuse_file_info* info) {
        try {
            const int rc = FSAccessFile::AccessWithFuseFlags(path, info->flags);
            if (rc != 0) {
                return rc;
            }
            if (IsHasFlag(info->flags, O_TRUNC)) {
                auto file = FindFile::FindRegularFile(path);
                auto wr   = file->Write();
                wr->Data.clear();
                auto now = std::chrono::system_clock::now();
                SetModifiedParameter{now}(wr);
                SetChangedParameter {now}(wr);
            }
            return 0;
        } catch(const FSException& ex) {
            return ex.Type();
        }
    }

    int FileSystem::Read(const char* path, char* buffer, size_t size, off_t offset, struct fuse_file_info* info) {
        try {
            auto file = FindFile::FindRegularFile(path);
            if(FSAccessFile::AccessWithFuseFlags(file, info->flags)==FileAccessType::Restricted) {
                return ExceptionTypeEnum::AccessNotPermitted;
            }
            {
                auto wr = file->Write();
                SetAccessedParameter{Clock::now()}(wr);
            }
            const auto fileRead = file->Read();
            const auto& data = fileRead->Data;
            const auto offsetSize = static_cast<size_t>(data.end() - (data.begin() + offset));
            const auto readSize = std::min(offsetSize, size);
            std::memcpy(buffer, fileRead->Data.data() + offset, readSize);
            return static_cast<int>(readSize);
        } catch(const FSException& ex) {
            return ex.Type();
        }
    }

    int FileSystem::Truncate(const char* path, off_t size, struct fuse_file_info* /*fi*/) {
        try {
            auto file = FindFile::FindRegularFile(path);
            auto wr   = file->Write();
            wr->Data.resize(static_cast<size_t>(size));
            auto now = std::chrono::system_clock::now();
            SetModifiedParameter{now}(wr);
            SetChangedParameter {now}(wr);
            return 0;
        } catch(const FSException& ex) {
            return ex.Type();
        }
    }

    template<std::integral T>
    constexpr bool IsHasFlag(T value, T flag) {
        return (value & flag) == flag;
    }

    int FileSystem::Write(const char* path, const char* buffer, size_t size, off_t offset, struct fuse_file_info* info) {
        try {
            auto file = FindFile::FindRegularFile(path);
            if(FSAccessFile::AccessWithFuseFlags(file, info->flags)==FileAccessType::Restricted) {
                return ExceptionTypeEnum::AccessNotPermitted;
            }
            auto wr = file->Write();
            auto& data = wr->Data;
            std::span src{buffer, size};

            if (IsHasFlag(info->flags, O_APPEND)) {
                data.insert(data.end(), src.begin(), src.end());
            } else {
                auto needed = offset + size;
                if (data.size() < needed) data.resize(needed);
                std::copy(src.begin(), src.end(), data.begin() + offset);
            }

            auto now = std::chrono::system_clock::now();
            SetModifiedParameter{now}(wr);
            SetChangedParameter {now}(wr);

            return static_cast<int>(size);
        } catch(const FSException& ex) {
            return ex.Type();
        }
    }

    int FileSystem::OpenDir(const char* path, struct fuse_file_info* info) {
        try {
            const int rc = FSAccessFile::AccessWithFuseFlags(path, info->flags);
            return rc;
        } catch(const FSException& ex) {
            return ex.Type();
        }
    }

    int FileSystem::ReadDir(const char* path, void* buffer, fuse_fill_dir_t filler, off_t offset,
        struct fuse_file_info* info, enum fuse_readdir_flags flags) {
        try {
            auto dir = FindFile::FindDir(path);
            SetAccessedParameter{ Clock::now() }( dir );

            ReadDirectory{ std::filesystem::path{path},
                           buffer,
                           filler,
                           offset,
                           flags }();

            return 0;
        } catch(const FSException& ex) {
            return ex.Type();
        }
    }

    int FileSystem::Access(const char* path, int accessMask) {
        try {
            int rc = FSAccessFile::Access(path, accessMask);
            return rc;
        } catch(const FSException& ex) {
            return ex.Type();
        }
    }

    const std::shared_ptr<read_write_lock::RWLock<Directory>>& FileSystem::RootDir() {
        static auto s_pRootDir = Directory::New(RootPath.data(), static_cast<mode_t>(0777), nullptr);
        return s_pRootDir;
    }

}
