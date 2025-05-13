#pragma once
#define FUSE_USE_VERSION 30

#include <Models/FileObjects.hpp>
#include <fuse3/fuse.h>

namespace fusevfs {

class ReadDirectory {
protected:
    const std::filesystem::path dirPath;
    void* Buffer = nullptr;
    fuse_fill_dir_t dirFiller = nullptr;
    off_t startCookie;
    bool wantPlus;
public:
    ReadDirectory(std::filesystem::path path, void* buffer, fuse_fill_dir_t filler, off_t offset,
                  enum fuse_readdir_flags flags);
    void operator()();

protected:
    void ReadDir(const std::shared_ptr<read_write_lock::RWLock<Directory>>& var);
    void ReadDir(const std::shared_ptr<read_write_lock::RWLock<RegularFile>>& var);
    void ReadDir(const std::shared_ptr<read_write_lock::RWLock<Link>>& var);

    void FillDirectory(const std::shared_ptr<read_write_lock::RWLock<Directory>>& dir);
};

}

