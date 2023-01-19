#if defined(_WIN32) || defined(_WIN64)	
#include <winapi-helpers/partition_information.h>
#include <winapi-helpers/win_errors.h>
#include <winapi-helpers/utilities.h>


#include <Windows.h>

#include <cassert>
#include <algorithm>
#include <iterator>
#include <mutex>
#include <cctype>
#include <regex>
#include <set>

using namespace helpers;

std::map<int, PartititonInformation::PlacementType> PartititonInformation::system_codes_2_placement_type_{
    { DRIVE_UNKNOWN, PartititonInformation::UnknownPlacement },
    { DRIVE_NO_ROOT_DIR, PartititonInformation::NoRootDir },
    { DRIVE_REMOVABLE, PartititonInformation::RemovebleDisk },
    { DRIVE_FIXED, PartititonInformation::FixedDisk },
    { DRIVE_REMOTE, PartititonInformation::NetworkDisk },
    { DRIVE_CDROM, PartititonInformation::CDROM },
    { DRIVE_RAMDISK, PartititonInformation::RamDisk }
};


PartititonInformation& PartititonInformation::instance()
{
    static PartititonInformation single_partition_information;
    return single_partition_information;
}

PartititonInformation::PartititonInformation()
{
    collect_partititon_information();
}

std::vector<PartititonInformation::NativePartititon> 
PartititonInformation::enumerate_partititons() const
{
    // read lock
    std::shared_lock<std::shared_mutex> lk(m_);
    std::vector<NativePartititon> partitions;
    std::transform(partititon_info_.begin(), partititon_info_.end(), std::back_inserter(partitions), [](const auto& part_info) {
        return part_info.second;
    });
    return std::move(partitions);
}

std::unique_ptr<PartititonInformation::NativePartititon> 
PartititonInformation::get_file_partition_info(const std::string& filepath) const
{
    if (filepath.empty() || !std::isalpha(filepath[0])) {
        return std::make_unique<NativePartititon>();
    }
    char drive_letter = std::toupper(filepath[0]);

    // read lock
    std::shared_lock<std::shared_mutex> lk(m_);

    auto it = partititon_info_.find(drive_letter);

    // first time not found - invalid path
    if (it == partititon_info_.end()) {
        return std::make_unique<NativePartititon>();
    }
    return std::make_unique<NativePartititon>((*it).second);
}

