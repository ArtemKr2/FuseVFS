#pragma once
#include <variant>
#include <vector>
#include <filesystem>

#include <Models/File.hpp>
#include <Models/FileType.hpp>
#include <ReadWriteLock/RWLock.hpp>

namespace fusevfs {

    class RegularFile;
    class Directory;
    class Link;


    template<typename T>
    concept FileObjectConcept = std::same_as<T, Directory>
        || std::same_as<T, RegularFile>
        || std::same_as<T, Link>;

    template<typename T>
    concept FileObjectReadGuardConcept = std::same_as<T, read_write_lock::RWLockReadGuard<Directory>>
        || std::same_as<T, read_write_lock::RWLockReadGuard<RegularFile>>
        || std::same_as<T, read_write_lock::RWLockReadGuard<Link>>;

    template<typename T>
    concept FileObjectWriteGuardConcept = std::same_as<T, read_write_lock::RWLockWriteGuard<Directory>>
        || std::same_as<T, read_write_lock::RWLockWriteGuard<RegularFile>>
        || std::same_as<T, read_write_lock::RWLockWriteGuard<Link>>;

    template<typename T>
    concept FileObjectGuardConcept = FileObjectReadGuardConcept<T> || FileObjectWriteGuardConcept<T>;

    template<typename T>
    concept FileObjectSharedRWConcept = std::same_as<T, std::shared_ptr<read_write_lock::RWLock<Directory>>>
        || std::same_as<T, std::shared_ptr<read_write_lock::RWLock<RegularFile>>>
        || std::same_as<T, std::shared_ptr<read_write_lock::RWLock<Link>>>;

    using FileObjectSharedVariant = std::variant<
    std::shared_ptr<read_write_lock::RWLock<Directory>>,
    std::shared_ptr<read_write_lock::RWLock<RegularFile>>,
    std::shared_ptr<read_write_lock::RWLock<Link>>>;

    class Directory : public File<Directory> {
    public:
        Directory()=default;
        static std::shared_ptr<read_write_lock::RWLock<Directory>> New(const std::string& name, mode_t mode, const std::shared_ptr<read_write_lock::RWLock<Directory>>& parent);

        std::vector<FileObjectSharedVariant> Files;
        static constexpr FileTypeEnum FileType = FileTypeEnum::Directory;
    };

    class RegularFile : public File<Directory> {
    public:
        RegularFile()=default;
        static std::shared_ptr<read_write_lock::RWLock<RegularFile>> New(const std::string& name, mode_t mode, const std::shared_ptr<read_write_lock::RWLock<Directory>>& parent);

        std::vector<char> Data;
        static constexpr FileTypeEnum FileType = FileTypeEnum::File;
    };



    class Link : File<Directory> {
    public:
        Link()=default;
        static std::shared_ptr<read_write_lock::RWLock<Link>> New(const std::string& name, mode_t mode, const std::shared_ptr<read_write_lock::RWLock<Directory>>& parent, const std::filesystem::path& path);

        std::filesystem::path LinkTo;
        static constexpr FileTypeEnum FileType = FileTypeEnum::Link;
    };

}

