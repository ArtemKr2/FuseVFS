#include <Controllers/NSFindFile.hpp>
#include <Controllers/TFileSystem.hpp>
#include <Exceptions/TFSException.hpp>
#include <Controllers/TGetFileParameter.hpp>
#include <Controllers/NSAccessFile.hpp>

#include <array>
#include <iostream>
#include <map>

namespace fusevfs::NSFindFile {

static constexpr std::string_view s_sRootPath = "/";
static auto s_mNamePath = read_write_lock::RWLock<std::map<std::string, std::set<std::filesystem::path>>>();

ASharedFileVariant RecursiveFind(const std::filesystem::path& path,
    std::filesystem::path::iterator it,
    const read_write_lock::RWLockReadGuard<TDirectory>& dirRead) {

    const auto& itName = it->native();
    const auto& files = dirRead->Files;

    const auto childIt = std::ranges::find_if(files,
        [&itName](const auto& f) {
            return std::visit(TGetInfoName{}, f) == itName;
        }
    );
    if(childIt == files.end()) {
        throw TFSException(path.begin(), it, NFSExceptionType::FileNotExist);
    }
    if(std::distance(it, path.end()) == 1) {
        return *childIt;
    }
    if(const auto childDirPtr = std::get_if<std::shared_ptr<read_write_lock::RWLock<TDirectory>>>(&*childIt)) {
        const auto& childDir = *childDirPtr;
        if(NSAccessFile::Access(childDir, X_OK)==NNFileAccess::Restricted) {
            throw TFSException(path.begin(), it, NFSExceptionType::AccessNotPermitted);
        }
        return RecursiveFind(path, ++it, childDir->Read());
    }
    throw TFSException(path.begin(), it, NFSExceptionType::NotDirectory);
}

void AddToNameHash(const std::filesystem::path& path) {
    auto namePathWrite = s_mNamePath.Write();
    std::filesystem::path normalPath = path.lexically_normal();
    namePathWrite->operator[](path.filename()).insert(normalPath);
}

void RemoveFromNameHash(const std::filesystem::path& path) {
    auto namePathWrite = s_mNamePath.Write();
    auto normalPath = path.lexically_normal();
    auto filenamePath = normalPath.filename();
    const auto& filename = filenamePath.native();
    auto& collisions = namePathWrite->operator[](filename);
    collisions.erase(normalPath);
    if(collisions.empty()) {
        collisions.erase(filename);
    }
}

const std::set<std::filesystem::path>& FindByName(const std::string& name) {
    auto namePathRead = s_mNamePath.Read();
    if(!namePathRead->contains(name)) {
        throw TFSException(std::string_view(name), NFSExceptionType::FileNotExist);
    }
    return namePathRead->at(name);
}

template<typename T, auto FSExceptionValue>
std::shared_ptr<read_write_lock::RWLock<T>> FindGeneral(const std::filesystem::path& path) {
    const auto obj = Find(path);
    if(const auto t = std::get_if<std::shared_ptr<read_write_lock::RWLock<T>>>(&obj)) {
        return *t;
    }
    throw TFSException(path.begin(), path.end(), FSExceptionValue);
}

ASharedFileVariant Find(const std::filesystem::path& path) {
    const auto& rootDir = TFileSystem::RootDir();
    const auto normalizedPath = path.lexically_normal();
    if(normalizedPath == s_sRootPath) {
        return rootDir;
    }
    return RecursiveFind(normalizedPath, ++normalizedPath.begin(), rootDir->Read());
}

std::shared_ptr<read_write_lock::RWLock<TDirectory>> FindDir(const std::filesystem::path& path) {
    return FindGeneral<TDirectory, NFSExceptionType::NotDirectory>(path);
}

std::shared_ptr<read_write_lock::RWLock<TLink>> FindLink(const std::filesystem::path& path) {
    return FindGeneral<TLink, NFSExceptionType::NotLink>(path);
}

std::shared_ptr<read_write_lock::RWLock<TRegularFile>> FindRegularFile(const std::filesystem::path& path) {
    return FindGeneral<TRegularFile, NFSExceptionType::NotFile>(path);
}

}