#pragma once
#include <shared_mutex>
#include <memory>
#include <type_traits>

namespace read_write_lock{

template<typename T>
class RWLockGuardBase {
    public:
    using InnerType = T;

    public:
    RWLockGuardBase(const std::shared_mutex* sharedMutex, const T* data)
        :   m_pSharedMutex{const_cast<std::shared_mutex*>(sharedMutex)},
            m_pData{const_cast<T*>(data)} {}
    ~RWLockGuardBase()=default;
    RWLockGuardBase(const RWLockGuardBase&)=delete;
    RWLockGuardBase& operator=(const RWLockGuardBase&)=delete;
    RWLockGuardBase(RWLockGuardBase&& other) noexcept { MoveInit(std::move(other)); }
    RWLockGuardBase& operator=(RWLockGuardBase&& other) noexcept { MoveInit(std::move(other)); }

    public:
    inline const T* GetPtr() const { return this->m_pData; }
    inline T* GetPtr() { return this->m_pData; }

    public:
    inline const T* operator->() const { return this->m_pData; }
    inline T* operator->() { return this->m_pData; }

    public:
    inline const T& operator*() const { return *this->m_pData; }
    inline T& operator*() { return *this->m_pData; }

    protected:
    void MoveInit(RWLockGuardBase&& other) noexcept {
        m_pSharedMutex = other.m_pSharedMutex;
        m_pData = std::move(other.m_pData);
        other.m_pSharedMutex = nullptr;
        other.m_pData = nullptr;
    }

    protected:
    std::shared_mutex* m_pSharedMutex;
    T* m_pData;
};

}

