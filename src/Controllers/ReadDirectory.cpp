#include <Controllers/ReadDirectory.hpp>
#include <Controllers/GetFileParameter.hpp>
#include <Controllers/FindFile.hpp>
#include <Exceptions/FSException.hpp>

namespace fusevfs {

    ReadDirectory::ReadDirectory(const std::filesystem::path& path, void* buffer, fuse_fill_dir_t filler)
        : m_pPath{path}, m_pBuffer{buffer}, m_xFiller{filler} {}

    void ReadDirectory::operator()() {
        const auto res = FindFile::Find(m_pPath);
        return std::visit([this](const auto& obj) { return DoReadDir(obj); }, res);
    }

    void ReadDirectory::DoReadDir(const std::shared_ptr<read_write_lock::RWLock<Directory>>& var) {
        FillerDirectory(var);
    }

    void ReadDirectory::DoReadDir(const std::shared_ptr<read_write_lock::RWLock<RegularFile>>& var) {
        throw FSException(m_pPath, ExceptionTypeEnum::NotDirectory);
    }

    void ReadDirectory::DoReadDir(const std::shared_ptr<read_write_lock::RWLock<Link>>& var) {
        const auto varRead = var->Read();
        const auto dir = FindFile::FindDir(varRead->LinkTo);
        FillerDirectory(dir);
    }

    void ReadDirectory::FillerBuffer(const std::string_view& name) {
        m_xFiller(m_pBuffer, name.data(), NULL, 0, fuse_fill_dir_flags::FUSE_FILL_DIR_PLUS);
    }

    void ReadDirectory::FillerDirectory(const std::shared_ptr<read_write_lock::RWLock<Directory>>& dir) {
        const auto dirRead = dir->Read();
        for(const auto& var : dirRead->Files) {
            const auto name = GetNameParameter{}(var);
            FillerBuffer(name);
        }
    }

}