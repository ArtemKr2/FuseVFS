#include <algorithm>
#include <iostream>

#include <Controllers/NSFileAttributes.hpp>
#include <Controllers/TGetFileParameter.hpp>

namespace fusevfs::NSFileAttributes {

    inline void FillTimespec(std::chrono::system_clock::time_point tp,
                         timespec& out)
    {
        using namespace std::chrono;
        auto s  = time_point_cast<seconds>(tp);
        auto ns = duration_cast<nanoseconds>(tp - s).count();
        out.tv_sec  = static_cast<time_t>(s.time_since_epoch().count());
        out.tv_nsec = static_cast<long>(ns);
    }

    void UpdateSize(const read_write_lock::RWLockReadGuard<TDirectory>&, struct stat* st) {
        st->st_size = 4096;
    }

    void UpdateSize(const read_write_lock::RWLockReadGuard<TRegularFile>& varRead, struct stat* st) {
        st->st_size = static_cast<off_t>(varRead->Data.size());
    }

    void UpdateSize(const read_write_lock::RWLockReadGuard<TLink>& varRead, struct stat* st) {
        st->st_size = static_cast<off_t>(std::string_view(varRead->LinkTo.c_str()).size());
    }

// void GetGeneral(const CSharedRwFileObject auto& var, struct stat* st) {
//     const auto varRead = var->Read();
//     st->st_mode = TGetInfoMode{}(varRead);
//     st->st_gid = TGetInfoGid{}(varRead);
//     st->st_uid = TGetInfoUid{}(varRead);
//     st->st_nlink = var.use_count();
//     UpdateSize(varRead, st);
// }
//
    void GetTotal(const CSharedRwFileObject auto& var, struct stat* st)
    {
        auto rd = var->Read();                     // shared-lock

        // type + mode
        st->st_mode = TGetInfoMode{}(rd);

        // uid + gid
        st->st_uid  = TGetInfoUid{}(rd);
        st->st_gid  =  TGetInfoGid{}(rd);

        // update size
        UpdateSize(rd, st);

        // set block size
        static constexpr std::size_t BLK = 512;
        st->st_blksize = 4096;
        st->st_blocks  = (st->st_size + BLK - 1) / BLK;

        // found number of hard-links
        using Inner = typename std::remove_reference_t<
                          decltype(var)>::element_type::InnerType;

        if constexpr(std::same_as<Inner, TDirectory>)
        {
            std::size_t subDirs = std::count_if(
                rd->Files.begin(), rd->Files.end(),
                [](auto& v){ return std::holds_alternative<std::shared_ptr<read_write_lock::RWLock<TDirectory>>>(v); });
            st->st_nlink = 2 + subDirs;
        }
        else
            st->st_nlink = 1;


        FillTimespec(TGetInfoChanged{}(var),  st->st_ctim); // ctime / birth
        FillTimespec(TGetInfoModified{}(var), st->st_mtim); // mtime  (ls -l)
        FillTimespec(TGetInfoAccessed{}(var), st->st_atim); // atime  (ls -lu)
    }

void Get(const ASharedFileVariant& var, struct stat* st) {
    std::visit([st](const auto& file) { GetTotal(file, st); }, var);
}

}
