#pragma once
#include <Models/FileObjects.hpp>
#include <ReadWriteLock/RWLock.hpp>
#include <set>

namespace fusevfs::FindFile {
    FileObjectSharedVariant Find(const std::filesystem::path& path);
    void AddNameToHash(const std::filesystem::path& path);
    void RemoveNameFromHash(const std::filesystem::path& path);
    const std::set<std::filesystem::path>& FindByName(const std::string& name);
    std::shared_ptr<read_write_lock::RWLock<Directory>> FindDir(const std::filesystem::path& path);
    std::shared_ptr<read_write_lock::RWLock<Link>> FindLink(const std::filesystem::path& path);
    std::shared_ptr<read_write_lock::RWLock<RegularFile>> FindRegularFile(const std::filesystem::path& path);

}

