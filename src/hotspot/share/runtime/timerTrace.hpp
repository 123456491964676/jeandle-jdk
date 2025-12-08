/*
 * Copyright (c) 2016, 2023, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#ifndef SHARE_RUNTIME_TIMERTRACE_HPP
#define SHARE_RUNTIME_TIMERTRACE_HPP

#include "logging/log.hpp"
#include "runtime/timer.hpp"
#include "utilities/globalDefinitions.hpp"

// TraceTime is used for tracing the execution time of a block
// Usage:
//  {
//    TraceTime t("some timer", TIMERTRACE_LOG(Info, startuptime, tagX...));
//    some_code();
//  }
//

typedef void (*TraceTimerLogPrintFunc)(const char*, ...);

// We need to explicit take address of LogImpl<>write<> and static cast
// due to MSVC is not compliant with templates two-phase lookup
// ？？？
#define TRACETIME_LOG(TT_LEVEL, ...) \
    log_is_enabled(TT_LEVEL, __VA_ARGS__) ? static_cast<TraceTimerLogPrintFunc>(&LogImpl<LOG_TAGS(__VA_ARGS__)>::write<LogLevel::TT_LEVEL>) : (TraceTimerLogPrintFunc)nullptr

// 继承自 StackObj，HotSpot 约定：这类对象应该只在栈上创建（不在堆上 new）
// 意义：生命周期由作用域决定，方便 RAII（构造开始计时，析构自动结束）
class TraceTime: public StackObj {
 private:
  bool          _active;    // do timing // 是否启用计时器
  bool          _verbose;   // report every timing // 是否在析构时打印输出
  elapsedTimer  _t;         // timer // elapsedTimer 类型的计时器对象，用来记录本次 TraceTime 的开始/结束时间
  elapsedTimer* _accum;     // accumulator // 用于累计多个 TraceTime 的时间：如果非空，在析构时会把 _t 记下的时间加到 _accum 中
  const char*   _title;     // name of timer // 标题字符串（计时的名称），用于打印输出
  // TraceTimerLogPrintFunc 类型的函数指针，指向某个日志输出函数（例如 LogImpl<...>::write<...>）。
  // 如果为 nullptr，则默认打印到 tty（标准 VM 控制台输出）
  TraceTimerLogPrintFunc _print; 

 public:
  // Constructors
  TraceTime(const char* title,
            bool doit = true);

  TraceTime(const char* title,
            elapsedTimer* accumulator,
            bool doit = true,
            bool verbose = false);

  TraceTime(const char* title,
            TraceTimerLogPrintFunc ttlpf);

  ~TraceTime();
};


#endif // SHARE_RUNTIME_TIMERTRACE_HPP
