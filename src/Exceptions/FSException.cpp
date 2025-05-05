#include <Exceptions/FSException.hpp>
#include <magic_enum.hpp>

namespace fusevfs {

FSException::FSException(const std::filesystem::path::iterator &begin,
    const std::filesystem::path::iterator &end,
    ExceptionTypeEnum type)
    : m_xType{type} {

    auto path = std::filesystem::path();
    for(auto it = begin; it != end; ++it) path.append(it->c_str());
    UpdateMessage(path.c_str(), type);
}

const char* FSException::what() const noexcept { return m_sMessage.c_str(); }
ExceptionTypeEnum FSException::Type() const { return m_xType; }

FSException::FSException(const std::filesystem::path& path, ExceptionTypeEnum type) {
    UpdateMessage(path.c_str(), type);
}

FSException::FSException(const std::string_view& path, ExceptionTypeEnum type) {
    UpdateMessage(path, type);
}

void FSException::UpdateMessage(const std::string_view& path, ExceptionTypeEnum type) {
    m_sMessage = static_cast<std::string>(magic_enum::enum_name(type)) + ": " + path.data();
}

}



