#include <Controllers/NSFindFile.hpp>
#include <Controllers/TFileSystem.hpp>
#include <Exceptions/FSException.hpp>
#include <Controllers/TGetFileParameter.hpp>
#include <Controllers/NSAccessFile.hpp>

#include <iostream>
#include <map>

namespace fusevfs::NSFindFile {

static constexpr std::string_view s_sRootPath = "/";
static auto s_mNamePath = read_write_lock::RWLock<std::map<std::string, std::set<std::filesystem::path>>>();

FileObjectSharedVariant RecursiveFind(const std::filesystem::path& path,
    std::filesystem::path::iterator it,
    const read_write_lock::RWLockReadGuard<Directory>& dirRead) {

    const auto& itName = it->native();
    const auto& files = dirRead->Files;

    const auto childIt = std::ranges::find_if(files,
        [&itName](const auto& f) {
            return std::visit(TGetInfoName{}, f) == itName;
        }
    );
    if(childIt == files.end()) {
        throw FSException(path.begin(), it, ExceptionTypeEnum::FileNotExist);
    }
    if(std::distance(it, path.end()) == 1) {
        return *childIt;
    }
    if(const auto childDirPtr = std::get_if<std::shared_ptr<read_write_lock::RWLock<Directory>>>(&*childIt)) {
        const auto& childDir = *childDirPtr;
        if(NSAccessFile::Access(childDir, X_OK)==FileAccessType::Restricted) {
            throw FSException(path.begin(), it, ExceptionTypeEnum::AccessNotPermitted);
        }
        return RecursiveFind(path, ++it, childDir->Read());
    }
    throw FSException(path.begin(), it, ExceptionTypeEnum::NotDirectory);
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
        throw FSException(std::string_view(name), ExceptionTypeEnum::FileNotExist);
    }
    return namePathRead->at(name);
}

template<typename T, auto FSExceptionValue>
std::shared_ptr<read_write_lock::RWLock<T>> FindGeneral(const std::filesystem::path& path) {
    const auto obj = Find(path);
    if(const auto t = std::get_if<std::shared_ptr<read_write_lock::RWLock<T>>>(&obj)) {
        return *t;
    }
    throw FSException(path.begin(), path.end(), FSExceptionValue);
}

FileObjectSharedVariant Find(const std::filesystem::path& path) {
    const auto& rootDir = TFileSystem::RootDir();
    const auto normalizedPath = path.lexically_normal();
    if(normalizedPath == s_sRootPath) {
        return rootDir;
    }
    return RecursiveFind(normalizedPath, ++normalizedPath.begin(), rootDir->Read());
}

std::shared_ptr<read_write_lock::RWLock<Directory>> FindDir(const std::filesystem::path& path) {
    return FindGeneral<Directory, ExceptionTypeEnum::NotDirectory>(path);
}

std::shared_ptr<read_write_lock::RWLock<Link>> FindLink(const std::filesystem::path& path) {
    return FindGeneral<Link, ExceptionTypeEnum::NotLink>(path);
}

std::shared_ptr<read_write_lock::RWLock<RegularFile>> FindRegularFile(const std::filesystem::path& path) {
    return FindGeneral<RegularFile, ExceptionTypeEnum::NotFile>(path);
}

}