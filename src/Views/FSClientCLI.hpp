#pragma once
#include <CLI/CLI.hpp>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>


namespace fusevfs {

    class FSClientCLI final : public CLI::App {
        std::filesystem::path PipePath;
        std::string FileName;

    public:
        FSClientCLI();

        static std::string FindByNameWithSocket(const std::filesystem::path& socketPath,
                                 const std::string& name) {
            const int sock = socket(AF_UNIX, SOCK_STREAM, 0);
            if (sock < 0) throw std::runtime_error("socket() failed");
            sockaddr_un addr{};
            addr.sun_family = AF_UNIX;
            std::strncpy(addr.sun_path, socketPath.c_str(),
                         sizeof(addr.sun_path)-1);
            if (connect(sock, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
                close(sock);
                throw std::runtime_error("connect() failed");
            }
            // отправляем запрос + '\n'
            std::string req = name + "\n";
            ssize_t w = write(sock, req.data(), req.size());
            if (w < 0) perror("write");
            // читаем весь ответ
            std::string all;
            char tmp[4096];
            ssize_t r;
            while ((r = ::read(sock, tmp, sizeof(tmp))) > 0) {
                all.append(tmp, r);
            }
            close(sock);
            return all;
        }

    };

}

