#include <thread>
#include <cstring>
#include <span>
#include <fstream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <Controllers/TFileSystem.hpp>
#include <Controllers/NSFindFile.hpp>
#include <Controllers/NSFileAttributes.hpp>
#include <Controllers/TSetFileParameter.hpp>
#include <Controllers/TReadDirectory.hpp>
#include <Controllers/NSDeleteFile.hpp>
#include <Controllers/NSAccessFile.hpp>
#include <Exceptions/FSException.hpp>

using namespace std::chrono_literals;



namespace fusevfs {


    using Clock = std::chrono::system_clock;

    // std::filesystem::path TFileSystem::FifoPath = "";
    std::filesystem::path TFileSystem::SocketPath = "";

    static constexpr std::string_view s_sRootPath = "/";

    int TFileSystem::Init(int argc, char *argv[]) {
        fuse_operations FileSystemOperations = {
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
        std::thread(&TFileSystem::ServerLoop, server_fd).detach();
        return fuse_main(argc, argv, &FileSystemOperations, nullptr);
    }

    void TFileSystem::ServerLoop(int server_fd) {
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
                auto&& s = NSFindFile::FindByName(name);
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

    int TFileSystem::Utimens(const char*           path,
                             const struct timespec  tv[2],
                             struct fuse_file_info*)
    {
        try {
            auto var = NSFindFile::Find(path);  // FileObjectSharedVariant

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
                    TSetInfoAccessed{ now }(wr);
                }
                else if (tv[0].tv_nsec != UTIME_OMIT) {
                    TSetInfoAccessed{ ts2tp(tv[0]) }(wr);
                }
                // ── mtime ───────────────────────────
                if (tv[1].tv_nsec == UTIME_NOW) {
                    TSetInfoModified{ now }(wr);
                }
                else if (tv[1].tv_nsec != UTIME_OMIT) {
                    TSetInfoModified{ ts2tp(tv[1]) }(wr);
                }
                // ── ctime ─────────────────────────── always
                TSetInfoChanged{ clock::now() }(wr);
            }, var);

            return 0;
        }
        catch(const FSException& ex) {
            return ex.Type();
        }
    }


    int TFileSystem::GetAttr(const char* path, struct stat* st, struct fuse_file_info* fi) {
        try {
            const auto result = NSFindFile::Find(path);
            NSFileAttributes::Get(result, st);
            return 0;
        } catch(const FSException& ex) {
            return ex.Type();
        }
    }

    int TFileSystem::ReadLink(const char* path, char* buffer, size_t size) {
        try {
            const auto link = NSFindFile::FindLink(path);
            {
                auto wr = link->Write();
                TSetInfoAccessed{ Clock::now() }( wr );
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
        auto parentDir = NSFindFile::FindDir(parentPath);
        if(NSAccessFile::Access(parentDir, W_OK)==FileAccessType::Restricted) {
            return ExceptionTypeEnum::AccessNotPermitted;
        }
        T::New(newPath.filename(), mode, parentDir, args...);
        {
            auto pw  = parentDir->Write();                 // unique-lock
            auto now = std::chrono::system_clock::now();
            TSetInfoModified{now}(pw);        // mtime
            TSetInfoChanged{now}(pw);         // ctime
        }

        NSFindFile::AddToNameHash(newPath);
        return 0;
    }

    int TFileSystem::MkNod(const char* path, mode_t mode, dev_t rdev) {
        try {
            return AddFile<RegularFile>(path, mode);
        } catch(const FSException& ex) {
            return ex.Type();
        }
    }

    int TFileSystem::MkDir(const char* path, mode_t mode) {
        try {
            return AddFile<Directory>(path, mode);
        } catch(const FSException& ex) {
            return ex.Type();
        }
    }

    int TFileSystem::Unlink(const char* path) {
        try {
            NSDeleteFile::Delete(path);
            auto parentDir = NSFindFile::FindDir(std::filesystem::path(path).parent_path());
            {
                auto pw  = parentDir->Write();                 // unique-lock
                auto now = std::chrono::system_clock::now();
                TSetInfoModified{now}(pw);        // mtime
                TSetInfoChanged{now}(pw);      // ctime
            }
            return 0;
        } catch(const FSException& ex) {
            return ex.Type();
        }
    }

    int TFileSystem::RmDir(const char* path) {
        try {
            NSDeleteFile::Delete(path);
            auto parentDir = NSFindFile::FindDir(std::filesystem::path(path).parent_path());
            {
                auto pw  = parentDir->Write();                 // unique-lock
                auto now = std::chrono::system_clock::now();
                TSetInfoModified{now}(pw);        // mtime
                TSetInfoChanged{now}(pw);      // ctime
            }
            return 0;
        } catch(const FSException& ex) {
            return ex.Type();
        }
    }

    int TFileSystem::SymLink(const char* target_path, const char* link_path) {
        try {
            return AddFile<Link>(link_path, 0777, target_path);

        } catch(const FSException& ex) {
            return ex.Type();
        }
    }

    int TFileSystem::ChMod(const char* path, mode_t mode, struct fuse_file_info* fi) {
        try {
            auto var = NSFindFile::Find(path);

            mode_t raw_perms = mode & (S_IRWXU|S_IRWXG|S_IRWXO);

            TSetInfoMode{ raw_perms }(var);
            TSetInfoChanged{ Clock::now() }(var);

            return 0;
        } catch(const FSException& ex) {
            return ex.Type();
        }
    }

    int TFileSystem::Open(const char* path, struct fuse_file_info* info) {
        try {
            const int rc = NSAccessFile::AccessWithFuseFlags(path, info->flags);
            if (rc != 0) {
                return rc;
            }
            if (IsHasFlag(info->flags, O_TRUNC)) {
                auto file = NSFindFile::FindRegularFile(path);
                auto wr   = file->Write();
                wr->Data.clear();
                auto now = std::chrono::system_clock::now();
                TSetInfoModified{now}(wr);
                TSetInfoChanged {now}(wr);
            }
            return 0;
        } catch(const FSException& ex) {
            return ex.Type();
        }
    }

    int TFileSystem::Read(const char* path, char* buffer, size_t size, off_t offset, struct fuse_file_info* info) {
        try {
            auto file = NSFindFile::FindRegularFile(path);
            if(NSAccessFile::AccessWithFuseFlags(file, info->flags)==FileAccessType::Restricted) {
                return ExceptionTypeEnum::AccessNotPermitted;
            }
            {
                auto wr = file->Write();
                TSetInfoAccessed{Clock::now()}(wr);
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

    int TFileSystem::Truncate(const char* path, off_t size, struct fuse_file_info* /*fi*/) {
        try {
            auto file = NSFindFile::FindRegularFile(path);
            auto wr   = file->Write();
            wr->Data.resize(static_cast<size_t>(size));
            auto now = std::chrono::system_clock::now();
            TSetInfoModified{now}(wr);
            TSetInfoChanged {now}(wr);
            return 0;
        } catch(const FSException& ex) {
            return ex.Type();
        }
    }

    template<std::integral T>
    constexpr bool IsHasFlag(T value, T flag) {
        return (value & flag) == flag;
    }

    int TFileSystem::Write(const char* path, const char* buffer, size_t size, off_t offset, struct fuse_file_info* info) {
        try {
            auto file = NSFindFile::FindRegularFile(path);
            if(NSAccessFile::AccessWithFuseFlags(file, info->flags)==FileAccessType::Restricted) {
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
            TSetInfoModified{now}(wr);
            TSetInfoChanged {now}(wr);

            return static_cast<int>(size);
        } catch(const FSException& ex) {
            return ex.Type();
        }
    }

    int TFileSystem::OpenDir(const char* path, struct fuse_file_info* info) {
        try {
            const int rc = NSAccessFile::AccessWithFuseFlags(path, info->flags);
            return rc;
        } catch(const FSException& ex) {
            return ex.Type();
        }
    }

    int TFileSystem::ReadDir(const char* path, void* buffer, fuse_fill_dir_t filler, off_t offset,
        struct fuse_file_info* info, enum fuse_readdir_flags flags) {
        try {
            const auto dir = NSFindFile::FindDir(path);
            TSetInfoAccessed{ Clock::now() }( dir );
            TReadDirectory{path, buffer, filler}();
            return 0;
        } catch(const FSException& ex) {
            return ex.Type();
        }
    }

    int TFileSystem::Access(const char* path, int accessMask) {
        try {
            int rc = NSAccessFile::Access(path, accessMask);
            return rc;
        } catch(const FSException& ex) {
            return ex.Type();
        }
    }

    const std::shared_ptr<read_write_lock::RWLock<Directory>>& TFileSystem::RootDir() {
        static auto s_pRootDir = Directory::New(s_sRootPath.data(), static_cast<mode_t>(0777), nullptr);
        return s_pRootDir;
    }



    // void TFileSystem::FindByNameThread() {
    //     auto buffer = std::array<char, s_uCommunicationBufferSize>();
    //     while(true) {
    //         {
    //             std::cerr << "[FindByNameThread] before fIn = std::ifstream(pipePath);" << std::endl;
    //             auto fIn = std::ifstream(FifoPath);
    //             std::cerr << "[FindByNameThread] after fIn = std::ifstream(pipePath);" << std::endl;
    //             std::cerr << "[FindByNameThread] before !fIn.is_open();" << std::endl;
    //             if(!fIn.is_open()) {
    //                 continue;
    //             }
    //             std::cerr << "[FindByNameThread] after !fIn.is_open();" << std::endl;
    //             std::cerr << "[FindByNameThread] before fIn.read();" << std::endl;
    //             fIn.read(buffer.data(), buffer.size());
    //             std::cerr << "[FindByNameThread] after fIn.read();" << std::endl;
    //         }
    //         const auto path = std::string(buffer.data());
    //         try {
    //             std::cerr << "[FindByNameThread] before FindByName(path);" << std::endl;
    //             const auto& paths = NSFindFile::FindByName(path);
    //             std::cerr << "[FindByNameThread] after FindByName(path);" << std::endl;
    //             std::cerr << "[FindByNameThread] before fOut = std::ofstream();" << std::endl;
    //             auto fOut = std::ofstream(FifoPath);
    //             std::cerr << "[FindByNameThread] after fOut = std::ofstream();" << std::endl;
    //             std::cerr << "[FindByNameThread] before !fOut.is_open();" << std::endl;
    //             if(!fOut.is_open()) {
    //                 continue;
    //             }
    //             std::cerr << "[FindByNameThread] after !fOut.is_open();" << std::endl;
    //             std::cerr << "[FindByNameThread] before cycle for fOut << p.native()" << std::endl;
    //             for(const auto& p : paths) {
    //                 fOut << p.native() << "\n";
    //             }
    //             std::cerr << "[FindByNameThread] after cycle for fOut << p.native()" << std::endl;
    //         } catch(const TFSException& ex) {
    //             std::cerr << "[FindByNameThread] exception caught" << std::endl;
    //             auto fOut = std::ofstream(FifoPath);
    //             if(fOut.is_open()) {
    //                 fOut << s_sNoFilesWithSuchName;
    //             }
    //             std::cerr << "[FindByNameThread] after exception caught" << std::endl;
    //         }
    //     }
    // }


}
