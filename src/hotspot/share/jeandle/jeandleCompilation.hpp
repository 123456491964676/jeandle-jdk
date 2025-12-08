/*
 * Copyright (c) 2025, the Jeandle-JDK Authors. All Rights Reserved.
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
 */

#ifndef SHARE_JEANDLE_COMPILATION_HPP
#define SHARE_JEANDLE_COMPILATION_HPP

#include "jeandle/__llvmHeadersBegin__.hpp"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Target/TargetMachine.h"

#include <memory>

#include "jeandle/jeandleCompiledCode.hpp"

#include "jeandle/__hotspotHeadersBegin__.hpp"
#include "ci/ciEnv.hpp"
#include "ci/ciMethod.hpp"
#include "memory/arena.hpp"

// 新增
#include "runtime/timer.hpp"        // elapsedTimer // 新增
#include "runtime/timerTrace.hpp"   // TraceTime // 新增
#include "compiler/compiler_globals.hpp"  // 声明 CITime / CITimeEach / CITimeVerbose 等

enum JeandleTimerName {
  _t_jeandle_compile = 0,           // 总编译时间
  _t_jeandle_abstract_interpreter,  // JeandleAbstractInterpreter 阶段
  _t_jeandle_llvm_optimizer,        // LLVM 优化阶段
  _t_jeandle_llvm_codegen,          // LLVM 代码生成阶段
  _t_jeandle_finalize,              // JeandleCompiledCode::finalize 阶段
  _t_jeandle_max_phase
};
// 静态计时器数组，对标 C1 的 Compilation::timers[]
static elapsedTimer jeandle_timers[_t_jeandle_max_phase];
// 用于统计 Jeandle 一共编译了多少个方法（可选）
static int jeandle_compilation_count = 0;

// 新增
// class JeandleTraceTime : public TraceTime {
//  public:
//   // 这里使用TraceTime的这种构造函数，暂切将后两个参数固定
//   JeandleTraceTime(const char* name, elapsedTimer* timer)
//   : TraceTime(name , timer /*elapsedTimer类型的时间累加器 ？？？？ */, true /*启用计时器*/, false /*在析构时不打印不输出*/){}
// };
class JeandleTraceTime : public TraceTime {
 private:
  JeandleTimerName _timer;
  
 public:
  

  //  // 这里使用TraceTime的这种构造函数，暂切将后两个参数固定
  //  JeandleTraceTime(const char* name, elapsedTimer* timer)
  //  : TraceTime(name , timer /*elapsedTimer类型的时间累加器 ？？？？ */, true /*启用计时器*/, true /*在析构时是否打印输出*/){}
  //  JeandleTraceTime(const char* name, elapsedTimer* timer)
  //  : TraceTime(name, timer, JeandleTime, JeandleVerboseTime){}
  JeandleTraceTime(const char* name, JeandleTimerName timerName)
  : TraceTime(name,
              &jeandle_timers[timerName], 
              CITime || CITimeEach, 
              CITimeEach && CITimeVerbose),
    _timer(timerName){
         // 如将来需要 CompileLog，可在这里添加 log->begin_head()/stamp()/end_head()
  }

  ~JeandleTraceTime() {
    // TraceTime 析构时会：
    // 1. 停止计时
    // 2. 把本次时间 add 到 jeandle_timers[_timer]
    // 3. 如果 verbose=true 则打印一行 "[title, xxx secs]"
    // 我们这里的 title 传了 nullptr，会走默认格式（不依赖 title）
    // 如将来需要 CompileLog，可在这里添加 log->done(...)
  }
  
};

// Jeandle 汇总输出函数，对标 C1 的 Compilation::print_timers()
// 可以在 JeandleCompiler::print_timers() 中调用

class JeandleCompilation : public StackObj {
 public:
  // Compile a Java method.
  JeandleCompilation(llvm::TargetMachine* target_machine,
                     llvm::DataLayout* data_layout,
                     ciEnv* env,
                     ciMethod* method,
                     int entry_bci,
                     bool install_code,
                     llvm::MemoryBuffer* template_buffer);

  // Compile a runtime stub that call a JeandleRuntimeRoutine.
  JeandleCompilation(llvm::TargetMachine* target_machine,
                     llvm::DataLayout* data_layout,
                     ciEnv* env,
                     std::unique_ptr<llvm::LLVMContext> context,
                     const char* name,
                     address c_func,
                     llvm::FunctionType* func_type);

  ~JeandleCompilation() = default;

  // 在任何地方，只要包含了这个文件的头文件，都可以调用这个函数，
  // 获取“当前线程正在进行的JeandleCompilation对象”
  // 可以类比为一个“全局访问接口”，但它是通过 ciEnv 间接实现的，而不是一个真正的全局变量
  // 这是一个静态工具函数，用来从当前线程的 HotSpot 编译环境 ciEnv 中取出本次编译对应的 JeandleCompilation 对象指针，从而在任何地方都可以方便地访问“当前编译会话”的上下文数据
  static JeandleCompilation* current() { return (JeandleCompilation*) ciEnv::current()->compiler_data(); }
  
  // Error related:
  void report_error(const char* msg) {
    if (msg != nullptr) {
      _error_msg = msg;
    }
  }
  bool error_occurred() const { return _error_msg != nullptr; }
  static void report_jeandle_error(const char* msg) { JeandleCompilation::current()->report_error(msg); }
  static bool jeandle_error_occurred() { return JeandleCompilation::current()->error_occurred(); }

  JeandleCompiledCode* compiled_code() { return &_code; }

  Arena* arena() { return _arena; }

//  新增
 public:
  // 为 finalize 用的访问接口（给 JeandleCompiledCode::finalize 调用）
  elapsedTimer* finalize_timer() { return &_timer_finalize; }
  static void print_jeandle_timers();


 private:
  Arena* _arena; // Hold compilation life-time objects (JeandleCompilationResourceObj).
  llvm::TargetMachine* _target_machine;
  llvm::DataLayout* _data_layout;
  ciEnv* _env;
  ciMethod* _method;
  int _entry_bci;
  std::unique_ptr<llvm::LLVMContext> _context;
  std::unique_ptr<llvm::Module> _llvm_module;
  std::string _comp_start_time;

  JeandleCompiledCode _code; // Compiled code.

  const char* _error_msg;

  // 新增
  elapsedTimer _timer_total;
  elapsedTimer _timer_abstract_interpreter;
  elapsedTimer _timer_llvm_optimizer;
  elapsedTimer _timer_llvm_codegen;
  elapsedTimer _timer_finalize;
  

  void initialize();
  void setup_llvm_module(llvm::MemoryBuffer* template_buffer);
  void compile_java_method();
  void compile_module();
  void install_code();

  void dump_obj();
  void dump_ir(bool optimized);
};

#endif // SHARE_JEANDLE_COMPILATION_HPP
