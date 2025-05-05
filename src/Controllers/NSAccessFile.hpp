#pragma once

#include <Models/TFileObjects.hpp>
#include <Models/NNFileAccess.hpp>

#include <filesystem>

namespace fusevfs::NSAccessFile {

    NFileAccess Access(const std::filesystem::path& path, const int accessMask);
    NFileAccess Access(const ASharedFileVariant& var, const int accessMask);
    NFileAccess Access(const std::shared_ptr<rwl::TRwLock<TLink>>& var, const int accessMask);
    NFileAccess Access(const std::shared_ptr<rwl::TRwLock<TRegularFile>>& var, const int accessMask);
    NFileAccess Access(const std::shared_ptr<rwl::TRwLock<TDirectory>>& var, const int accessMask);
    NFileAccess AccessWithFuseFlags(const std::filesystem::path& path, const int fuseFlags);
    NFileAccess AccessWithFuseFlags(const ASharedFileVariant& var, const int fuseFlags);
    NFileAccess AccessWithFuseFlags(const std::shared_ptr<rwl::TRwLock<TRegularFile>>& var, const int fuseFlags);
    NFileAccess AccessWithFuseFlags(const std::shared_ptr<rwl::TRwLock<TLink>>& var, const int fuseFlags);
    NFileAccess AccessWithFuseFlags(const std::shared_ptr<rwl::TRwLock<TDirectory>>& var, const int fuseFlags);

}

