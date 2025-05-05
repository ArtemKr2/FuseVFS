#pragma once
#include <RwLock/TRwLockGuardBase.hpp>

namespace rwl {

template<typename T>
class TRwLockTryWriteGuard : public TRwLockGuardBase<T> {
    public:
    TRwLockTryWriteGuard(const std::shared_mutex* sharedMutex, const T* data, bool& isAcquired)
        : TRwLockGuardBase<T>(sharedMutex, data) {
        isAcquired = this->m_pSharedMutex->try_lock();
    };
    ~TRwLockTryWriteGuard() { this->m_pSharedMutex->unlock(); }
    TRwLockTryWriteGuard(TRwLockTryWriteGuard&& other) noexcept
        : TRwLockGuardBase<T>(std::move(other)) {}
};

}

