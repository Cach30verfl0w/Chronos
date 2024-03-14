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

#include <thread>
#include <iostream>

#ifdef PLATFORM_WINDOWS
#include <processthreadsapi.h>
#else
#include <unistd.h>
#endif

auto print_tid() noexcept -> void {
#ifdef PLATFORM_WINDOWS
    const auto thread_id = ::GetCurrentThreadId();
#else
    const auto thread_id = ::gettid();
#endif

    printf("%d %i (%i)\n", ::getppid(), ::getpid(), thread_id);
    while (true) {}
}

auto main() noexcept -> int {
    auto thread = std::thread {print_tid};
    thread.join();

    // print_tid();
    return 0;
}