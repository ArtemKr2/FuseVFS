#pragma once
#include <ReadWriteLock/RWLockGuardBase.hpp>

namespace read_write_lock{

template<typename T>
class RWLockReadGuard : public RWLockGuardBase<const T> {
    public:
    RWLockReadGuard(const std::shared_mutex* sharedMutex, const T* data)
        : RWLockGuardBase<const T>(sharedMutex, data) {
        this->m_pSharedMutex->lock_shared();
    }
    ~RWLockReadGuard() { this->m_pSharedMutex->unlock_shared();}
    RWLockReadGuard(RWLockReadGuard&& other) noexcept
        : RWLockGuardBase<T>(std::move(other)) {}

};

}

