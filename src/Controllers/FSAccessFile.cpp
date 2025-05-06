#include <array>
#include <map>

#include <Controllers/FSAccessFile.hpp>
#include <Controllers/GetFileParameter.hpp>
#include <Controllers/FindFile.hpp>

#define FUSE_USE_VERSION 30
#include <fuse3/fuse.h>

namespace fusevfs::FSAccessFile {

    FileAccessType DoAccess(const std::array<int, 3>& sFlags, const mode_t mode, const int accessMask) {
        auto specializedMode = 0;
        static std::array accessFlags = {R_OK, W_OK, X_OK};
        for(auto i = 0u; i < accessFlags.size(); ++i) {
            if(mode & sFlags[i]) specializedMode |= accessFlags[i];
        }
        auto res = specializedMode & accessMask;
        return res ? FileAccessType::Ok : FileAccessType::Restricted;
    }

    FileAccessType AccessSpecialized(const FileObjectSharedRWConcept auto& var, const int accessMask) {
        const auto mode = GetModeParameter{}(var);
        const auto context = fuse_get_context();
        const auto uid = GetUIDParameter{}(var);

        if(uid == 0) {
            return FileAccessType::Ok;
        }

        if(uid == context->uid) {
            return DoAccess({S_IRUSR, S_IWUSR, S_IXUSR}, mode, accessMask);
        }
        if(GetGIDParameter{}(var) == context->gid) {
            return DoAccess({S_IRGRP, S_IWGRP, S_IXGRP}, mode, accessMask);
        }
        return DoAccess({S_IROTH, S_IWOTH, S_IXOTH}, mode, accessMask);
    }

    FileAccessType Access(const std::filesystem::path& path, const int accessMask) {
        return Access(FindFile::Find(path), accessMask);
    }

    FileAccessType Access(const FileObjectSharedVariant& var, const int accessMask) {
        return std::visit([accessMask](const auto& file) {
            return FSAccessFile::Access(file, accessMask);
        }, var);
    }

    FileAccessType Access(const std::shared_ptr<read_write_lock::RWLock<Link>>& var, const int accessMask) {
        return Access(FindFile::Find(var->Read()->LinkTo), accessMask);
    }

    FileAccessType Access(const std::shared_ptr<read_write_lock::RWLock<RegularFile>>& var, const int accessMask) {
        return AccessSpecialized(var, accessMask);
    }

    FileAccessType Access(const std::shared_ptr<read_write_lock::RWLock<Directory>>& var, const int accessMask) {
        return AccessSpecialized(var, accessMask);
    }

    FileAccessType AccessWithFuseFlags(const std::filesystem::path& path, const int fuseFlags) {
        return AccessWithFuseFlags(FindFile::Find(path), fuseFlags);
    }

    FileAccessType AccessWithFuseFlags(const FileObjectSharedVariant& var, const int fuseFlags) {
        return std::visit([fuseFlags](const auto& file) {
            return FSAccessFile::AccessWithFuseFlags(file, fuseFlags);
        }, var);
    }

    FileAccessType AccessWithFuseFlagsSpecialized(const FileObjectSharedRWConcept auto& var, const int fuseFlags) {
        const auto s_mAccessFlags = std::map<int, int> {
            {O_RDONLY, R_OK},
            {O_WRONLY, W_OK},
            {O_RDWR, W_OK | R_OK},
            {O_PATH, X_OK}
        };

        auto mask = 0;
        for(const auto [oFlag, okFlag] : s_mAccessFlags) {
            if((fuseFlags & oFlag) == oFlag) {
                mask |= okFlag;
            }
        }
        return FSAccessFile::Access(var, mask);
    }

    FileAccessType AccessWithFuseFlags(const std::shared_ptr<read_write_lock::RWLock<RegularFile>>& var, const int fuseFlags) {
        return AccessWithFuseFlagsSpecialized(var, fuseFlags);
    }

    FileAccessType AccessWithFuseFlags(const std::shared_ptr<read_write_lock::RWLock<Link>>& var, const int fuseFlags) {
        return AccessWithFuseFlagsSpecialized(var, fuseFlags);
    }

    FileAccessType AccessWithFuseFlags(const std::shared_ptr<read_write_lock::RWLock<Directory>>& var, const int fuseFlags) {
        return AccessWithFuseFlagsSpecialized(var, fuseFlags);
    }

}