#pragma once
#if defined(_WIN32) || defined(_WIN64)
#include <vector>
#include <map>
#include <memory>
#include <string>
#include <atomic>
#include <boost\thread\shared_mutex.hpp>

namespace helpers {

/// @brief Windows-native information about logical partitions and all data associated with them:
/// Physical drive index, type of disk, type of placement (fixed/external/network/etc...)
class NativePartititonInformation{
public:

    enum PlacementType {
        FixedDisk,
        RemovebleDisk,
        NoRootDir,
        NetworkDisk,
        CDROM,
        RamDisk,
        UnknownPlacement
    };

    enum DiskType
    {
        HDD,
        SSD,
        UnknownType
    };


    /// @brief Basic Windows partition-associated info
    struct NativePartititon
    {
        NativePartititon() = default;
        NativePartititon(int drv_ind, int plc_type, int dsk_type, char drv_letter) 
            : drive_index(drv_ind), placement_type(plc_type), disk_type(dsk_type), drive_letter(drv_letter) {}

        // Up to 0, physical drive; -1 in invalid value
        int drive_index = 0;

        // fixed/external/network/etc...
        int placement_type = 0;

        // HDD/SSD
        int disk_type = 0;

        // A-Z in upper case
        char drive_letter = '\0';
    };

    /// @brief partition information collected here. C-tor does not throw, but may work long time
    /// Do not use in in the GUI thread
    NativePartititonInformation();

    /// @brief Default, just satisfy compiler
    ~NativePartititonInformation() = default;

    /// @brief Get the current 'slice' of partition information
    /// If the partition information is updated during reading, it won't be returned
    /// Method has read lock
    std::vector<NativePartititon> enumerate_partititons() const;

    /// @brief Get the current 'slice' of partition information
    /// Method has read lock in the PImpl layer
    std::vector<NativePartititon> enumerate_drive_partititons(int drive_index) const;

    /// @brief Get the some logical drive information by the path
    /// If the partition information is updated during reading, it won't be returned
    /// Method has read lock
    std::unique_ptr<NativePartititon> get_file_partition_info(const std::string& filepath) const;

    /// @brief Update information about logical partitions
    /// Called from constructor, other use case is update partition information
    /// Method has write lock
    void collect_partititon_information();

    /// @brief 
    const std::vector<int> get_physical_drives() const;

    /// @brief 
    size_t root_string_size() const { return 1; }

private:

    /// Convert Win API defines to enum
    DiskType get_drive_type(int physical_drive_number) const;

    /// Convert drive root to logical partition placement
    PlacementType get_drive_placement(const std::wstring& windows_drive_root) const;

    /// Convert drive letter to physical drive index
    int get_physical_drive_number(char windows_drive_letter) const;

    /// Partition information itself
    std::map<char, NativePartititon> partititon_info_;

    /// Partition information by physical drive
    std::map<int, std::vector<NativePartititon>> physical_disk_info_;

    /// Map WinAPI defines to application enum
    static std::map<int, PlacementType> system_codes_2_placement_type_;

    // protect from updating disk information during reading
    mutable boost::shared_mutex m_;
};

} // namespace helpers

#endif // defined(_WIN32) || defined(_WIN64)