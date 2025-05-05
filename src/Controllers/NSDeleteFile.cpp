#include <algorithm>

#include <Controllers/NSDeleteFile.hpp>
#include <Controllers/TGetFileParameter.hpp>
#include <Controllers/NSFindFile.hpp>
#include <Exceptions/FSException.hpp>

namespace fusevfs::NSDeleteFile {

void DeleteChildrenInDirectory(const std::shared_ptr<read_write_lock::RWLock<Directory>>& dir, const std::filesystem::path& dirPath);

static void DeleteWithIterator( std::vector<FileObjectSharedVariant>& parentFiles,
                                std::vector<FileObjectSharedVariant>::iterator it, const std::filesystem::path& itPath) {

    if(const auto childDirPtr = std::get_if<std::shared_ptr<read_write_lock::RWLock<Directory>>>(&*it)) {
        DeleteChildrenInDirectory(*childDirPtr, itPath);
    }

    NSFindFile::RemoveFromNameHash(itPath);
    parentFiles.erase(it);
}

void DeleteChildrenInDirectory(const std::shared_ptr<read_write_lock::RWLock<Directory>>& dir, const std::filesystem::path& dirPath) {
    auto dirWrite = dir->Write();
    auto& files = dirWrite->Files;
    for(auto i = unsigned(0); i < files.size(); ++i) {
        const auto it = files.begin() + i;
        DeleteWithIterator(files, it, dirPath / TGetInfoName{}(*it));
        --i;
    }
}

void Delete(const std::filesystem::path& path) {
    const auto fileName = path.filename();
    const auto parentDir = NSFindFile::FindDir(path.parent_path());
    auto parentDirWrite = parentDir->Write();
    auto& parentFiles = parentDirWrite->Files;
    const auto it = std::find_if(parentFiles.begin(), parentFiles.end(),
        [&fileName](const auto& f) {
            return TGetInfoName{}(f) == fileName;
        }
    );
    if(it == parentFiles.end()) {
        throw FSException(path, ExceptionTypeEnum::FileNotExist);
    }
    DeleteWithIterator(parentFiles, it, path);
}

}
