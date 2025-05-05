#pragma once
#include <cerrno>

namespace fusevfs {

namespace NNFSExceptionType {
    enum NFSExceptionType {
        AccessNotPermitted = -EACCES,
        FileNotExist = -ENOENT,
        NotDirectory = -ENOTDIR,
        NotLink = -ENOLINK,
        NotFile = -ENOENT
    };
}

using NFSExceptionType = NNFSExceptionType::NFSExceptionType;

}

