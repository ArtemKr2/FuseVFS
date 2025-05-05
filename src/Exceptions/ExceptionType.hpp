#pragma once
#include <cerrno>

namespace fusevfs {

namespace ExceptionType {
    enum ExceptionTypeEnum {
        AccessNotPermitted = -EACCES,
        FileNotExist = -ENOENT,
        NotDirectory = -ENOTDIR,
        NotLink = -ENOLINK,
        NotFile = -ENOENT
    };
}

using ExceptionTypeEnum = ExceptionType::ExceptionTypeEnum;

}

