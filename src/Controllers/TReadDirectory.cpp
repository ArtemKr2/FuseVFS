#include <Controllers/TReadDirectory.hpp>
#include <Controllers/TGetFileParameter.hpp>
#include <Controllers/NSFindFile.hpp>
#include <Exceptions/TFSException.hpp>

namespace fusevfs {

    TReadDirectory::TReadDirectory(const std::filesystem::path& path, void* buffer, fuse_fill_dir_t filler)
        : m_pPath{path}, m_pBuffer{buffer}, m_xFiller{filler} {}

    void TReadDirectory::operator()() {
        const auto res = NSFindFile::Find(m_pPath);
        return std::visit([this](const auto& obj) { return DoReadDir(obj); }, res);
    }

    void TReadDirectory::DoReadDir(const std::shared_ptr<read_write_lock::RWLock<TDirectory>>& var) {
        FillerDirectory(var);
    }

    void TReadDirectory::DoReadDir(const std::shared_ptr<read_write_lock::RWLock<TRegularFile>>& var) {
        throw TFSException(m_pPath, NFSExceptionType::NotDirectory);
    }

    void TReadDirectory::DoReadDir(const std::shared_ptr<read_write_lock::RWLock<TLink>>& var) {
        const auto varRead = var->Read();
        const auto dir = NSFindFile::FindDir(varRead->LinkTo);
        FillerDirectory(dir);
    }

    void TReadDirectory::FillerBuffer(const std::string_view& name) {
        m_xFiller(m_pBuffer, name.data(), NULL, 0, fuse_fill_dir_flags::FUSE_FILL_DIR_PLUS);
    }

    void TReadDirectory::FillerDirectory(const std::shared_ptr<read_write_lock::RWLock<TDirectory>>& dir) {
        const auto dirRead = dir->Read();
        for(const auto& var : dirRead->Files) {
            const auto name = TGetInfoName{}(var);
            FillerBuffer(name);
        }
    }

}