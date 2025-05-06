#pragma once
#include <Models/FileObjects.hpp>
#include <iostream>

namespace fusevfs {

    template<typename ParamType, typename DerivedType>
    class SetParameterMixin {
    protected:
        const ParamType m_xParam;
    public:
        explicit SetParameterMixin(const ParamType& param) : m_xParam{param} {}
        void operator()(const FileObjectSharedVariant& var) { std::visit(*Self(), var); }

        protected:
        constexpr DerivedType* Self() { return reinterpret_cast<DerivedType*>(this); }
        File<Directory>* FileBase(FileObjectWriteGuardConcept auto& var) {
            return reinterpret_cast<File<Directory>*>(var.GetPtr());
        }

    };

    template<typename ParamType, typename DerivedType>
    class SetParameterGeneral : public SetParameterMixin<ParamType, DerivedType> {
    public:
        explicit SetParameterGeneral(const ParamType& param)
            : SetParameterMixin<ParamType, DerivedType>(param) {}

        using SetParameterMixin<ParamType, DerivedType>::operator();
        void operator()(const FileObjectSharedRWConcept auto& var) {
            auto varWrite = var->Write();
            this->Self()->operator()(varWrite);
        }
    };

    // ────────────────────────────────────────────────────────────────
    //  Accessed  (atime)
    // ────────────────────────────────────────────────────────────────
    class SetAccessedParameter
        : public SetParameterGeneral<
              std::chrono::system_clock::time_point,
              SetAccessedParameter>
    {

        using Base = SetParameterGeneral<
                         std::chrono::system_clock::time_point,
                         SetAccessedParameter>;

    public:
        explicit SetAccessedParameter(std::chrono::system_clock::time_point tp)
            : Base(tp) {}

        using Base::operator();

        void operator()(FileObjectWriteGuardConcept auto& g) {
            this->FileBase(g)->m_accessed = this->m_xParam;
        }
    };

    // ────────────────────────────────────────────────────────────────
    //  Modified  (mtime)
    // ────────────────────────────────────────────────────────────────
    class SetModifiedParameter
        : public SetParameterGeneral<
              std::chrono::system_clock::time_point,
              SetModifiedParameter>
    {
        using Base = SetParameterGeneral<
                         std::chrono::system_clock::time_point,
                         SetModifiedParameter>;
    public:
        explicit SetModifiedParameter(std::chrono::system_clock::time_point tp)
            : Base(tp) {}

        using Base::operator();

        void operator()(FileObjectWriteGuardConcept auto& g) {
            this->FileBase(g)->m_modified = this->m_xParam;
        }
    };

    // ────────────────────────────────────────────────────────────────
    // ctime
    // ────────────────────────────────────────────────────────────────
    class SetChangedParameter
        : public SetParameterGeneral<
              std::chrono::system_clock::time_point,
              SetChangedParameter>
    {
        using Base = SetParameterGeneral<
                         std::chrono::system_clock::time_point,
                         SetChangedParameter>;
    public:
        explicit SetChangedParameter(std::chrono::system_clock::time_point tp)
            : Base(tp) {}

        using Base::operator();

        void operator()(FileObjectWriteGuardConcept auto& g) {
            this->FileBase(g)->m_changed = this->m_xParam;
        }
    };

    // ────────────────────────────────────────────────────────────────
    //  Name  (FileName)
    // ────────────────────────────────────────────────────────────────
    class SetNameParameter
        : public SetParameterGeneral<std::string, SetNameParameter>
    {
        using Base = SetParameterGeneral<std::string, SetNameParameter>;

    public:
        explicit SetNameParameter(const std::string& p) : Base(p) {}

        using Base::operator();

        void operator()(FileObjectWriteGuardConcept auto& g) {
            this->FileBase(g)->FileName = m_xParam;
        }
    };

    // ────────────────────────────────────────────────────────────────
    //  Uid  (FileUID)
    // ────────────────────────────────────────────────────────────────
    class SetUIDParameter
        : public SetParameterGeneral<uid_t, SetUIDParameter>
    {
        using Base = SetParameterGeneral<uid_t, SetUIDParameter>;
    public:
        explicit SetUIDParameter(uid_t id) : Base(id) {}

        using Base::operator();

        void operator()(FileObjectWriteGuardConcept auto& g) {
            this->FileBase(g)->FileUID = m_xParam;
        }
    };

    // ────────────────────────────────────────────────────────────────
    //  Gid  (FileGID)
    // ────────────────────────────────────────────────────────────────
    class SetGIDParameter
        : public SetParameterGeneral<gid_t, SetGIDParameter>
    {
        using Base = SetParameterGeneral<gid_t, SetGIDParameter>;
    public:
        explicit SetGIDParameter(gid_t id) : Base(id) {}

        using Base::operator();

        void operator()(FileObjectWriteGuardConcept auto& g) {
            this->FileBase(g)->FileGID = m_xParam;
        }
    };

    // ────────────────────────────────────────────────────────────────
    //  Mode  (FileMode)
    // ────────────────────────────────────────────────────────────────
    class SeModeParameter
        : public SetParameterGeneral<mode_t, SeModeParameter>
    {
        using Base = SetParameterGeneral<mode_t, SeModeParameter>;
    public:
        explicit SeModeParameter(mode_t m) : Base(m) { }

        using Base::operator();

        void operator()(FileObjectWriteGuardConcept auto& g, bool is_first=false) {
            auto* fileBase = this->FileBase(g);
            constexpr mode_t PERM_MASK = S_IRWXU|S_IRWXG|S_IRWXO;  // 0777
            constexpr mode_t TYPE_MASK = S_IFMT;                  // 0170000

            if (is_first) {
                fileBase->FileMode = m_xParam;
            } else {
                mode_t old_type  = fileBase->FileMode & TYPE_MASK;
                mode_t new_perms = m_xParam  & PERM_MASK;
                fileBase->FileMode = old_type | new_perms;
            }

        }
    };

    // ────────────────────────────────────────────────────────────────
    //  Parent  (FileParent)
    // ────────────────────────────────────────────────────────────────
    class SetParentParameter
        : public SetParameterMixin<
              std::shared_ptr<read_write_lock::RWLock<Directory>>,
              SetParentParameter>
    {
        using Base = SetParameterMixin<
                         std::shared_ptr<read_write_lock::RWLock<Directory>>,
                         SetParentParameter>;
    public:
        explicit SetParentParameter(const std::shared_ptr<read_write_lock::RWLock<Directory>>& p)
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
            this->FileBase(g)->FileParent = m_xParam;
        }
    };

}
