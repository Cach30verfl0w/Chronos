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

#include <gtest/gtest.h>
#include <libdebug/process.hpp>
#include <thread>

TEST(libdebug_ProcessContext, test_multi_thread_attach) {
    const auto child_pid = ::fork();
    if (child_pid == 0) {
        ::personality(ADDR_NO_RANDOMIZE);
        ::execle(SAMPLE_MULTITHREAD_FILE, SAMPLE_MULTITHREAD_FILE);
    } else {
        sleep(1);
        const auto process_context = libdebug::ProcessContext {child_pid};
        ASSERT_EQ(process_context.get_threads().size(), 2);
        ::kill(child_pid, SIGKILL);
    }
}
