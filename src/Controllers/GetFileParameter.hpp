#pragma once
#include <Models/FileObjects.hpp>

namespace fusevfs {

template<typename FieldType, typename Derived>
class GetFileParameter {
    public:
    GetFileParameter()=default;

    const FieldType& operator()(const FileObjectSharedVariant& var) {
        return std::visit(*this, var);
    }
    const FieldType& operator()(const FileObjectSharedRWConcept auto& var) {
        return reinterpret_cast<Derived*>(this)->operator()(var->Read());
    }
};

class GetNameParameter : public GetFileParameter<std::string, GetNameParameter> {
    public:
    using GetFileParameter<std::string, GetNameParameter>::operator();
    GetNameParameter()=default;
    const std::string& operator()(const FileObjectGuardConcept auto& var) {
        return reinterpret_cast<const File<Directory>*>(var.GetPtr())->FileName;
    }
    std::string& operator()(FileObjectWriteGuardConcept auto& var) {
        return reinterpret_cast<File<Directory>*>(var.GetPtr())->FileName;
    }
};

class GetUIDParameter : public GetFileParameter<uid_t, GetUIDParameter> {
    public:
    using GetFileParameter<uid_t, GetUIDParameter>::operator();
    GetUIDParameter()=default;
    const uid_t& operator()(const FileObjectGuardConcept auto& var) {
        return reinterpret_cast<const File<Directory>*>(var.GetPtr())->FileUID;
    }
    uid_t& operator()(FileObjectWriteGuardConcept auto& var) {
        return reinterpret_cast<File<Directory>*>(var.GetPtr())->FileUID;
    }
};

class GetGIDParameter : public GetFileParameter<gid_t, GetGIDParameter> {
    public:
    using GetFileParameter<gid_t, GetGIDParameter>::operator();
    GetGIDParameter()=default;
    const gid_t& operator()(const FileObjectGuardConcept auto& var) {
        return reinterpret_cast<const File<Directory>*>(var.GetPtr())->FileGID;
    }
    gid_t& operator()(FileObjectWriteGuardConcept auto& var) {
        return reinterpret_cast<File<Directory>*>(var.GetPtr())->FileGID;
    }
};

class GetModeParameter : public GetFileParameter<mode_t, GetModeParameter> {
    public:
    using GetFileParameter<mode_t, GetModeParameter>::operator();
    GetModeParameter()=default;
    const mode_t& operator()(const FileObjectGuardConcept auto& var) {
        return reinterpret_cast<const File<Directory>*>(var.GetPtr())->FileMode;
    }
    mode_t& operator()(FileObjectWriteGuardConcept auto& var) {
        return reinterpret_cast<File<Directory>*>(var.GetPtr())->FileMode;
    }
};

class GetParentParameter : public GetFileParameter<std::weak_ptr<read_write_lock::RWLock<Directory>>, GetParentParameter> {
    public:
    using GetFileParameter<std::weak_ptr<read_write_lock::RWLock<Directory>>, GetParentParameter>::operator();
    GetParentParameter()=default;
    const std::weak_ptr<read_write_lock::RWLock<Directory>>& operator()(const FileObjectGuardConcept auto& var) {
        return reinterpret_cast<const File<Directory>*>(var.GetPtr())->FileParent;
    }
    std::weak_ptr<read_write_lock::RWLock<Directory>>& operator()(FileObjectWriteGuardConcept auto& var) {
        return reinterpret_cast<File<Directory>*>(var.GetPtr())->FileParent;
    }
};

template<typename Derived>
class GetTimePointParameter
    : public GetFileParameter<std::chrono::system_clock::time_point, Derived>
{
    using base =
        GetFileParameter<std::chrono::system_clock::time_point, Derived>;
public:
    using base::operator();

};

/*------------------------------------------------------------------
 * 1. creation-time (birth-time)
 *-----------------------------------------------------------------*/
class GetChangedParameter: public GetTimePointParameter<GetChangedParameter>
{
public:
    using GetTimePointParameter<GetChangedParameter>::operator();
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
class GetModifiedParameter : public GetTimePointParameter<GetModifiedParameter>
{
public:
    using GetTimePointParameter<GetModifiedParameter>::operator();

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
class GetAccessedParameter : public GetTimePointParameter<GetAccessedParameter>
{
public:
    using GetTimePointParameter<GetAccessedParameter>::operator();

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