#ifdef PARTITIONS_MOCK
/// This is "mock" method which fill partition information with fake data
/// Various partition types present so that test behavior of any API user
void NativePartititonInformation::collect_partititon_information()
{
    // Physical drive 0
    NativePartititon c{ 0, NativePartititonInformation::FixedDisk, DiskType::HDD, 'C' };
    NativePartititon d{ 0, NativePartititonInformation::FixedDisk, DiskType::HDD, 'D' };

    // Physical drive 1
    NativePartititon e{ 1, NativePartititonInformation::FixedDisk, DiskType::HDD, 'E' };
    NativePartititon f{ 1, NativePartititonInformation::FixedDisk, DiskType::HDD, 'F' };
    NativePartititon g{ 1, NativePartititonInformation::FixedDisk, DiskType::HDD, 'G' };

    // Physical drive 2
    NativePartititon h{ 2, NativePartititonInformation::FixedDisk, DiskType::SSD, 'H' };
    NativePartititon i{ 2, NativePartititonInformation::FixedDisk, DiskType::SSD, 'I' };

    // External drive 1 (type unknown)
    NativePartititon j{ 3, NativePartititonInformation::RemovebleDisk, DiskType::UnknownType, 'J' };

    // External drive 2 (type unknown)
    NativePartititon k{ 4, NativePartititonInformation::RemovebleDisk, DiskType::UnknownType, 'K' };

    // Virtual drive (container)
    NativePartititon l{ -1, NativePartititonInformation::FixedDisk, DiskType::UnknownType, 'L' };

    // sort out manually
    partititon_info_['C'] = c;
    partititon_info_['D'] = d;
    partititon_info_['E'] = e;
    partititon_info_['F'] = f;
    partititon_info_['G'] = g;
    partititon_info_['H'] = h;
    partititon_info_['I'] = i;
    partititon_info_['J'] = j;
    partititon_info_['K'] = k;
    partititon_info_['L'] = l;

    // by physical disk number
    physical_disk_info_[0].assign({ c, d });
    physical_disk_info_[1].assign({ e, f, g });
    physical_disk_info_[2].assign({ h, i });
    physical_disk_info_[3].push_back(j);
    physical_disk_info_[4].push_back(k);

    // physical disk without indexes
    unrecognized_partititons_.push_back(l);
}
#else
void PartititonInformation::collect_partititon_information()
{
    char disk_windows_pattern[] = "X:\\";
    char current_drive_letter = 'A';
    DWORD drive_mask = ::GetLogicalDrives();
    if (0 == drive_mask) {
        // just leave data empty
        return;
    }
    
    // write lock
    std::unique_lock<std::shared_mutex> unique_lk(m_);

    partititon_info_.clear();
    while (drive_mask) {
        if (drive_mask & 0x1) {

            NativePartititon part_info;

            // compose correct windows drive root
            disk_windows_pattern[0] = current_drive_letter;
            auto wdisk_windows_pattern = helpers::string_to_wstring(disk_windows_pattern);

            part_info.drive_letter = current_drive_letter;
            part_info.placement_type = get_drive_placement(wdisk_windows_pattern);
            part_info.drive_index = get_physical_drive_number(current_drive_letter);
            part_info.disk_type = get_drive_type(part_info.drive_index);

            std::string volume_name;
            std::string volume_id;
            std::string filesystem_name;
            std::string last_error = get_volume_information(disk_windows_pattern, volume_name, volume_id, filesystem_name);
            if (last_error.empty()) {
                part_info.volume_name = volume_name;
                part_info.volume_id = volume_id;
                part_info.filesystem_name = filesystem_name;
            }

            partititon_info_[current_drive_letter] = part_info;
            
            if ((part_info.placement_type == FixedDisk) 
                || (part_info.placement_type == RemovebleDisk)
                || (part_info.placement_type == UnknownPlacement) ) {
                physical_disk_info_[part_info.drive_index].push_back(part_info);
            }
        }
        ++current_drive_letter;
        drive_mask >>= 1;
    }

}
#endif

int PartititonInformation::get_physical_drive_number(char windows_drive_letter) const
{
    static const size_t drive_letter_position = 4;
    wchar_t system_partition_name_pattern[] = L"\\\\.\\X:";
    
    system_partition_name_pattern[drive_letter_position] = windows_drive_letter;
    HANDLE partitionHandle = ::CreateFileW(system_partition_name_pattern,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
        OPEN_EXISTING, 0, nullptr);

    if (partitionHandle == INVALID_HANDLE_VALUE) {
        return -1;
    }

    VOLUME_DISK_EXTENTS diskExtents{};

    DWORD read_structure_size{};
    BOOL result = DeviceIoControl(
        partitionHandle,
        IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,
        nullptr,
        0,
        &diskExtents,
        sizeof(diskExtents),
        &read_structure_size,
        nullptr);

    if (result) {
        return diskExtents.Extents[0].DiskNumber;
    }
    else {
        return -1;
    }

}

helpers::PartititonInformation::PlacementType 
PartititonInformation::get_drive_placement(const std::wstring& windows_drive_root) const
{
    if (windows_drive_root.size() != 3) {
        assert(false);
        return PlacementType::UnknownPlacement;
    }
    auto it  = system_codes_2_placement_type_.find(GetDriveTypeW(windows_drive_root.c_str()));
    if (it == system_codes_2_placement_type_.end()) {
        assert(false);
        return PlacementType::UnknownPlacement;
    }
    return (*it).second;
}

