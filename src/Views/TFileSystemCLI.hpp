#pragma once
#include <CLI/CLI.hpp>

namespace fusevfs {

    class TFileSystemCLI final : public CLI::App {
    public:
        TFileSystemCLI();

    private:
        std::filesystem::path m_mountPoint;
    };
}

