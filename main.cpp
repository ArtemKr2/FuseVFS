#include <Views/FSCLI.hpp>

int main(int argc, char *argv[]) {
    auto cli = fusevfs::FSCLI();
    CLI11_PARSE(cli, argc, argv);
    return 0;
}