#pragma once
#include <RwLock/TRwLockGuardBase.hpp>

namespace rwl {

template<typename T>
class TRwLockTryReadGuard : public TRwLockGuardBase<const T> {
    public:
    TRwLockTryReadGuard(const std::shared_mutex* sharedMutex, const T* data, bool& isAcquired)
        : TRwLockGuardBase<const T>(sharedMutex, data) {
        isAcquired = this->m_pSharedMutex->try_lock_shared();
    }
    ~TRwLockTryReadGuard() { this->m_pSharedMutex->unlock_shared(); }
    TRwLockTryReadGuard(TRwLockTryReadGuard&& other) noexcept
        : TRwLockGuardBase<const T>(std::move(other)) {}
};

}