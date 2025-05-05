#pragma once
namespace fusevfs {

namespace FileAccess {
    enum FileAccessType {
        Ok = 0,
        Restricted = -1
    };
}

using FileAccessType = FileAccess::FileAccessType;

}

