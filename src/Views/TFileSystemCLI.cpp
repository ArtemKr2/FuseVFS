#include <Views/TFileSystemCLI.hpp>
#include <Controllers/TFileSystem.hpp>

namespace fusevfs {


    TFileSystemCLI::TFileSystemCLI()
        : CLI::App{"FuseVFS"}
    {
        // Создаём опции и сохраняем указатели
        auto* fg         = add_flag("-f,--foreground",   "Keep process in foreground");
        auto* offThreads = add_flag("-o,--off-threads",  "Disable multithreading");
        auto* dbg        = add_flag("-d,--debug",        "Show debug messages")
                              ->needs(fg);

        add_option("-m,--mount-point", m_mountPoint, "Mount point")
            ->required()
            ->check(CLI::ExistingDirectory);

        add_option("-p,--pipe-point", TFileSystem::SocketPath, "UNIX-socket path")
            ->required();

        parse_complete_callback([this, fg, offThreads, dbg]() {
            std::vector<const char*> args;
            // argv[0]
            args.push_back(this->get_name().c_str());

            // Проверяем флаги через as<bool>()
            if (fg->as<bool>())         args.push_back("-f");
            if (dbg->as<bool>())        args.push_back("-d");
            if (offThreads->as<bool>()) args.push_back("-o");

            args.push_back(m_mountPoint.c_str());

            // Запускаем FUSE
            TFileSystem::Init(
                static_cast<int>(args.size()),
                const_cast<char**>(args.data())
            );
        });
    }

}
