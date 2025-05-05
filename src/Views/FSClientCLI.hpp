#pragma once
#include <CLI/CLI.hpp>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>


namespace fusevfs {

    class TFileSystemClientCLI final : public CLI::App {
        std::filesystem::path m_xPipePath;
        std::string m_sFileName;
        static constexpr std::string_view s_sError = "Can not open the pipe for writing";

    public:
        TFileSystemClientCLI();

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

     //    template<unsigned long BufferSize>
     // static void FindByName(const std::filesystem::path& pipePath, const std::string& fileName, std::array<char, BufferSize>& buffer) {
     //        {
     //            std::cerr << "[FindByName] before fOut = std::ofstream(pipePath);" << std::endl;
     //            auto fOut = std::ofstream(pipePath);
     //            std::cerr << "[FindByName] after fOut = std::ofstream(pipePath);" << std::endl;
     //            std::cerr << "[FindByName] before !fOut.is_open();" << std::endl;
     //            if(!fOut.is_open()) {
     //                throw std::invalid_argument(s_sError.data());
     //            }
     //            std::cerr << "[FindByName] after !fOut.is_open();" << std::endl;
     //            std::cerr << "[FindByName] before fOut << fileName;" << std::endl;
     //            fOut << fileName;
     //            std::cerr << "[FindByName] after fOut << fileName;" << std::endl;
     //        }
     //        {
     //            std::cerr << "[FindByName] before fIn = std::ifstream(pipePath);" << std::endl;
     //            auto fIn = std::ifstream(pipePath);
     //            std::cerr << "[FindByName] after fIn = std::ifstream(pipePath);" << std::endl;
     //            std::cerr << "[FindByName] before !fIn.is_open();" << std::endl;
     //            if(!fIn.is_open()) {
     //                throw std::invalid_argument(s_sError.data());
     //            }
     //            std::cerr << "[FindByName] after !fIn.is_open();" << std::endl;
     //            std::cerr << "[FindByName] before !fIn.read();" << std::endl;
     //            fIn.read(buffer.data(), buffer.size());
     //            std::cerr << "[FindByName] after !fIn.read();" << std::endl;
     //        }
     //    }

    };

}

