#pragma once
#if defined(_WIN32) || defined(_WIN64)
#include <vector>
#include <map>
#include <memory>
#include <string>
#include <atomic>
#include <shared_mutex>

namespace helpers {

/// @brief Windows-native information about logical partitions and all data associated with them:
/// Physical drive index, type of disk, type of placement (fixed/external/network/etc...)
class PartititonInformation{
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
        /// @brief Create by default in STL container
        NativePartititon() = default;
        NativePartititon(int drv_ind, int plc_type, int dsk_type, char drv_letter) 
            : drive_index(drv_ind), placement_type(plc_type), disk_type(dsk_type), drive_letter(drv_letter) {}

        /// Up to 0, physical drive; -1 in invalid value
        int drive_index = 0;

        /// fixed/external/network/etc...
        int placement_type = 0;

        /// HDD/SSD
        int disk_type = 0;

        /// A-Z in upper case
        char drive_letter = '\0';

        /// Volume name if exist 
        std::string volume_name;

        /// Volume ID , string of numbers
        std::string volume_id;

        /// Filesystem name output, e.g. "NTFS"
        std::string filesystem_name;
    };

    /// @brief Basic portable partition-associated info
    struct PortablePartititon
    {
        PortablePartititon() = default;
        ~PortablePartititon() = default;
        PortablePartititon(const PortablePartititon&) = default;

        /// drive letter or mounting point
        std::wstring root;

        /// Up to 0, physical drive in system up to 0
        /// -1 is invalid value
        int drive_number = -1;

        /// see enum
        PlacementType placement_type = UnknownPlacement;

        /// see enum
        DiskType disk_type = UnknownType;

        /// Volume name if exist
        std::string volume_name;

        /// Volume ID , string of numbers
        std::string volume_id;

        /// Filesystem name output, e.g. "NTFS"
        std::string filesystem_name;
    };

    /// @brief partition information collected here. C-tor does not throw, but may work long time
    /// Do not use in in the GUI thread
    PartititonInformation();

    /// @brief Default, just satisfy compiler
    ~PartititonInformation() = default;

    /// @brief Construct on the first call, return "Meyers singleton" afterwards
    static PartititonInformation& instance();

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

    /// @brief List of physical drive system indexes
    const std::vector<int> get_physical_drives() const;

    /// @brief Size of partition root name (e.g. 'C')
    size_t root_string_size() const { return 1; }

    /// @brief System drive name in Windows format "X:\\"
    static std::string get_system_drive();

    /// @brief Get system-dependent volume information using GetVolumeInformationA()
    /// @param volume_index: Drive name in Windows format "X:\\"
    /// @param volume_name_out: Volume name output if exist 
    /// @param volume_id_out: Volume ID output, string of numbers
    /// @param filesystem_name_out: Filesystem name output, e.g. "NTFS"
    static std::string get_volume_information(const std::string& volume_index, 
        std::string& volume_name_out, 
        std::string& volume_id_out, 
        std::string& filesystem_name_out);

private:

    /// Physical drive type (HDD/SSD/Unknown)
    DiskType get_drive_type(int physical_drive_number) const;

    /// Physical drive placement (Fixed/Removable/CDROM/...)
    PlacementType get_drive_placement(const std::wstring& windows_drive_root) const;

    /// Convert drive letter to physical drive index (e.g C -> 1)
    int get_physical_drive_number(char windows_drive_letter) const;

    /// Every partition information by partition letter
    std::map<char, NativePartititon> partititon_info_;

    /// Every partition information by physical drive index
    std::map<int, std::vector<NativePartititon>> physical_disk_info_;

    /// Map WinAPI defines to application enum, e.g. DRIVE_REMOVABLE to RemovebleDisk
    static std::map<int, PlacementType> system_codes_2_placement_type_;

    // protect from updating disk information during reading
    mutable std::shared_mutex m_;
};

} // namespace helpers

#endif // defined(_WIN32) || defined(_WIN64)