PartititonInformation::DiskType 
PartititonInformation::get_drive_type(int physical_drive_number) const
{
    static const size_t drive_number_position = 17;
    wchar_t physical_drive_pattern[] = L"\\\\?\\PhysicalDriveX";

    if (physical_drive_number == -1 || physical_drive_number > 9) {
        return DiskType::UnknownType;
    }
    
    // '\0' + 48 is char("0")
    physical_drive_pattern[drive_number_position] = physical_drive_number + 48; 

    DWORD bytesReturned{};
    //As an example, let's test 1st physical drive
    HANDLE physical_drive_device = ::CreateFileW(physical_drive_pattern,
        //We need write access to send ATA command to read RPMs
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
        OPEN_EXISTING, 0, nullptr);

    if (physical_drive_device == INVALID_HANDLE_VALUE) {
        return DiskType::UnknownType;
    }

    STORAGE_PROPERTY_QUERY spqTrim{};
    spqTrim.PropertyId = StorageDeviceTrimProperty;
    spqTrim.QueryType = PropertyStandardQuery;

    bytesReturned = 0;
    DEVICE_TRIM_DESCRIPTOR dtd = {};
    if (::DeviceIoControl(physical_drive_device, IOCTL_STORAGE_QUERY_PROPERTY,
        &spqTrim, sizeof(spqTrim), &dtd, sizeof(dtd), &bytesReturned, nullptr) &&
        bytesReturned == sizeof(dtd)) {
        //Got it
        return dtd.TrimEnabled ? DiskType::SSD : DiskType::HDD;
    }
    else {
        return DiskType::UnknownType;
    }
}

std::vector<PartititonInformation::NativePartititon>
PartititonInformation::enumerate_drive_partititons(int drive_index) const
{
    auto it = physical_disk_info_.find(drive_index);
    if (physical_disk_info_.end() != it) {
        return (*it).second;
    }
    return std::vector<PartititonInformation::NativePartititon>{};
}

#ifdef PARTITIONS_MOCK
const std::vector<int> NativePartititonInformation::get_physical_drives() const
{
    return std::move(std::vector<int>{ 0, 1, 2, 3 });
}
#else
const std::vector<int> PartititonInformation::get_physical_drives() const
{
    std::set<int> drive_indexes;
    for (const auto& part_info : partititon_info_) {
        drive_indexes.insert(part_info.second.drive_index);
    }
    return std::move(std::vector<int>{drive_indexes.begin(), drive_indexes.end()});
}

std::string PartititonInformation::get_system_drive()
{
    static const char* drive_regex_str = R"(^[a-zA-Z]:\\$)";
    std::regex drive_regex(drive_regex_str);
    std::string system_drive;

    const char* system_drive_env = std::getenv("SystemDrive");
    if (nullptr == system_drive_env) {
        system_drive = "C:\\";
    }
    else {
        system_drive = system_drive_env;
        system_drive += '\\';
    }

    if (!std::regex_match(system_drive, drive_regex)) {
        system_drive = "C:\\";
    }
    return std::move(system_drive);
}

std::string helpers::PartititonInformation::get_volume_information(const std::string& volume_index, 
    std::string& volume_name_out, std::string& volume_id_out, std::string& filesystem_name_out)
{
    const size_t path_size = MAX_PATH + 1;
    char volume_name_ret[path_size] = { 0 };
    char filesystem_name_ret[path_size] = { 0 };
    DWORD serial_number_ret = 0;
    DWORD max_component_len = 0;
    DWORD file_system_flags = 0;

    std::string last_error = WinErrorChecker::last_error_nothrow_boolean(GetVolumeInformationA(
        volume_index.c_str(),
        volume_name_ret,
        path_size,
        &serial_number_ret,
        &max_component_len,
        &file_system_flags,
        filesystem_name_ret,
        path_size));

    if (volume_name_ret) {
        volume_name_out = volume_name_ret;
    }

    if (serial_number_ret) {
        volume_id_out = std::to_string(static_cast<unsigned long>(serial_number_ret));
    }

    if (filesystem_name_ret) {
        filesystem_name_out = filesystem_name_ret;
    }
    return last_error;
}

#endif

#endif // defined(_WIN32) || defined(_WIN64)
