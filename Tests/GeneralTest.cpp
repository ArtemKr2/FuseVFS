#include <gtest/gtest.h>
#include <Views/FSClientCLI.hpp>

#include <filesystem>
#include <cstdlib>
#include <unistd.h>
#include <csignal>
#include <thread>
#include <chrono>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std::chrono_literals;

static constexpr const char* FuseBinary = FUSE_MAIN_BIN_PATH;

static constexpr char MountPoint[] = "/mnt/custom_fs2";

static std::filesystem::path SocketPath;

static pid_t s_fusePid = -1;

// ——————————————

class TFileSystemTestFixture : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        std::filesystem::remove_all(MountPoint);
        std::filesystem::create_directory(MountPoint);

        SocketPath = "/tmp/fusevfs.sock";

        int r = std::system(("fusermount3 -u " + std::string(MountPoint) + " 2>/dev/null || true").c_str());
        if (r == -1) {
            perror("system");
        }

        r = std::system(("mkdir -p " + std::string(MountPoint)).c_str());
        if (r == -1) {
            perror("system");
        }

        // s_fifoDummy = open(FifoPath.c_str(), O_RDWR | O_NONBLOCK);
        // if (s_fifoDummy < 0) { perror("open fifo"); exit(1); }

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
    }

    static void TearDownTestSuite() {
        if (s_fusePid > 0) {
            kill(s_fusePid, SIGTERM);
            std::this_thread::sleep_for(200ms);
        }
        // if (s_fifoDummy >= 0) close(s_fifoDummy);

        int r = std::system(("fusermount3 -u " + std::string(MountPoint) + " 2>/dev/null || true").c_str());
        if (r == -1) {
            perror("system");
        }

        std::filesystem::remove_all(MountPoint);

    }
};

TEST_F(TFileSystemTestFixture, RegularFile) {
    const auto filePath = MountPoint / std::filesystem::path("text.txt");
    std::ofstream(filePath.c_str()) << "information";
    EXPECT_TRUE(std::filesystem::is_regular_file(filePath));
    std::string r;
    std::ifstream(filePath.c_str()) >> r;
    EXPECT_EQ(r.find_first_of("information"), 0);
}

TEST_F(TFileSystemTestFixture, Link) {
    const auto filePath = MountPoint / std::filesystem::path("linked");
    std::ofstream give_me_a_name(filePath.c_str());
    const auto linkPath = MountPoint / std::filesystem::path("link");
    std::filesystem::create_symlink(filePath, linkPath);
    EXPECT_TRUE(std::filesystem::is_symlink(linkPath));
    EXPECT_EQ(filePath, std::filesystem::read_symlink(linkPath));
}

TEST_F(TFileSystemTestFixture, Directory) {
    const auto dirPath = MountPoint / std::filesystem::path("dir");
    std::filesystem::create_directory(dirPath);
    EXPECT_TRUE(std::filesystem::is_directory(dirPath));
    const auto filePath = dirPath / std::filesystem::path("indirFile");
    {
        std::ofstream(filePath.c_str());
    }

    {
        SCOPED_TRACE("CheckFileInsideDirectory");
        const auto fileIt = std::filesystem::directory_iterator(dirPath);
        EXPECT_TRUE(fileIt->is_regular_file());
        EXPECT_EQ(fileIt->path().filename(), "indirFile");
        EXPECT_NE(fileIt, end(fileIt));
    }

    {
        SCOPED_TRACE("CheckDeleteFileInsideDirectory");
        std::filesystem::remove(filePath);
        const auto fileIt = std::filesystem::directory_iterator(dirPath);
        EXPECT_EQ(fileIt, end(fileIt));
    }
}

