#include <Views/TFileSystemCLI.hpp>

int main(int argc, char *argv[]) {
    auto cli = fusevfs::TFileSystemCLI();
    CLI11_PARSE(cli, argc, argv);
    return 0;
}