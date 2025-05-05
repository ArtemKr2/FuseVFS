#pragma once
#include <Models/FileObjects.hpp>

namespace fusevfs {

template<typename FieldType, typename Derived>
class TGetFileParameter {
    public:
    TGetFileParameter()=default;

    const FieldType& operator()(const FileObjectSharedVariant& var) {
        return std::visit(*this, var);
    }
    const FieldType& operator()(const FileObjectSharedRWConcept auto& var) {
        return reinterpret_cast<Derived*>(this)->operator()(var->Read());
    }
};


class TGetInfoName : public TGetFileParameter<std::string, TGetInfoName> {
    public:
    using TGetFileParameter<std::string, TGetInfoName>::operator();
    TGetInfoName()=default;
    const std::string& operator()(const FileObjectGuardConcept auto& var) {
        return reinterpret_cast<const File<Directory>*>(var.GetPtr())->m_sName;
    }
    std::string& operator()(FileObjectWriteGuardConcept auto& var) {
        return reinterpret_cast<File<Directory>*>(var.GetPtr())->m_sName;
    }
};

class TGetInfoUid : public TGetFileParameter<uid_t, TGetInfoUid> {
    public:
    using TGetFileParameter<uid_t, TGetInfoUid>::operator();
    TGetInfoUid()=default;
    const uid_t& operator()(const FileObjectGuardConcept auto& var) {
        return reinterpret_cast<const File<Directory>*>(var.GetPtr())->m_uUid;
    }
    uid_t& operator()(FileObjectWriteGuardConcept auto& var) {
        return reinterpret_cast<File<Directory>*>(var.GetPtr())->m_uUid;
    }
};

class TGetInfoGid : public TGetFileParameter<gid_t, TGetInfoGid> {
    public:
    using TGetFileParameter<gid_t, TGetInfoGid>::operator();
    TGetInfoGid()=default;
    const gid_t& operator()(const FileObjectGuardConcept auto& var) {
        return reinterpret_cast<const File<Directory>*>(var.GetPtr())->m_uGid;
    }
    gid_t& operator()(FileObjectWriteGuardConcept auto& var) {
        return reinterpret_cast<File<Directory>*>(var.GetPtr())->m_uGid;
    }
};

class TGetInfoMode : public TGetFileParameter<mode_t, TGetInfoMode> {
    public:
    using TGetFileParameter<mode_t, TGetInfoMode>::operator();
    TGetInfoMode()=default;
    const mode_t& operator()(const FileObjectGuardConcept auto& var) {
        return reinterpret_cast<const File<Directory>*>(var.GetPtr())->m_uMode;
    }
    mode_t& operator()(FileObjectWriteGuardConcept auto& var) {
        return reinterpret_cast<File<Directory>*>(var.GetPtr())->m_uMode;
    }
};

class TGetInfoParent : public TGetFileParameter<std::weak_ptr<read_write_lock::RWLock<Directory>>, TGetInfoParent> {
    public:
    using TGetFileParameter<std::weak_ptr<read_write_lock::RWLock<Directory>>, TGetInfoParent>::operator();
    TGetInfoParent()=default;
    const std::weak_ptr<read_write_lock::RWLock<Directory>>& operator()(const FileObjectGuardConcept auto& var) {
        return reinterpret_cast<const File<Directory>*>(var.GetPtr())->m_pParent;
    }
    std::weak_ptr<read_write_lock::RWLock<Directory>>& operator()(FileObjectWriteGuardConcept auto& var) {
        return reinterpret_cast<File<Directory>*>(var.GetPtr())->m_pParent;
    }
};

template<typename Derived>
class TGetTimePoint
    : public TGetFileParameter<std::chrono::system_clock::time_point, Derived>
{
    using base =
        TGetFileParameter<std::chrono::system_clock::time_point, Derived>;
public:
    using base::operator();

};

/*------------------------------------------------------------------
 * 1. creation-time (birth-time)
 *-----------------------------------------------------------------*/
class TGetInfoChanged: public TGetTimePoint<TGetInfoChanged>
{
public:
    using TGetTimePoint<TGetInfoChanged>::operator();
    /* read-guard  (const) */
    const std::chrono::system_clock::time_point&
    operator()(const FileObjectGuardConcept auto& var) const
    {
        return reinterpret_cast<
            const File<Directory>*>(var.GetPtr())->m_changed;
    }
    /* write-guard (non-const) */
    std::chrono::system_clock::time_point&
    operator()(FileObjectWriteGuardConcept auto& var) const
    {
        return reinterpret_cast<
            File<Directory>*>(var.GetPtr())->m_changed;
    }
};

/*------------------------------------------------------------------
 * 2. modification-time (mtime)
 *-----------------------------------------------------------------*/
class TGetInfoModified : public TGetTimePoint<TGetInfoModified>
{
public:
    using TGetTimePoint<TGetInfoModified>::operator();

    const std::chrono::system_clock::time_point&
    operator()(const FileObjectGuardConcept auto& var) const
    {
        return reinterpret_cast<
            const File<Directory>*>(var.GetPtr())->m_modified;
    }
    std::chrono::system_clock::time_point&
    operator()(FileObjectWriteGuardConcept auto& var) const
    {
        return reinterpret_cast<
            File<Directory>*>(var.GetPtr())->m_modified;
    }
};

/*------------------------------------------------------------------
 * 3. access-time (atime)
 *-----------------------------------------------------------------*/
class TGetInfoAccessed : public TGetTimePoint<TGetInfoAccessed>
{
public:
    using TGetTimePoint<TGetInfoAccessed>::operator();

    const std::chrono::system_clock::time_point&
    operator()(const FileObjectGuardConcept auto& var) const
    {
        return reinterpret_cast<
            const File<Directory>*>(var.GetPtr())->m_accessed;
    }
    std::chrono::system_clock::time_point&
    operator()(FileObjectWriteGuardConcept auto& var) const
    {
        return reinterpret_cast<
            File<Directory>*>(var.GetPtr())->m_accessed;
    }
};

}

