#include <iostream>
#include <Controllers/ReadDirectory.hpp>
#include <Controllers/GetFileParameter.hpp>
#include <Controllers/FindFile.hpp>
#include <Exceptions/FSException.hpp>
#include <Controllers/FileAttributes.hpp>

namespace fusevfs {

    ReadDirectory::ReadDirectory(std::filesystem::path p,
                  void* buf,
                  fuse_fill_dir_t fill,
                  off_t offset,
                  enum fuse_readdir_flags flags)
        : dirPath{std::move(p)}
    , Buffer{buf}
    , dirFiller{fill}
    , startCookie{offset}
    , wantPlus{static_cast<bool>(flags & FUSE_READDIR_PLUS)} {}

    void ReadDirectory::operator()() {
        std::visit([this](const auto& obj){ ReadDir(obj); }, FindFile::Find(dirPath));
    }

    void ReadDirectory::ReadDir(const std::shared_ptr<read_write_lock::RWLock<Directory>>& var) {
        FillDirectory(var);
    }

    void ReadDirectory::ReadDir(const std::shared_ptr<read_write_lock::RWLock<RegularFile>>& var) {
        throw FSException(dirPath, ExceptionTypeEnum::NotDirectory);
    }

    void ReadDirectory::ReadDir(const std::shared_ptr<read_write_lock::RWLock<Link>>& var) {
        const auto varRead = var->Read();
        const auto dir = FindFile::FindDir(varRead->LinkTo);
        FillDirectory(dir);
    }

    void ReadDirectory::FillDirectory(const std::shared_ptr<read_write_lock::RWLock<Directory>>& dir) {
        const auto dirRead = dir->Read();
        off_t idx = 0;

        if (startCookie <= idx) {
            dirFiller(Buffer,
                      ".",           // имя
                      nullptr,       // атрибуты (NULL → пусть FUSE добавит default stat)
                      ++idx,         // cookie для следующего вызова
                      wantPlus ? FUSE_FILL_DIR_PLUS : (fuse_fill_dir_flags)0);
        } else {
            ++idx;
        }

        if (startCookie <= idx) {
            dirFiller(Buffer,
                      "..",
                      nullptr,
                      ++idx,
                      wantPlus ? FUSE_FILL_DIR_PLUS : (fuse_fill_dir_flags)0);
        } else {
            ++idx;
        }

        for (const auto& var : dirRead->Files) {
            if (startCookie > idx) {
                ++idx;
                continue;
            }

            const auto name = GetNameParameter{}(var);
            dirFiller(Buffer,
                      name.data(),
                      nullptr,
                      ++idx,
                      wantPlus ? FUSE_FILL_DIR_PLUS : (fuse_fill_dir_flags)0);
        }
    }

}
