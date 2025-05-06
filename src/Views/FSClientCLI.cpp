#include <Views/FSClientCLI.hpp>

namespace fusevfs {

    FSClientCLI::FSClientCLI() : CLI::App("FindByName") {
        add_option("--pipe-point,-p", PipePath, "Pipe point")
            ->required(true);
        add_option("--file-name,-f", FileName, "File name")
            ->required(true);
        parse_complete_callback([this] {
            const auto response = FindByNameWithSocket(PipePath, FileName);
            std::cout << response;
        });
    }


}


