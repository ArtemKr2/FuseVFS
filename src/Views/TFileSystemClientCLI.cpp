#include <Views/TFileSystemClientCLI.hpp>

namespace fusevfs {

    static constexpr unsigned long bufferSize = 1000;

    TFileSystemClientCLI::TFileSystemClientCLI() : CLI::App("FindByName") {
        add_option("--pipe-point,-p", m_xPipePath, "Pipe point")
            ->required(true);
        add_option("--file-name,-f", m_sFileName, "File name")
            ->required(true);
        parse_complete_callback([this] {
            const auto response = FindByNameWithSocket(m_xPipePath, m_sFileName);
            std::cout << response;
        });
    }


}


