#pragma once
#include <ReadWriteLock/RWLockGuardBase.hpp>

namespace read_write_lock{

template<typename T>
class RWLockTryReadGuard : public RWLockGuardBase<const T> {
    public:
    RWLockTryReadGuard(const std::shared_mutex* sharedMutex, const T* data, bool& isAcquired)
        : RWLockGuardBase<const T>(sharedMutex, data) {
        isAcquired = this->m_pSharedMutex->try_lock_shared();
    }
    ~RWLockTryReadGuard() { this->m_pSharedMutex->unlock_shared(); }
    RWLockTryReadGuard(RWLockTryReadGuard&& other) noexcept
        : RWLockGuardBase<const T>(std::move(other)) {}
};

}