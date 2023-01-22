#include <cassert>

#if defined(_WIN32) || defined(_WIN64)
#include <winapi-helpers/win_partition_information.h>
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include <algorithm>  // std::transform
#include <posix-helpers/posix_partition_information.h>
#endif

#include <winapi-helpers/partition_information.h>

using namespace helpers;

std::map<PartititonInformation::PlacementType, std::string> PartititonInformation::placement_type_description_{
    { PartititonInformation::UnknownPlacement, "Unknown drive type" },
    { PartititonInformation::NoRootDir, "No such root directory" },
    { PartititonInformation::RemovebleDisk, "Removable or external drive" },
    { PartititonInformation::FixedDisk, "Fixed drive" },
    { PartititonInformation::NetworkDisk, "Network disk or network-protocol accessible" },
    { PartititonInformation::CDROM, "CDROM" },
    { PartititonInformation::RamDisk, "RAM drive" }
};

std::map<PartititonInformation::DiskType, std::string> PartititonInformation::disk_type_description_{
    { HDD, "HDD" },
    { SSD, "SSD" },
    { UnknownType, "Unknown" },
};

PartititonInformation& PartititonInformation::instance()
{
    static PartititonInformation single_partition_information;
    return single_partition_information;
}

PartititonInformation::PartititonInformation() 
    : native_partition_info_(std::make_unique<NativePartititonInformation>())
{
}

std::string PartititonInformation::get_system_drive()
{
    return native_partition_info_->get_system_drive();
}

std::string PartititonInformation::placement_type_string(int placement_type)
{
    auto it = placement_type_description_.find(
        static_cast<PlacementType>(placement_type));
    if (it == placement_type_description_.end()) {
        assert(false);
    }
    return (*it).second;
}

std::string PartititonInformation::disk_type_string(int disk_type)
{
    auto it = disk_type_description_.find(
        static_cast<DiskType>(disk_type));
    if (it == disk_type_description_.end()) {
        assert(false);
    }
    return (*it).second;

}

std::vector<PartititonInformation::PortablePartititon> PartititonInformation::enumerate_partititons() const
{
    std::vector<NativePartititonInformation::NativePartititon> native_partititons = 
        native_partition_info_->enumerate_partititons();
    std::vector<PortablePartititon> portable_partitions;
    std::transform(native_partititons.begin(), native_partititons.end(), std::back_inserter(portable_partitions), 
        [](const auto& part_info) {

        PortablePartititon p;
#if defined(_WIN32) || defined(_WIN64)
        p.root = part_info.drive_letter;
#else
        p.root = part_info.mounting_point;
#endif
        p.drive_number = part_info.drive_index;
        p.placement_type = static_cast<PartititonInformation::PlacementType>(part_info.placement_type);
        p.disk_type = static_cast<PartititonInformation::DiskType>(part_info.disk_type);
        return p;
    });
    return std::move(portable_partitions);

}

std::vector<PartititonInformation::PortablePartititon> PartititonInformation::enumerate_drive_partititons(int drive_index) const
{
    std::vector<NativePartititonInformation::NativePartititon> native_partititons = 
        native_partition_info_->enumerate_drive_partititons(drive_index);

    std::vector<PortablePartititon> portable_partitions;
    std::transform(native_partititons.begin(), native_partititons.end(), std::back_inserter(portable_partitions),
        [](const auto& part_info) {

        PortablePartititon p;
#if defined(_WIN32) || defined(_WIN64)
        p.root = part_info.drive_letter;
#else
        p.root = part_info.mounting_point;
#endif
        p.drive_number = part_info.drive_index;
        p.placement_type = static_cast<PartititonInformation::PlacementType>(part_info.placement_type);
        p.disk_type = static_cast<PartititonInformation::DiskType>(part_info.disk_type);
        p.volume_name = part_info.volume_name;
        p.volume_id = part_info.volume_id;
        p.filesystem_name = part_info.filesystem_name;
        return p;
    });
    return std::move(portable_partitions);

}

std::unique_ptr<PartititonInformation::PortablePartititon>
PartititonInformation::get_file_partition_info(const std::string& filepath) const
{
    std::unique_ptr<NativePartititonInformation::NativePartititon> native_partition 
        = native_partition_info_->get_file_partition_info(filepath);

    std::unique_ptr<PortablePartititon> partition = std::make_unique<PortablePartititon>();
    partition->disk_type = static_cast<DiskType>(native_partition->disk_type);
    partition->drive_number = native_partition->drive_index;
    partition->placement_type = static_cast<PlacementType>(native_partition->placement_type);

    return std::move(partition);
}

void PartititonInformation::collect_partititon_information()
{
    native_partition_info_->collect_partititon_information();
}

const std::vector<int> PartititonInformation::get_physical_drives() const
{
    return native_partition_info_->get_physical_drives();
}

size_t PartititonInformation::root_string_size() const
{
    return native_partition_info_->root_string_size();
}

