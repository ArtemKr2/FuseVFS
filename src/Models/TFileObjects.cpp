#include <Models/TFileObjects.hpp>
#include <Controllers/TSetFileParameter.hpp>

#define FUSE_USE_VERSION 30
#include <fuse3/fuse.h>

namespace fusevfs {

static void Update(rwl::TRwLockWriteGuard<TLink>& writeObj, const std::filesystem::path& path) {
    writeObj->LinkTo = path;
}

template<typename T, typename... Args>
static std::shared_ptr<rwl::TRwLock<T>> DoNew(const std::string& name, mode_t mode, const std::shared_ptr<rwl::TRwLock<TDirectory>>& parent, Args&& ... args) {
    const auto obj = std::make_shared<rwl::TRwLock<T>>();
    {
        auto objWrite = obj->Write();
        auto now  = std::chrono::system_clock::now();

        TSetInfoModified{now}(objWrite);
        TSetInfoAccessed{now}(objWrite);
        TSetInfoChanged{now}(objWrite);

        TSetInfoName{name}(objWrite);
        // TSetInfoMode{mode}(objWrite);
        const auto context = fuse_get_context();
        TSetInfoUid{context->uid}(objWrite);
        TSetInfoGid{context->gid}(objWrite);

        if constexpr(std::same_as<T, TLink>) {
            Update(objWrite, args...);
        }
        constexpr mode_t PERM_MASK = S_IRWXU|S_IRWXG|S_IRWXO;
        mode_t raw_perms = mode & PERM_MASK;

        // при создании передаём полный (тип|права) и is_first=true
        constexpr mode_t FILETYPE =
            std::same_as<T, TDirectory>   ? S_IFDIR  :
            std::same_as<T, TRegularFile> ? S_IFREG  :
            std::same_as<T, TLink>        ? S_IFLNK  :
                                            0;
        TSetInfoMode{ FILETYPE | raw_perms }(objWrite, true);
    }
    TSetInfoParent{parent}(obj);
    return obj;
}

std::shared_ptr<rwl::TRwLock<TDirectory>> TDirectory::New(const std::string& name, mode_t mode, const std::shared_ptr<rwl::TRwLock<fusevfs::TDirectory>>& parent) {
    return DoNew<TDirectory>(name, mode, parent);
}

std::shared_ptr<rwl::TRwLock<TRegularFile>> TRegularFile::New(const std::string& name, mode_t mode, const std::shared_ptr<rwl::TRwLock<fusevfs::TDirectory>>& parent) {
    return DoNew<TRegularFile>(name, mode, parent);
}

std::shared_ptr<rwl::TRwLock<TLink>> TLink::New(const std::string& name, mode_t mode, const std::shared_ptr<rwl::TRwLock<fusevfs::TDirectory>>& parent, const std::filesystem::path& path) {
    return DoNew<TLink>(name, mode, parent, path);
}

}