#pragma once
#include <ReadWriteLock/RWLockGuardBase.hpp>

namespace read_write_lock{

template<typename T>
class RWLockTryWriteGuard : public RWLockGuardBase<T> {
    public:
    RWLockTryWriteGuard(const std::shared_mutex* sharedMutex, const T* data, bool& isAcquired)
        : RWLockGuardBase<T>(sharedMutex, data) {
        isAcquired = this->m_pSharedMutex->try_lock();
    };
    ~RWLockTryWriteGuard() { this->m_pSharedMutex->unlock(); }
    RWLockTryWriteGuard(RWLockTryWriteGuard&& other) noexcept
        : RWLockGuardBase<T>(std::move(other)) {}
};

}

