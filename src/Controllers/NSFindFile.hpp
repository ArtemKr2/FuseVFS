#pragma once
#include <Models/TFileObjects.hpp>
#include <ReadWriteLock/RWLock.hpp>
#include <set>

namespace fusevfs {

namespace NSFindFile {

ASharedFileVariant Find(const std::filesystem::path& path);
void AddToNameHash(const std::filesystem::path& path);
void RemoveFromNameHash(const std::filesystem::path& path);
const std::set<std::filesystem::path>& FindByName(const std::string& name);
std::shared_ptr<read_write_lock::RWLock<TDirectory>> FindDir(const std::filesystem::path& path);
std::shared_ptr<read_write_lock::RWLock<TLink>> FindLink(const std::filesystem::path& path);
std::shared_ptr<read_write_lock::RWLock<TRegularFile>> FindRegularFile(const std::filesystem::path& path);

};

}

