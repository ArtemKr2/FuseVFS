#pragma once
#include <sys/stat.h>

namespace fusevfs {

namespace NNFileType {
    enum NFileType {
        Directory = S_IFDIR,
        File = S_IFREG,
        Link = S_IFLNK
    };
}

using NFileType = NNFileType::NFileType;

}

