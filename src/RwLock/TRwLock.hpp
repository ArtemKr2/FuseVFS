#pragma once
#include <shared_mutex>
#include <optional>

#include <RwLock/TRwLockWriteGuard.hpp>
#include <RwLock/TRwLockReadGuard.hpp>
#include <RwLock/TRwLockTryWriteGuard.hpp>
#include <RwLock/TRwLockTryReadGuard.hpp>

namespace rwl {

template<typename T>
class TRwLock {
    public:
    using InnerType = T;

    template<typename ...Args>
    TRwLock(Args&&... args) : m_xData(std::forward<Args>(args)...) {}

    TRwLock(const TRwLock&)=delete;
    TRwLock& operator=(const TRwLock&)=delete;

    public:
    TRwLockReadGuard<T> Read() const { return TRwLockReadGuard<T>(&m_xSharedMutex, &m_xData); }
    TRwLockWriteGuard<T> Write() const { return TRwLockWriteGuard(&m_xSharedMutex, &m_xData); }
    std::optional<TRwLockTryReadGuard<T>> TryRead() const { return TryGuard<TRwLockTryReadGuard<T>>(); }
    std::optional<TRwLockTryWriteGuard<T>> TryWrite() const { return TryGuard<TRwLockTryWriteGuard<T>>(); }

    protected:
    template<typename TryGuardType>
    std::optional<TryGuardType> TryGuard() const {
        bool isAcquired = false;
        auto guard = std::make_optional<TryGuardType>(&m_xSharedMutex, &m_xData, isAcquired);
        if(!isAcquired) {
            guard.reset();
        }
        return guard;
    }

    protected:
    std::shared_mutex m_xSharedMutex;
    T m_xData;
};

}

