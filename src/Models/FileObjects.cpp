#include <Models/FileObjects.hpp>
#include <Controllers/SetFileParameter.hpp>

#define FUSE_USE_VERSION 30
#include <fuse3/fuse.h>

namespace fusevfs {

static void Update(read_write_lock::RWLockWriteGuard<Link>& writeObj, const std::filesystem::path& path) {
    writeObj->LinkTo = path;
}

template<typename T, typename... Args>
static std::shared_ptr<read_write_lock::RWLock<T>> DoNew(const std::string& name, mode_t mode, const std::shared_ptr<read_write_lock::RWLock<Directory>>& parent, Args&& ... args) {
    const auto obj = std::make_shared<read_write_lock::RWLock<T>>();
    {
        auto objWrite = obj->Write();
        auto now  = std::chrono::system_clock::now();

        SetModifiedParameter{now}(objWrite);
        SetAccessedParameter{now}(objWrite);
        SetChangedParameter{now}(objWrite);

        SetNameParameter{name}(objWrite);
        // SeModeParameter{mode}(objWrite);
        const auto context = fuse_get_context();
        SetUIDParameter{context->uid}(objWrite);
        SetGIDParameter{context->gid}(objWrite);

        if constexpr(std::same_as<T, Link>) {
            Update(objWrite, args...);
        }
        constexpr mode_t PERM_MASK = S_IRWXU|S_IRWXG|S_IRWXO;
        mode_t raw_perms = mode & PERM_MASK;

        // при создании передаём полный (тип|права) и is_first=true
        constexpr mode_t FILETYPE =
            std::same_as<T, Directory>   ? S_IFDIR  :
            std::same_as<T, RegularFile> ? S_IFREG  :
            std::same_as<T, Link>        ? S_IFLNK  :
                                            0;
        SeModeParameter{ FILETYPE | raw_perms }(objWrite, true);
    }
    SetParentParameter{parent}(obj);
    return obj;
}

std::shared_ptr<read_write_lock::RWLock<Directory>> Directory::New(const std::string& name, mode_t mode, const std::shared_ptr<read_write_lock::RWLock<fusevfs::Directory>>& parent) {
    return DoNew<Directory>(name, mode, parent);
}

std::shared_ptr<read_write_lock::RWLock<RegularFile>> RegularFile::New(const std::string& name, mode_t mode, const std::shared_ptr<read_write_lock::RWLock<fusevfs::Directory>>& parent) {
    return DoNew<RegularFile>(name, mode, parent);
}

std::shared_ptr<read_write_lock::RWLock<Link>> Link::New(const std::string& name, mode_t mode, const std::shared_ptr<read_write_lock::RWLock<fusevfs::Directory>>& parent, const std::filesystem::path& path) {
    return DoNew<Link>(name, mode, parent, path);
}

}