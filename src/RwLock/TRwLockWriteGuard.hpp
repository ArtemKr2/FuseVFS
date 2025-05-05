#pragma once
#include <RwLock/TRwLockGuardBase.hpp>

namespace rwl {

template<typename T>
class TRwLockWriteGuard : public TRwLockGuardBase<T> {
    public:
    TRwLockWriteGuard(const std::shared_mutex* sharedMutex, const T* data)
        : TRwLockGuardBase<T>(sharedMutex, data) {
        this->m_pSharedMutex->lock();
    }
    ~TRwLockWriteGuard() { this->m_pSharedMutex->unlock(); }
    TRwLockWriteGuard(TRwLockWriteGuard&& other) noexcept
        : TRwLockGuardBase<T>(std::move(other)) {}
};

}

