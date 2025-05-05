#pragma once
namespace fusevfs {

namespace NNFileAccess {
    enum NFileAccess {
        Ok = 0,
        Restricted = -1
    };
}

using NFileAccess = NNFileAccess::NFileAccess;

}

