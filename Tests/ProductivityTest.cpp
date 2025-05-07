#include <Views/FSCLI.hpp>
#include <Controllers/FindFile.hpp>
#include <gtest/gtest.h>

#include <mntent.h>
#include <sys/vfs.h>
#include <filesystem>
#include <thread>
#include <chrono>

#include <unistd.h>
#include <Views/FSClientCLI.hpp>

using namespace std::chrono_literals;

static constexpr const char* FuseBinary   = FUSE_MAIN_BIN_PATH;
static constexpr char        MountPoint[] = "/mnt/custom_fs";
static std::filesystem::path              SocketPath;
static std::filesystem::path              s_xTestFolderPath;
static pid_t                 s_fusePid     = -1;

bool is_fuse_mounted(const char* mountpoint) {
    struct statfs fsinfo;
    if (statfs(mountpoint, &fsinfo) != 0) return false;

    return fsinfo.f_type == 0x65735546;
}

class FSTestProductivity : public ::testing::Test {
protected:

    static constexpr unsigned s_uMaxDepth = 4;

    static void CreateTree(const std::filesystem::path& base, unsigned depth = 0) {
        if (depth >= s_uMaxDepth) return;
        for (char c = 'A'; c <= 'Z'; ++c) {
            std::filesystem::path next = base / std::string(1, c);
            std::filesystem::create_directory(next);
            CreateTree(next, depth+1);
        }
    }

    static void SetUpTestSuite() {
        std::cerr << "Started SetUpTestSuite()" << std::endl;
        int r = std::system(("fusermount3 -u " + std::string(MountPoint) + " 2>/dev/null || true").c_str());
        if (r == -1) {
            perror("system");
        }
        std::filesystem::remove_all(MountPoint);
        std::filesystem::create_directory(MountPoint);

        s_xTestFolderPath = std::filesystem::path("/home/artem4ik/test23") / "TestTree";
       // std::filesystem::remove_all(s_xTestFolderPath);
       // std::filesystem::create_directory(s_xTestFolderPath);


        SocketPath = "/tmp/fusevfs.sock";

        s_fusePid = fork();
        if (s_fusePid == 0) {
            execl(FuseBinary,
                  FuseBinary,
                  "-f",
                  "-m", MountPoint,
                  "-p", SocketPath.c_str(),
                  nullptr);
            perror("execl");
            _exit(1);
        }
        std::this_thread::sleep_for(1s);
        bool mounted = false;
        for (int i = 0; i < 50; ++i) {
            if (is_fuse_mounted(MountPoint)) {
                std::cerr << "fuse mounted! " << i << std::endl;
                mounted = true;
                break;
            }
            std::this_thread::sleep_for(100ms);
        }

        ASSERT_TRUE(mounted) << "FUSE never mounted on " << MountPoint;
        CreateTree(MountPoint);
        //CreateTree(s_xTestFolderPath);
    }

    static void TearDownTestSuite() {
        if (s_fusePid > 0) {
            kill(s_fusePid, SIGTERM);
            std::this_thread::sleep_for(200ms);
        }
        int r = std::system(("fusermount3 -u " + std::string(MountPoint) + " 2>/dev/null || true").c_str());
        if (r == -1) {
            perror("system");
        }
        std::filesystem::remove_all(MountPoint);

        //std::filesystem::remove_all(s_xTestFolderPath);
    }
};

TEST_F(FSTestProductivity, FindFileTimeVFS) {
    auto t0 = std::chrono::steady_clock::now();
    const std::string res = fusevfs::FSClientCLI::FindByNameWithSocket(SocketPath, "F");
    auto t1 = std::chrono::steady_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    std::vector<std::string> paths;
    std::istringstream in(res);
    for (std::string line; std::getline(in, line); ) {
        if (!line.empty()) {
            paths.push_back(line);
        }
    }

    EXPECT_EQ(paths.size(), 18279);
    for (auto& p : paths) {
        EXPECT_EQ(std::filesystem::path(p).filename(), "F");
    }
    std::cerr << "[Timing] FindFuseVFS " << dur << " ms\n";
}

// 18279
TEST_F(FSTestProductivity, FindFileTimeGeneralFS) {
    auto t0 = std::chrono::steady_clock::now();
    size_t count = std::ranges::count_if(
        std::filesystem::recursive_directory_iterator(s_xTestFolderPath),
        [](auto const& entry){
            return entry.path().filename() == "F";
        });
    auto t1 = std::chrono::steady_clock::now();
    EXPECT_EQ(count, 18279);
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    std::cerr << "[Timing] FindRegularFS " << dur << " ms\n";
}
