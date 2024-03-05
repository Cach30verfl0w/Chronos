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
#define CHRONOS_OPEN ::open64
#define CHRONOS_MMAP ::mmap64
#else
#define CHRONOS_OPEN ::open
#define CHRONOS_MMAP ::mmap
#endif

namespace chronos::platform {
    FileMapping::FileMapping(u8* file_ptr, usize size) noexcept ://NOLINT
            _pointer {file_ptr},
            _size {size} {
    }

    FileMapping::FileMapping(chronos::platform::FileMapping&& other) noexcept ://NOLINT
            _pointer {other._pointer},
            _size {other._size} {
        other._pointer = nullptr;
    }

    FileMapping::~FileMapping() noexcept {
        if(_pointer != nullptr) {
            ::munmap(_pointer, _size);
            _pointer = nullptr;
        }
    }

    auto FileMapping::operator=(chronos::platform::FileMapping&& other) noexcept -> FileMapping& {
        _pointer = other._pointer;
        _size = other._size;
        other._pointer = nullptr;
        return *this;
    }

    auto FileMapping::operator*() const noexcept -> const u8* {
        return _pointer;
    }

    File::File(std::filesystem::path file_path, FileFlags flags) ://NOLINT
            _path {std::move(file_path)},
            _flags {flags} {
        const auto exists = std::filesystem::exists(file_path);

        // Create parent directory if necessary
        if(!exists && _path.has_parent_path()) {
            if(const auto parent_path = _path.parent_path(); std::filesystem::exists(parent_path)) {
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
        _file_handle = CHRONOS_OPEN(_path.c_str(), file_flags, permissions);
        if(_file_handle == -1) {
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
            ::close(_file_handle);
            _file_handle = invalid_file_handle;
        }
    }

    auto File::map_into_memory() const noexcept -> kstd::Result<FileMapping> {
        const auto file_size = get_file_size();
        if(file_size.is_error()) {
            return kstd::Error {file_size.get_error()};
        }

        // Generate flags for memory mapping
        int flags = 0;
        if(are_flags_set<FileFlags, FileFlags::READ>(_flags)) {
            flags |= PROT_READ;
        }

        if(are_flags_set<FileFlags, FileFlags::WRITE>(_flags)) {
            flags |= PROT_WRITE;
        }

        if(are_flags_set<FileFlags, FileFlags::EXECUTE>(_flags)) {
            flags |= PROT_EXEC;
        }

        // Pointer to mapped memory section
        auto* memory_ptr = CHRONOS_MMAP(nullptr, *file_size, flags, MAP_SHARED, _file_handle, 0);
        if(memory_ptr == MAP_FAILED) {
            return kstd::Error {fmt::format("Unable to map file into memory: {}", get_last_error())};
        }

        // Return file mapping
        return {{static_cast<u8*>(memory_ptr), *file_size}};
    }

    auto File::get_file_size() const noexcept -> kstd::Result<usize> {
        const auto temp_file_handle = ::fdopen(_file_handle, "r");
        if(temp_file_handle == nullptr) {
            return kstd::Error {fmt::format("Unable to acquire size of file: {}", get_last_error())};
        }

        fseek(temp_file_handle, EOF, SEEK_END);
        const auto position = ftell(temp_file_handle);
        //fclose(temp_file_handle);
        return position;
    }

    auto File::operator=(File&& other) noexcept -> File& {
        _file_handle = other._file_handle;
        _path = std::move(other._path);
        other._file_handle = invalid_file_handle;
        return *this;
    }
}// namespace chronos::platform
#endif