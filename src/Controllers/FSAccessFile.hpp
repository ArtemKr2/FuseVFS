#pragma once

#include <Models/FileObjects.hpp>
#include <Models/FileAccess.hpp>

#include <filesystem>

namespace fusevfs::FSAccessFile {

    FileAccessType Access(const std::filesystem::path& path, const int accessMask);
    FileAccessType Access(const FileObjectSharedVariant& var, const int accessMask);
    FileAccessType Access(const std::shared_ptr<read_write_lock::RWLock<Link>>& var, const int accessMask);
    FileAccessType Access(const std::shared_ptr<read_write_lock::RWLock<RegularFile>>& var, const int accessMask);
    FileAccessType Access(const std::shared_ptr<read_write_lock::RWLock<Directory>>& var, const int accessMask);
    FileAccessType AccessWithFuseFlags(const std::filesystem::path& path, const int fuseFlags);
    FileAccessType AccessWithFuseFlags(const FileObjectSharedVariant& var, const int fuseFlags);
    FileAccessType AccessWithFuseFlags(const std::shared_ptr<read_write_lock::RWLock<RegularFile>>& var, const int fuseFlags);
    FileAccessType AccessWithFuseFlags(const std::shared_ptr<read_write_lock::RWLock<Link>>& var, const int fuseFlags);
    FileAccessType AccessWithFuseFlags(const std::shared_ptr<read_write_lock::RWLock<Directory>>& var, const int fuseFlags);

}

