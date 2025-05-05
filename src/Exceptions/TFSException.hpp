#pragma once
#include <Exceptions/NNFSExceptionType.hpp>

#include <exception>
#include <filesystem>



namespace fusevfs {

class TFSException : public std::exception {
    public:
    TFSException(const std::filesystem::path::iterator &begin, const std::filesystem::path::iterator &end, NFSExceptionType type);
    TFSException(const std::filesystem::path& path, NFSExceptionType type);
    TFSException(const std::string_view& path, NFSExceptionType type);
    virtual const char* what() const noexcept override;
    [[nodiscard]] NFSExceptionType Type() const;

    protected:
    void UpdateMessage(const std::string_view& path, NFSExceptionType type);

    protected:
    NFSExceptionType m_xType = NFSExceptionType::NotDirectory;
    std::string m_sMessage;
};

}

