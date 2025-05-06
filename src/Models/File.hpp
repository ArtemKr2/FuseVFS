#pragma once
#include <ReadWriteLock/RWLock.hpp>

namespace fusevfs {

    template<typename Parent>
    class File {

    protected:
        std::chrono::system_clock::time_point m_modified;  // modification time
        std::chrono::system_clock::time_point m_accessed;  // accessed time
        std::chrono::system_clock::time_point m_changed;   // ctime

        std::string FileName;
        mode_t FileMode = 0;
        uid_t FileUID = 0;
        gid_t FileGID = 0;
        std::weak_ptr<read_write_lock::RWLock<Parent>> FileParent;
    public:
        File()=default;


        friend class GetChangedParameter;
        friend class GetModifiedParameter;
        friend class GetAccessedParameter;
        friend class SetNameParameter;
        friend class SeModeParameter;
        friend class SetUIDParameter;
        friend class SetGIDParameter;
        friend class SetParentParameter;
        friend class SetAccessedParameter;
        friend class SetModifiedParameter;
        friend class SetChangedParameter;

        friend class GetNameParameter;
        friend class GetModeParameter;
        friend class GetUIDParameter;
        friend class GetGIDParameter;
        friend class GetParentParameter;

    };

}

