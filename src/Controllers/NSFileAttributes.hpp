#pragma once
#include <Models/FileObjects.hpp>
#include <sys/stat.h>

namespace fusevfs::NSFileAttributes {
    void Get(const FileObjectSharedVariant& var, struct stat* st);
}

