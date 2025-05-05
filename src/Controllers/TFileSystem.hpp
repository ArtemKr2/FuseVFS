#pragma once
#define FUSE_USE_VERSION 30
#include <fuse3/fuse.h>

#include <Models/FileObjects.hpp>

#include <filesystem>

#include "CLI/Option.hpp"

namespace fusevfs {

class TFileSystem {
public:
    static const std::shared_ptr<read_write_lock::RWLock<Directory>>& RootDir();

    static std::filesystem::path SocketPath;

    static int Init(int argc, char *argv[]);

protected:
    static int Utimens(const char* path, const struct timespec tv[2], struct fuse_file_info*);

    static int Truncate(const char* path, off_t size, struct fuse_file_info* fi);

    static int Access(const char* path, int accessMask);

    static int GetAttr(const char* path, struct stat* st, struct fuse_file_info* fi);

    static int ReadLink(const char* path, char* buffer, size_t size);

    static int MkNod(const char* path, mode_t mode, dev_t rdev);

    static int MkDir(const char* path, mode_t mode);

    static int RmDir(const char* path);

    static int Unlink(const char* path);

    static int SymLink(const char* target_path, const char* link_path);

    static int ChMod(const char* path, mode_t mode, struct fuse_file_info *fi);

    static int Open(const char* path, struct fuse_file_info* info);

    static int Read(const char* path, char* buffer, size_t size, off_t offset, struct fuse_file_info *fi);

    static int Write(const char* path, const char* buffer, size_t size, off_t offset, struct fuse_file_info *info);

    static int OpenDir(const char* path, struct fuse_file_info* info);

    static int ReadDir(const char* path, void* buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *info, enum fuse_readdir_flags flags);

    static void ServerLoop(int server_fd);

    // static void FindByNameThread();
};
    template<std::integral T>
        constexpr bool IsHasFlag(T value, T flag);
}



