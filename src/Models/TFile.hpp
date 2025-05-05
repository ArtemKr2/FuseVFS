#pragma once
#include <RwLock/TRwLock.hpp>

namespace fusevfs {

    template<typename ParentType>
    class TFile {

    protected:
        std::chrono::system_clock::time_point m_modified;  // modification time
        std::chrono::system_clock::time_point m_accessed;  // accessed time
        std::chrono::system_clock::time_point m_changed;   // ctime

        std::string m_sName;
        mode_t m_uMode = 0;
        uid_t m_uUid = 0;
        gid_t m_uGid = 0;
        std::weak_ptr<rwl::TRwLock<ParentType>> m_pParent;
    public:
        TFile()=default;


        friend class TGetInfoChanged;
        friend class TGetInfoModified;
        friend class TGetInfoAccessed;
        friend class TSetInfoName;
        friend class TSetInfoMode;
        friend class TSetInfoUid;
        friend class TSetInfoGid;
        friend class TSetInfoParent;
        friend class TSetInfoAccessed;
        friend class TSetInfoModified;
        friend class TSetInfoChanged;

        friend class TGetInfoName;
        friend class TGetInfoMode;
        friend class TGetInfoUid;
        friend class TGetInfoGid;
        friend class TGetInfoParent;

    };

}

