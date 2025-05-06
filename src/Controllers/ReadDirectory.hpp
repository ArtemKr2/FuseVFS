#pragma once
#define FUSE_USE_VERSION 30

#include <Models/FileObjects.hpp>
#include <fuse3/fuse.h>
#include <string_view>

namespace fusevfs {

class ReadDirectory {
protected:
    const std::filesystem::path& m_pPath;
    void* m_pBuffer = nullptr;
    fuse_fill_dir_t m_xFiller = nullptr;
public:
    ReadDirectory(const std::filesystem::path& path, void* buffer, fuse_fill_dir_t filler);
    void operator()();

protected:
    void DoReadDir(const std::shared_ptr<read_write_lock::RWLock<Directory>>& var);
    void DoReadDir(const std::shared_ptr<read_write_lock::RWLock<RegularFile>>& var);
    void DoReadDir(const std::shared_ptr<read_write_lock::RWLock<Link>>& var);


    void FillerBuffer(const std::string_view& name);
    void FillerDirectory(const std::shared_ptr<read_write_lock::RWLock<Directory>>& dir);
};

}

