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

#ifdef PLATFORM_LINUX
#include "chronos/platform/file.hpp"

#ifdef CPU_64_BIT
#define CHRONOS_STAT struct ::stat64
#define CHRONOS_FSTAT ::fstat64
#define CHRONOS_OPEN ::open64
#else
#define CHRONOS_STAT struct ::stat
#define CHRONOS_FSTAT ::fstat
#define CHRONOS_OPEN ::open
#endif

namespace chronos::platform {
    File::File(std::filesystem::path file_path, FileFlags flags) ://NOLINT
            _path {std::move(file_path)} {
        const auto exists = std::filesystem::exists(file_path);

        // Create parent directory if necessary
        if(!exists && file_path.has_parent_path()) {
            if(const auto parent_path = file_path.parent_path(); std::filesystem::exists(parent_path)) {
                std::filesystem::create_directories(parent_path);
            }
        }

        // Create flags and file permissions
        int file_flags = O_CREAT;
        int permissions = 0;
        if(are_flags_set<FileFlags, FileFlags::WRITE, FileFlags::READ>(flags)) {
            permissions |= S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IWOTH;
            file_flags |= O_RDWR;
        }
        else {
            if(are_flags_set<FileFlags, FileFlags::READ>(flags)) {
                permissions |= S_IWUSR | S_IWGRP | S_IWOTH;
                file_flags |= O_RDONLY;
            }

            if(are_flags_set<FileFlags, FileFlags::WRITE>(flags)) {
                permissions |= S_IRUSR | S_IRGRP | S_IROTH;
                file_flags |= O_WRONLY;
            }
        }

        if(are_flags_set<FileFlags, FileFlags::EXECUTE>(flags)) {
            permissions |= S_IXUSR | S_IXGRP | S_IXOTH;
        }

        // Create or open file
        _file_handle = CHRONOS_OPEN(file_path.c_str(), file_flags, permissions);
        if(_file_handle == -1) {
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
            ::close(_file_handle);
            _file_handle = invalid_file_handle;
        }
    }

    auto File::get_file_size() const noexcept -> kstd::Result<usize> {
        CHRONOS_STAT stat_buf {};
        if(CHRONOS_FSTAT(_file_handle, &stat_buf) == -1) {
            return kstd::Error {fmt::format("Unable to get size of file: {}", get_last_error())};
        }
        return stat_buf.st_size;
    }

    auto File::operator=(File&& other) noexcept -> File& {
        _file_handle = other._file_handle;
        _path = std::move(other._path);
        other._file_handle = invalid_file_handle;
        return *this;
    }
}// namespace chronos::platform
#endif