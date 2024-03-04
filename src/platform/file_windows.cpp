//  Copyright 2024 Cach30verfl0w
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#ifdef PLATFORM_WINDOWS
#include <kstd/utils.hpp>
#include <utility>

#include "chronos/platform/file.hpp"

namespace chronos::platform {
    FileMapping::FileMapping(u8* file_ptr, HANDLE memory_map_handle, usize size) noexcept ://NOLINT
            _pointer {file_ptr},
            _size {size},
            _memory_map_handle {memory_map_handle} {
    }

    FileMapping::FileMapping(chronos::platform::FileMapping&& other) noexcept ://NOLINT
            _pointer {other._pointer},
            _size {other._size},
            _memory_map_handle {other._memory_map_handle} {
        other._pointer = nullptr;
        other._memory_map_handle = INVALID_HANDLE_VALUE;
    }

    FileMapping::~FileMapping() noexcept {
        if(_memory_map_handle != INVALID_HANDLE_VALUE && _pointer != nullptr) {
            ::UnmapViewOfFile(_pointer);
            ::CloseHandle(_memory_map_handle);
            _pointer = nullptr;
        }
    }

    auto FileMapping::operator=(chronos::platform::FileMapping&& other) noexcept -> FileMapping& {
        _memory_map_handle = other._memory_map_handle;
        _pointer = other._pointer;
        _size = other._size;
        other._memory_map_handle = INVALID_HANDLE_VALUE;
        other._pointer = nullptr;
        return *this;
    }

    auto FileMapping::operator*() const noexcept -> const u8* {
        return _pointer;
    }

    File::File(std::filesystem::path file_path, chronos::platform::FileFlags flags) ://NOLINT
            _path {std::move(file_path)},
            _flags {flags} {
        const auto path = kstd::utils::to_wcs(_path.string());
        const auto exists = std::filesystem::exists(_path);

        // Create parent directory if necessary
        if(!exists && _path.has_parent_path()) {
            if(const auto parent_path = _path.parent_path(); std::filesystem::exists(parent_path)) {
                std::filesystem::create_directories(parent_path);
            }
        }

        // Create access flags
        DWORD access = 0;
        if(are_flags_set<FileFlags, FileFlags::WRITE>(flags)) {
            access |= GENERIC_WRITE;
        }

        if(are_flags_set<FileFlags, FileFlags::READ>(flags)) {
            access |= GENERIC_READ;
        }

        if(are_flags_set<FileFlags, FileFlags::EXECUTE>(flags)) {
            access |= GENERIC_EXECUTE;
        }

        // Create or open file
        _file_handle = ::CreateFileW(path.data(), access, 0, nullptr, exists ? OPEN_EXISTING : CREATE_NEW,
                                     FILE_ATTRIBUTE_NORMAL, nullptr);
        if(_file_handle == invalid_file_handle) {
            throw std::runtime_error {fmt::format("Unable to open file: {}", get_last_error())};
        }
    }

    File::File(File&& other) noexcept ://NOLINT
            _file_handle {other._file_handle},
            _path {std::move(other._path)},
            _flags {other._flags} {
        other._file_handle = invalid_file_handle;
    }

    File::~File() noexcept {
        if(_file_handle != invalid_file_handle) {
            ::CloseHandle(_file_handle);
            _file_handle = invalid_file_handle;
        }
    }

    auto File::map_into_memory() const noexcept -> kstd::Result<FileMapping> {
        const auto file_size = get_file_size();
        if(file_size.is_error()) {
            return kstd::Error {file_size.get_error()};
        }

        int flags = 0;
        if(are_flags_set<FileFlags, FileFlags::READ, FileFlags::WRITE, FileFlags::EXECUTE>(_flags)) {
            flags = PAGE_EXECUTE_READWRITE;
        }
        else if(are_flags_set<FileFlags, FileFlags::READ, FileFlags::EXECUTE>(_flags)) {
            flags = PAGE_EXECUTE_READ;
        }
        else if(are_flags_set<FileFlags, FileFlags::READ, FileFlags::WRITE>(_flags)) {
            flags = PAGE_READWRITE;
        }
        else if(are_flags_set<FileFlags, FileFlags::EXECUTE>(_flags)) {
            flags = PAGE_EXECUTE;
        }
        else if(are_flags_set<FileFlags, FileFlags::READ>(_flags)) {
            flags = PAGE_READONLY;
        }
        else {
            using namespace std::string_literals;
            return kstd::Error {"Unable to map the file into the memory: Illegal flags"s};
        }

        // Create file mapping
        const auto file_mapping_handle = ::CreateFileMapping(_file_handle, nullptr, flags, 0, 0, nullptr);
        if(file_mapping_handle == nullptr) {
            return kstd::Error {fmt::format("Unable to map the file into the memory: {}", get_last_error())};
        }

        // Create desired access
        int desired_access = 0;
        if(are_flags_set<FileFlags, FileFlags::EXECUTE>(_flags)) {
            desired_access = FILE_MAP_EXECUTE;
        }

        if(are_flags_set<FileFlags, FileFlags::WRITE>(_flags)) {
            desired_access = FILE_MAP_WRITE;
        }


        if(are_flags_set<FileFlags, FileFlags::READ>(_flags)) {
            desired_access = FILE_MAP_READ;
        }

        // Map view of file and return
        const auto base_ptr = ::MapViewOfFile(file_mapping_handle, desired_access, 0, 0, 0);
        if(base_ptr == nullptr) {
            CloseHandle(file_mapping_handle);
            return kstd::Error {fmt::format("{}", get_last_error())};
        }

        return {{static_cast<u8*>(base_ptr), file_mapping_handle, file_size}};
    }

    auto File::get_file_size() const noexcept -> kstd::Result<usize> {
        DWORD file_size = 0;
        if(::GetFileSize(_file_handle, &file_size) == INVALID_FILE_SIZE) {
            return kstd::Error {fmt::format("Unable to get size of file: {}", get_last_error())};
        }
        return file_size;
    }

    auto File::operator=(File&& other) noexcept -> File& {
        _file_handle = other._file_handle;
        _path = std::move(other._path);
        _flags = other._flags;
        other._file_handle = invalid_file_handle;
        return *this;
    }
}// namespace chronos::platform
#endif