TEST_F(TFileSystemTestFixture, FindByName) {
    std::filesystem::create_directory(std::filesystem::path(MountPoint) / "bar");
    std::filesystem::create_directory(std::filesystem::path(MountPoint) / "bar/bar");
    std::filesystem::create_directory(std::filesystem::path(MountPoint) / "bar/bar/baz");
    std::filesystem::create_directory(std::filesystem::path(MountPoint) / "bar/bar/baz/bar");
    const auto filePath = std::filesystem::path(MountPoint) / "bar/testfile.txt";
    std::ofstream(filePath.c_str()) << "information";
    const std::string result = fusevfs::FSClientCLI::FindByNameWithSocket(SocketPath, "testfile.txt");
    std::cout << result << std::endl;
    // EXPECT_STREQ(buffer.data(), "/bar\n/bar/bar\n/bar/bar/baz/bar\n");
    EXPECT_STREQ(result.c_str(), "/bar/testfile.txt\n");
}

 TEST_F(TFileSystemTestFixture, FileAccess) {
     const auto filePath = std::filesystem::path(MountPoint) / std::filesystem::path("accessFile");
     {
         auto f = std::ofstream(filePath.c_str());
     }
     {
         SCOPED_TRACE("AllPermissionGranted");
         std::filesystem::permissions(filePath, std::filesystem::perms::owner_all, std::filesystem::perm_options::add);
         const auto file = std::fstream(filePath.c_str(), std::ios::ate | std::ios::in);
         EXPECT_TRUE(file.is_open());
         const auto perms = std::filesystem::status(filePath).permissions();
         EXPECT_EQ(perms & std::filesystem::perms::owner_read, std::filesystem::perms::owner_read);
         EXPECT_EQ(perms & std::filesystem::perms::owner_write, std::filesystem::perms::owner_write);
         EXPECT_EQ(perms & std::filesystem::perms::owner_exec, std::filesystem::perms::owner_exec);
     }
     {
         SCOPED_TRACE("WriteProtected");
         std::filesystem::permissions(filePath, std::filesystem::perms::owner_write, std::filesystem::perm_options::remove);
         EXPECT_EQ(std::filesystem::status(filePath).permissions() & std::filesystem::perms::owner_write, std::filesystem::perms::none);
         auto file = std::fstream(filePath.c_str(), std::ios::ate);
         EXPECT_FALSE(file.is_open());
         std::filesystem::permissions(filePath, std::filesystem::perms::owner_write, std::filesystem::perm_options::add);
     }
     {
         SCOPED_TRACE("ReadProtected");
         std::filesystem::permissions(filePath, std::filesystem::perms::owner_read, std::filesystem::perm_options::remove);
         EXPECT_EQ(std::filesystem::status(filePath).permissions() & std::filesystem::perms::owner_read, std::filesystem::perms::none);
         const auto file = std::fstream(filePath.c_str(), std::ios::in);
         EXPECT_FALSE(file.is_open());
         std::filesystem::permissions(filePath, std::filesystem::perms::owner_read, std::filesystem::perm_options::add);
     }
     {
         SCOPED_TRACE("ExecuteProtected");
         const auto cmdName = std::string("mkdir");
         const auto cmdPath = std::filesystem::path(MountPoint)  / cmdName;
         int r = std::system(("cp /bin/" + cmdName + " " + cmdPath.native()).c_str());
         if (r == -1) {
             perror("system");
         }
         //std::filesystem::copy(std::filesystem::path("/bin") / cmdName, cmdPath);
         std::filesystem::permissions(cmdPath,
             std::filesystem::perms::owner_all | std::filesystem::perms::group_all | std::filesystem::perms::others_all,
             std::filesystem::perm_options::add);
         const auto cmdOne = cmdPath.native() + " " + (std::filesystem::path(MountPoint)  / "cmdDir1").c_str();
         EXPECT_EQ(std::system(cmdOne.c_str()), 0);
         std::filesystem::permissions(cmdPath,
             std::filesystem::perms::owner_exec | std::filesystem::perms::group_exec | std::filesystem::perms::others_exec,
             std::filesystem::perm_options::remove);
         EXPECT_EQ(std::filesystem::status(cmdPath).permissions() & std::filesystem::perms::owner_exec, std::filesystem::perms::none);
         const auto cmdTwo = cmdPath.native() + " " + (std::filesystem::path(MountPoint)  / "cmdDir2").c_str();
         EXPECT_EQ(std::system(cmdTwo.c_str()), 32256);
         std::filesystem::permissions(cmdPath,
             std::filesystem::perms::owner_exec | std::filesystem::perms::group_exec | std::filesystem::perms::others_exec,
             std::filesystem::perm_options::add);
     }
 }

  TEST_F(TFileSystemTestFixture, DirectoryAccess) {
      const auto dirPath = std::filesystem::path(MountPoint)  / std::filesystem::path("accessDirectory");
      std::filesystem::create_directory(dirPath);
      {
          SCOPED_TRACE("AllPermissionGranted");
          std::filesystem::permissions(dirPath, std::filesystem::perms::owner_all, std::filesystem::perm_options::add);
          const auto perms = std::filesystem::status(dirPath).permissions();
          EXPECT_EQ(perms & std::filesystem::perms::owner_read, std::filesystem::perms::owner_read);
          EXPECT_EQ(perms & std::filesystem::perms::owner_write, std::filesystem::perms::owner_write);
          EXPECT_EQ(perms & std::filesystem::perms::owner_exec, std::filesystem::perms::owner_exec);
          const auto testSubDir = dirPath / "accessDirectorySubDir";
          std::filesystem::create_directory(testSubDir);
          EXPECT_TRUE(std::filesystem::exists(testSubDir));
          auto it = std::filesystem::directory_iterator(dirPath);
          EXPECT_EQ(std::distance(it, std::filesystem::end(it)), 1);
      }
      {
          SCOPED_TRACE("ExecuteProtected");
          std::filesystem::permissions(dirPath, std::filesystem::perms::owner_exec, std::filesystem::perm_options::remove);
          EXPECT_EQ(std::filesystem::status(dirPath).permissions() & std::filesystem::perms::owner_exec, std::filesystem::perms::none);
          {
              // still can read this directory
              auto isCaughtError = false;
              try {
                  auto it = ++std::filesystem::directory_iterator(dirPath);
                  isCaughtError = false;
              } catch(const std::filesystem::filesystem_error& ex) {
                  isCaughtError = true;
              }
              EXPECT_FALSE(isCaughtError);
          }
          {
              // but can not move into
              auto isCaughtError = false;
              try {
                  // tyring to access "accessDirectorySubDir"
                  auto it = ++std::filesystem::recursive_directory_iterator(dirPath);
              } catch(const std::filesystem::filesystem_error& ex) {
                  isCaughtError = true;
              }
              EXPECT_TRUE(isCaughtError);
          }
          std::filesystem::permissions(dirPath, std::filesystem::perms::owner_exec, std::filesystem::perm_options::add);
      }
      {
          SCOPED_TRACE("WriteProtected");
          std::filesystem::permissions(dirPath, std::filesystem::perms::owner_write, std::filesystem::perm_options::remove);
          EXPECT_EQ(std::filesystem::status(dirPath).permissions() & std::filesystem::perms::owner_write, std::filesystem::perms::none);
          const auto testFileTwoPath = dirPath / "accessDirectoryTestFileTwo";
          auto file = std::ofstream(testFileTwoPath);
          EXPECT_FALSE(file.is_open());
          std::filesystem::permissions(dirPath, std::filesystem::perms::owner_write, std::filesystem::perm_options::add);
      }
      {
          SCOPED_TRACE("ReadProtected");
          std::filesystem::permissions(dirPath, std::filesystem::perms::owner_read, std::filesystem::perm_options::remove);
          EXPECT_EQ(std::filesystem::status(dirPath).permissions() & std::filesystem::perms::owner_read, std::filesystem::perms::none);
          const auto file = std::fstream(dirPath.c_str(), std::ios::in);
          auto isCaughtError = false;
          try {
              auto it = std::filesystem::directory_iterator(dirPath);
          } catch(const std::filesystem::filesystem_error& ex) {
              isCaughtError = true;
          }
          EXPECT_TRUE(isCaughtError);
          std::filesystem::permissions(dirPath, std::filesystem::perms::owner_read, std::filesystem::perm_options::add);
      }
  }

 TEST_F(TFileSystemTestFixture, LinkAccess) {
     const auto linkPath = std::filesystem::path(MountPoint)  / "accessLink";
     const auto filePath = std::filesystem::path(MountPoint)  / "accessLinkFile";
     {
         auto f = std::ofstream(filePath.c_str());
     }
     std::filesystem::create_symlink(filePath, linkPath);
     {
         SCOPED_TRACE("AllPermissionGranted");
         std::filesystem::permissions(linkPath, std::filesystem::perms::owner_all, std::filesystem::perm_options::add);
         const auto file = std::fstream(linkPath.c_str(), std::ios::out | std::ios::in);
         EXPECT_TRUE(file.is_open());
         const auto perms = std::filesystem::status(linkPath).permissions();
         EXPECT_EQ(perms & std::filesystem::perms::owner_read, std::filesystem::perms::owner_read);
         EXPECT_EQ(perms & std::filesystem::perms::owner_write, std::filesystem::perms::owner_write);
         EXPECT_EQ(perms & std::filesystem::perms::owner_exec, std::filesystem::perms::owner_exec);
     }
     {
         SCOPED_TRACE("WriteProtected");
         std::filesystem::permissions(linkPath, std::filesystem::perms::owner_write, std::filesystem::perm_options::remove);
         EXPECT_EQ(std::filesystem::status(linkPath).permissions() & std::filesystem::perms::owner_write, std::filesystem::perms::none);
         auto file = std::fstream(linkPath.c_str(), std::ios::ate);
         EXPECT_FALSE(file.is_open());
         std::filesystem::permissions(linkPath, std::filesystem::perms::owner_write, std::filesystem::perm_options::add);
     }
     {
         SCOPED_TRACE("ReadProtected");
         std::filesystem::permissions(linkPath, std::filesystem::perms::owner_read, std::filesystem::perm_options::remove);
         EXPECT_EQ(std::filesystem::status(linkPath).permissions() & std::filesystem::perms::owner_read, std::filesystem::perms::none);
         const auto file = std::fstream(linkPath.c_str(), std::ios::in);
         EXPECT_FALSE(file.is_open());
         std::filesystem::permissions(linkPath, std::filesystem::perms::owner_read, std::filesystem::perm_options::add);
     }
     {
         SCOPED_TRACE("ExecuteProtected");
         const auto cmdPath = std::filesystem::path(MountPoint)  / "mkdir2";
         int r = std::system((" cp /bin/mkdir " + cmdPath.native()).c_str());
         if (r == -1) {
             perror("system");
         }
         const auto cmdLink = std::filesystem::path(MountPoint)  / "mkdir2Link";
         std::filesystem::create_symlink(cmdPath, cmdLink);
         std::filesystem::permissions(cmdLink,
             std::filesystem::perms::owner_all | std::filesystem::perms::group_all | std::filesystem::perms::others_all,
             std::filesystem::perm_options::add);
         const auto cmdOne = cmdLink.native() + " " + (std::filesystem::path(MountPoint)  / "cmdLinkDir1").c_str();
         EXPECT_EQ(std::system(cmdOne.c_str()), 0);
         std::filesystem::permissions(cmdLink,
             std::filesystem::perms::owner_exec | std::filesystem::perms::group_exec | std::filesystem::perms::others_exec,
             std::filesystem::perm_options::remove);
         EXPECT_EQ(std::filesystem::status(cmdLink).permissions() & std::filesystem::perms::owner_exec, std::filesystem::perms::none);
         const auto cmdTwo = cmdLink.native() + " " + (std::filesystem::path(MountPoint)  / "cmdLinkDir2").c_str();
         EXPECT_EQ(std::system(cmdTwo.c_str()), 32256);
         std::filesystem::permissions(cmdLink,
             std::filesystem::perms::owner_exec | std::filesystem::perms::group_exec | std::filesystem::perms::others_exec,
             std::filesystem::perm_options::add);
     }
}