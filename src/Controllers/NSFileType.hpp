#pragma once
#include <Models/FileType.hpp>
#include <Models/FileObjects.hpp>

namespace fusevfs::NSFileType {

constexpr FileTypeEnum Get(const FileObjectSharedVariant& var) {
    return std::visit([](const auto& file) { return Get(file); }, var);
}
constexpr FileTypeEnum Get(const FileObjectSharedRWConcept auto& var) {
    return std::remove_reference_t<decltype(var)>::element_type::InnerType::FileType;
}
constexpr FileTypeEnum Get(const FileObjectGuardConcept auto& var) {
    return std::remove_reference_t<decltype(var)>::InnerType::FileType;
}

}

