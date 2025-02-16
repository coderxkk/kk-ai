﻿/*
 * Copyright (c) 2016 The ZLToolKit project authors. All Rights Reserved.
 *
 * This file is part of ZLToolKit(https://github.com/ZLMediaKit/ZLToolKit).
 *
 * Use of this source code is governed by MIT license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#include <chrono>
#include "Util/logger.h"
#include "Util/onceToken.h"
#include "Util/TimeTicker.h"
#include "Thread/ThreadPool.h"

using namespace std;
using namespace toolkit;

int main() {
    //初始化日志系统  [AUTO-TRANSLATED:25c549de]
    // Initialize the logging system
    Logger::Instance().add(std::make_shared<ConsoleChannel>());
    Logger::Instance().setWriter(std::make_shared<AsyncLogWriter>());

    ThreadPool pool(thread::hardware_concurrency(), ThreadPool::PRIORITY_HIGHEST, true);

    //每个任务耗时3秒  [AUTO-TRANSLATED:c1b83e8e]
    // Each task takes 3 seconds
    auto task_second = 3;
    //每个线程平均执行4次任务，总耗时应该为12秒  [AUTO-TRANSLATED:ceab38cc]
    // Each thread executes 4 tasks on average, the total time should be 12 seconds
    auto task_count = thread::hardware_concurrency() * 4;

    semaphore sem;
    vector<int> vec;
    vec.resize(task_count);
    Ticker ticker;
    {
        //放在作用域中确保token引用次数减1  [AUTO-TRANSLATED:a81d2393]
        // Put it in a scope to ensure the token reference count is decremented
        auto token = std::make_shared<onceToken>(nullptr, [&]() {
            sem.post();
        });

        for (auto i = 0; i < task_count; ++i) {
            pool.async([token, i, task_second, &vec]() {
                setThreadName(("thread pool " + to_string(i)).data());
                std::this_thread::sleep_for(std::chrono::seconds(task_second)); //休眠三秒
                InfoL << "task " << i << " done!";
                vec[i] = i;
            });
        }
    }

    sem.wait();
    InfoL << "all task done, used milliseconds:" << ticker.elapsedTime();

    //打印执行结果  [AUTO-TRANSLATED:08995cc8]
    // Print the execution result
    for (auto i = 0; i < task_count; ++i) {
        InfoL << vec[i];
    }
    return 0;
}
