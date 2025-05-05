#include <Views/FSClientCLI.hpp>

int main(int argc, char *argv[]) {
    auto cli = fusevfs::FSClientCLI();
    CLI11_PARSE(cli, argc, argv);
    return 0;
}