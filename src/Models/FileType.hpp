#pragma once
#include <sys/stat.h>

namespace fusevfs {

namespace FileType {
    enum FileTypeEnum {
        Directory = S_IFDIR,
        File = S_IFREG,
        Link = S_IFLNK
    };
}

using FileTypeEnum = FileType::FileTypeEnum;

}

