#pragma once
#include <variant>
#include <vector>
#include <filesystem>

#include <Models/TFile.hpp>
#include <Models/NNFileType.hpp>
#include <ReadWriteLock/RWLock.hpp>

namespace fusevfs {

    class TRegularFile;
    class TDirectory;
    class TLink;


    template<typename T>
    concept CFileObject = std::same_as<T, TDirectory>
        || std::same_as<T, TRegularFile>
        || std::same_as<T, TLink>;

    template<typename T>
    concept CReadGuardFileObject = std::same_as<T, read_write_lock::RWLockReadGuard<TDirectory>>
        || std::same_as<T, read_write_lock::RWLockReadGuard<TRegularFile>>
        || std::same_as<T, read_write_lock::RWLockReadGuard<TLink>>;

    template<typename T>
    concept CWriteGuardFileObject = std::same_as<T, read_write_lock::RWLockWriteGuard<TDirectory>>
        || std::same_as<T, read_write_lock::RWLockWriteGuard<TRegularFile>>
        || std::same_as<T, read_write_lock::RWLockWriteGuard<TLink>>;

    template<typename T>
    concept CGuardFileObject = CReadGuardFileObject<T> || CWriteGuardFileObject<T>;

    template<typename T>
    concept CSharedRwFileObject = std::same_as<T, std::shared_ptr<read_write_lock::RWLock<TDirectory>>>
        || std::same_as<T, std::shared_ptr<read_write_lock::RWLock<TRegularFile>>>
        || std::same_as<T, std::shared_ptr<read_write_lock::RWLock<TLink>>>;

    using ASharedFileVariant = std::variant<
    std::shared_ptr<read_write_lock::RWLock<TDirectory>>,
    std::shared_ptr<read_write_lock::RWLock<TRegularFile>>,
    std::shared_ptr<read_write_lock::RWLock<TLink>>>;

    class TDirectory : public TFile<TDirectory> {
    public:
        TDirectory()=default;
        static std::shared_ptr<read_write_lock::RWLock<TDirectory>> New(const std::string& name, mode_t mode, const std::shared_ptr<read_write_lock::RWLock<TDirectory>>& parent);

        std::vector<ASharedFileVariant> Files;
        static constexpr NFileType FileType = NFileType::Directory;
    };

    class TRegularFile : public TFile<TDirectory> {
    public:
        TRegularFile()=default;
        static std::shared_ptr<read_write_lock::RWLock<TRegularFile>> New(const std::string& name, mode_t mode, const std::shared_ptr<read_write_lock::RWLock<TDirectory>>& parent);

        std::vector<char> Data;
        static constexpr NFileType FileType = NFileType::File;
    };



    class TLink : TFile<TDirectory> {
    public:
        TLink()=default;
        static std::shared_ptr<read_write_lock::RWLock<TLink>> New(const std::string& name, mode_t mode, const std::shared_ptr<read_write_lock::RWLock<TDirectory>>& parent, const std::filesystem::path& path);

        std::filesystem::path LinkTo;
        static constexpr NFileType FileType = NFileType::Link;
    };

}

