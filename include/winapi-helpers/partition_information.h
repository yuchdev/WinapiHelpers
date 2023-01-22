/* 2017-2020 WEBGEARS SERVICES LIMITED (c) All Rights Reserved. 
 * Proprietary and confidential.
 * The Software and any accompanying documentation are copyrighted and protected 
 * by copyright laws and international copyright treaties, as well as other 
 * intellectual property laws and treaties.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 */

#pragma once
#include <memory>
#include <map>
#include <string>
#include <vector>

namespace helpers {

class NativePartititonInformation;

/// @brief System-independent information about logical partitions and all data associated with them:
/// Physical drive index, type of disk, type of placement (fixed/external/network/etc...)
/// Drive letter under Windows, nount point under POSIX
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

    /// @brief Construct on the first call, return "Meyers singleton" afterwards
    static PartititonInformation& instance();

    /// @Just satisfy compiler
    ~PartititonInformation() = default;

    /// @brief System drive name or mounting point
    std::string get_system_drive();

    /// @brief Get the current 'slice' of partition information
    /// Method has read lock in the PImpl layer
    std::vector<PortablePartititon> enumerate_partititons() const;

    /// @brief Get the current 'slice' of partition information
    /// Method has read lock in the PImpl layer
    std::vector<PortablePartititon> enumerate_drive_partititons(int drive_index) const;

    /// @brief Get the some logical drive information by the path
    /// Method has read lock in the PImpl layer
    std::unique_ptr<PortablePartititon> get_file_partition_info(const std::string& filepath) const;

    /// @brief Convert disk placement to string (fixed/external/network/etc...)
    static std::string placement_type_string(int placement_type);

    /// @brief Convert disk placement to string (HDD/SSD/Unknown)
    static std::string disk_type_string(int disk_type);

    /// @brief 
    const std::vector<int> get_physical_drives() const;

    /// @brief Update information about logical partitions
    /// Called from constructor, other use case is update partition information
    /// Method may have write lock in the PImpl layer
    void collect_partititon_information();

    /// @brief 
    size_t root_string_size() const;

private:

    /// Polling all drive information sources using underlying system-dependent interface
    /// Never call directly, never copy
    PartititonInformation();
    PartititonInformation(const PartititonInformation&) = delete;
    PartititonInformation& operator=(const PartititonInformation&) = delete;

    /// Convert disk placement to string (fixed/external/network/etc...)
    static std::map<PlacementType, std::string> placement_type_description_;

    /// Convert disk placement to string(HDD / SSD / Unknown)
    static std::map<DiskType, std::string> disk_type_description_;

    /// PImpl to native system-specific implementation
    std::unique_ptr<NativePartititonInformation> native_partition_info_;
};

} // namespace helpers

