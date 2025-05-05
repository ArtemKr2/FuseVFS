#pragma once
#define FUSE_USE_VERSION 30

#include <Models/TFileObjects.hpp>
#include <fuse3/fuse.h>
#include <string_view>

namespace fusevfs {

class TReadDirectory {
protected:
    const std::filesystem::path& m_pPath;
    void* m_pBuffer = nullptr;
    fuse_fill_dir_t m_xFiller = nullptr;
public:
    TReadDirectory(const std::filesystem::path& path, void* buffer, fuse_fill_dir_t filler);
    void operator()();

protected:
    void DoReadDir(const std::shared_ptr<rwl::TRwLock<TDirectory>>& var);
    void DoReadDir(const std::shared_ptr<rwl::TRwLock<TRegularFile>>& var);
    void DoReadDir(const std::shared_ptr<rwl::TRwLock<TLink>>& var);


    void FillerBuffer(const std::string_view& name);
    void FillerDirectory(const std::shared_ptr<rwl::TRwLock<TDirectory>>& dir);
};

}

