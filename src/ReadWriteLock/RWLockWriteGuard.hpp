#pragma once
#include <ReadWriteLock/RWLockGuardBase.hpp>

namespace read_write_lock{

template<typename T>
class RWLockWriteGuard : public RWLockGuardBase<T> {
    public:
    RWLockWriteGuard(const std::shared_mutex* sharedMutex, const T* data)
        : RWLockGuardBase<T>(sharedMutex, data) {
        this->m_pSharedMutex->lock();
    }
    ~RWLockWriteGuard() { this->m_pSharedMutex->unlock(); }
    RWLockWriteGuard(RWLockWriteGuard&& other) noexcept
        : RWLockGuardBase<T>(std::move(other)) {}
};

}

