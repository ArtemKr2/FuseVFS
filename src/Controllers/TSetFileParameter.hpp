#pragma once
#include <Models/FileObjects.hpp>
#include <Controllers/NSFileType.hpp>
#include <iostream>

namespace fusevfs {

    template<typename ParamType, typename DerivedType>
    class TSetInfoParameterMixin {
    protected:
        const ParamType m_xParam;
    public:
        explicit TSetInfoParameterMixin(const ParamType& param) : m_xParam{param} {}
        void operator()(const FileObjectSharedVariant& var) { std::visit(*Self(), var); }

        protected:
        constexpr DerivedType* Self() { return reinterpret_cast<DerivedType*>(this); }
        File<Directory>* FileBase(FileObjectWriteGuardConcept auto& var) {
            return reinterpret_cast<File<Directory>*>(var.GetPtr());
        }

    };

    template<typename ParamType, typename DerivedType>
    class TSetInfoParameterGeneralMixin : public TSetInfoParameterMixin<ParamType, DerivedType> {
    public:
        explicit TSetInfoParameterGeneralMixin(const ParamType& param)
            : TSetInfoParameterMixin<ParamType, DerivedType>(param) {}

        using TSetInfoParameterMixin<ParamType, DerivedType>::operator();
        void operator()(const FileObjectSharedRWConcept auto& var) {
            auto varWrite = var->Write();
            this->Self()->operator()(varWrite);
        }
    };

    // ────────────────────────────────────────────────────────────────
    //  Accessed  (atime)
    // ────────────────────────────────────────────────────────────────
    class TSetInfoAccessed
        : public TSetInfoParameterGeneralMixin<
              std::chrono::system_clock::time_point,
              TSetInfoAccessed>
    {

        using Base = TSetInfoParameterGeneralMixin<
                         std::chrono::system_clock::time_point,
                         TSetInfoAccessed>;

    public:
        explicit TSetInfoAccessed(std::chrono::system_clock::time_point tp)
            : Base(tp) {}

        using Base::operator();

        void operator()(FileObjectWriteGuardConcept auto& g) {
            this->FileBase(g)->m_accessed = this->m_xParam;
        }
    };

    // ────────────────────────────────────────────────────────────────
    //  Modified  (mtime)
    // ────────────────────────────────────────────────────────────────
    class TSetInfoModified
        : public TSetInfoParameterGeneralMixin<
              std::chrono::system_clock::time_point,
              TSetInfoModified>
    {
        using Base = TSetInfoParameterGeneralMixin<
                         std::chrono::system_clock::time_point,
                         TSetInfoModified>;
    public:
        explicit TSetInfoModified(std::chrono::system_clock::time_point tp)
            : Base(tp) {}

        using Base::operator();

        void operator()(FileObjectWriteGuardConcept auto& g) {
            this->FileBase(g)->m_modified = this->m_xParam;
        }
    };

    // ────────────────────────────────────────────────────────────────
    // ctime
    // ────────────────────────────────────────────────────────────────
    class TSetInfoChanged
        : public TSetInfoParameterGeneralMixin<
              std::chrono::system_clock::time_point,
              TSetInfoChanged>
    {
        using Base = TSetInfoParameterGeneralMixin<
                         std::chrono::system_clock::time_point,
                         TSetInfoChanged>;
    public:
        explicit TSetInfoChanged(std::chrono::system_clock::time_point tp)
            : Base(tp) {}

        using Base::operator();

        void operator()(FileObjectWriteGuardConcept auto& g) {
            this->FileBase(g)->m_changed = this->m_xParam;
        }
    };

    // ────────────────────────────────────────────────────────────────
    //  Name  (m_sName)
    // ────────────────────────────────────────────────────────────────
    class TSetInfoName
        : public TSetInfoParameterGeneralMixin<std::string, TSetInfoName>
    {
        using Base = TSetInfoParameterGeneralMixin<std::string, TSetInfoName>;

    public:
        explicit TSetInfoName(const std::string& p) : Base(p) {}

        using Base::operator();

        void operator()(FileObjectWriteGuardConcept auto& g) {
            this->FileBase(g)->m_sName = m_xParam;
        }
    };

    // ────────────────────────────────────────────────────────────────
    //  Uid  (m_uUid)
    // ────────────────────────────────────────────────────────────────
    class TSetInfoUid
        : public TSetInfoParameterGeneralMixin<uid_t, TSetInfoUid>
    {
        using Base = TSetInfoParameterGeneralMixin<uid_t, TSetInfoUid>;
    public:
        explicit TSetInfoUid(uid_t id) : Base(id) {}

        using Base::operator();

        void operator()(FileObjectWriteGuardConcept auto& g) {
            this->FileBase(g)->m_uUid = m_xParam;
        }
    };

    // ────────────────────────────────────────────────────────────────
    //  Gid  (m_uGid)
    // ────────────────────────────────────────────────────────────────
    class TSetInfoGid
        : public TSetInfoParameterGeneralMixin<gid_t, TSetInfoGid>
    {
        using Base = TSetInfoParameterGeneralMixin<gid_t, TSetInfoGid>;
    public:
        explicit TSetInfoGid(gid_t id) : Base(id) {}

        using Base::operator();

        void operator()(FileObjectWriteGuardConcept auto& g) {
            this->FileBase(g)->m_uGid = m_xParam;
        }
    };

    // ────────────────────────────────────────────────────────────────
    //  Mode  (m_uMode)
    // ────────────────────────────────────────────────────────────────
    class TSetInfoMode
        : public TSetInfoParameterGeneralMixin<mode_t, TSetInfoMode>
    {
        using Base = TSetInfoParameterGeneralMixin<mode_t, TSetInfoMode>;
    public:
        explicit TSetInfoMode(mode_t m) : Base(m) { }

        using Base::operator();

        void operator()(FileObjectWriteGuardConcept auto& g, bool is_first=false) {
            auto* fileBase = this->FileBase(g);
            constexpr mode_t PERM_MASK = S_IRWXU|S_IRWXG|S_IRWXO;  // 0777
            constexpr mode_t TYPE_MASK = S_IFMT;                  // 0170000

            if (is_first) {
                fileBase->m_uMode = m_xParam;
            } else {
                mode_t old_type  = fileBase->m_uMode & TYPE_MASK;
                mode_t new_perms = m_xParam  & PERM_MASK;
                fileBase->m_uMode = old_type | new_perms;
            }

        }
    };

    // ────────────────────────────────────────────────────────────────
    //  Parent  (m_pParent)
    // ────────────────────────────────────────────────────────────────
    class TSetInfoParent
        : public TSetInfoParameterMixin<
              std::shared_ptr<read_write_lock::RWLock<Directory>>,
              TSetInfoParent>
    {
        using Base = TSetInfoParameterMixin<
                         std::shared_ptr<read_write_lock::RWLock<Directory>>,
                         TSetInfoParent>;
    public:
        explicit TSetInfoParent(const std::shared_ptr<read_write_lock::RWLock<Directory>>& p)
            : Base(p) {}

        using Base::operator();

        void operator()(const FileObjectSharedRWConcept auto& lock)
        {
            {
                auto wr = lock->Write();
                (*this)(wr);
            }
            if (m_xParam) {
                auto parentWr = m_xParam->Write();
                parentWr->Files.push_back(lock);
            }
        }

    protected:
        void operator()(FileObjectWriteGuardConcept auto& g)
        {
            this->FileBase(g)->m_pParent = m_xParam;
        }
    };

}
