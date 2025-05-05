#pragma once
#include <Models/TFileObjects.hpp>
#include <RwLock/TRwLock.hpp>
#include <set>

namespace fusevfs {

namespace NSFindFile {

ASharedFileVariant Find(const std::filesystem::path& path);
void AddToNameHash(const std::filesystem::path& path);
void RemoveFromNameHash(const std::filesystem::path& path);
const std::set<std::filesystem::path>& FindByName(const std::string& name);
std::shared_ptr<rwl::TRwLock<TDirectory>> FindDir(const std::filesystem::path& path);
std::shared_ptr<rwl::TRwLock<TLink>> FindLink(const std::filesystem::path& path);
std::shared_ptr<rwl::TRwLock<TRegularFile>> FindRegularFile(const std::filesystem::path& path);

};

}

