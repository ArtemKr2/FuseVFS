#pragma once
#include <CLI/CLI.hpp>

namespace fusevfs {

    class FSCLI final : public CLI::App {
    public:
        FSCLI();

    private:
        std::filesystem::path m_mountPoint;
    };
}

