#pragma once
#include <Models/TFileObjects.hpp>
#include <sys/stat.h>

namespace fusevfs::NSFileAttributes {
    void Get(const ASharedFileVariant& var, struct stat* st);
}

