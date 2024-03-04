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
    File::File(std::filesystem::path file_path, chronos::platform::FileFlags flags) ://NOLINT
            _path {std::move(file_path)} {
        const auto path = kstd::utils::to_wcs(file_path.string());
        const auto exists = std::filesystem::exists(file_path);

        // Create parent directory if necessary
        if(!exists && file_path.has_parent_path()) {
            if(const auto parent_path = file_path.parent_path(); std::filesystem::exists(parent_path)) {
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

    File::File(File&& other) noexcept {
        _file_handle = other._file_handle;
        _path = std::move(other._path);
        other._file_handle = invalid_file_handle;
    }

    File::~File() noexcept {
        if(_file_handle != invalid_file_handle) {
            ::CloseHandle(_file_handle);
            _file_handle = invalid_file_handle;
        }
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
        other._file_handle = invalid_file_handle;
        return *this;
    }
}// namespace chronos::platform
#endif