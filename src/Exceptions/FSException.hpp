#pragma once
#include <Exceptions/ExceptionType.hpp>

#include <exception>
#include <filesystem>

namespace fusevfs {

class FSException : public std::exception {
    public:
    FSException(const std::filesystem::path::iterator &begin, const std::filesystem::path::iterator &end, ExceptionTypeEnum type);
    FSException(const std::filesystem::path& path, ExceptionTypeEnum type);
    FSException(const std::string_view& path, ExceptionTypeEnum type);
    [[nodiscard]] const char* what() const noexcept override;
    [[nodiscard]] ExceptionTypeEnum Type() const;

    protected:
    void UpdateMessage(const std::string_view& path, ExceptionTypeEnum type);

    protected:
    ExceptionTypeEnum m_xType = ExceptionTypeEnum::NotDirectory;
    std::string m_sMessage;
};

}

