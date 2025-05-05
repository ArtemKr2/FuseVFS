#pragma once
#include <variant>
#include <vector>
#include <filesystem>

#include <Models/TFile.hpp>
#include <Models/NNFileType.hpp>
#include <RwLock/TRwLock.hpp>

namespace fusevfs {

    class TRegularFile;
    class TDirectory;
    class TLink;


    template<typename T>
    concept CFileObject = std::same_as<T, TDirectory>
        || std::same_as<T, TRegularFile>
        || std::same_as<T, TLink>;

    template<typename T>
    concept CReadGuardFileObject = std::same_as<T, rwl::TRwLockReadGuard<TDirectory>>
        || std::same_as<T, rwl::TRwLockReadGuard<TRegularFile>>
        || std::same_as<T, rwl::TRwLockReadGuard<TLink>>;

    template<typename T>
    concept CWriteGuardFileObject = std::same_as<T, rwl::TRwLockWriteGuard<TDirectory>>
        || std::same_as<T, rwl::TRwLockWriteGuard<TRegularFile>>
        || std::same_as<T, rwl::TRwLockWriteGuard<TLink>>;

    template<typename T>
    concept CGuardFileObject = CReadGuardFileObject<T> || CWriteGuardFileObject<T>;

    template<typename T>
    concept CSharedRwFileObject = std::same_as<T, std::shared_ptr<rwl::TRwLock<TDirectory>>>
        || std::same_as<T, std::shared_ptr<rwl::TRwLock<TRegularFile>>>
        || std::same_as<T, std::shared_ptr<rwl::TRwLock<TLink>>>;

    using ASharedFileVariant = std::variant<
    std::shared_ptr<rwl::TRwLock<TDirectory>>,
    std::shared_ptr<rwl::TRwLock<TRegularFile>>,
    std::shared_ptr<rwl::TRwLock<TLink>>>;

    class TDirectory : public TFile<TDirectory> {
    public:
        TDirectory()=default;
        static std::shared_ptr<rwl::TRwLock<TDirectory>> New(const std::string& name, mode_t mode, const std::shared_ptr<rwl::TRwLock<TDirectory>>& parent);

        std::vector<ASharedFileVariant> Files;
        static constexpr NFileType FileType = NFileType::Directory;
    };

    class TRegularFile : public TFile<TDirectory> {
    public:
        TRegularFile()=default;
        static std::shared_ptr<rwl::TRwLock<TRegularFile>> New(const std::string& name, mode_t mode, const std::shared_ptr<rwl::TRwLock<TDirectory>>& parent);

        std::vector<char> Data;
        static constexpr NFileType FileType = NFileType::File;
    };



    class TLink : TFile<TDirectory> {
    public:
        TLink()=default;
        static std::shared_ptr<rwl::TRwLock<TLink>> New(const std::string& name, mode_t mode, const std::shared_ptr<rwl::TRwLock<TDirectory>>& parent, const std::filesystem::path& path);

        std::filesystem::path LinkTo;
        static constexpr NFileType FileType = NFileType::Link;
    };

}

