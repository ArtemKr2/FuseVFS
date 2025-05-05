#pragma once
#include <shared_mutex>
#include <optional>

#include <ReadWriteLock/RWLockWriteGuard.hpp>
#include <ReadWriteLock/RWLockReadGuard.hpp>
#include <ReadWriteLock/RWLockTryWriteGuard.hpp>
#include <ReadWriteLock/RWLockTryReadGuard.hpp>

namespace read_write_lock{

template<typename T>
class RWLock {
    public:
    using InnerType = T;

    template<typename ...Args>
    RWLock(Args&&... args) : m_xData(std::forward<Args>(args)...) {}

    RWLock(const RWLock&)=delete;
    RWLock& operator=(const RWLock&)=delete;

    public:
    RWLockReadGuard<T> Read() const { return RWLockReadGuard<T>(&m_xSharedMutex, &m_xData); }
    RWLockWriteGuard<T> Write() const { return RWLockWriteGuard(&m_xSharedMutex, &m_xData); }
    std::optional<RWLockTryReadGuard<T>> TryRead() const { return TryGuard<RWLockTryReadGuard<T>>(); }
    std::optional<RWLockTryWriteGuard<T>> TryWrite() const { return TryGuard<RWLockTryWriteGuard<T>>(); }

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

