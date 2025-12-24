# 使用jeandle LLVM工具重现Jeandle 编译过程
## 简单介绍
Jeandle编译器支持dump 出 IR 并通过 Jeandle LLVM 工具进行中端优化和后端复现

## 前置条件
你需要先按照[Getting Started](https://github.com/jeandle/jeandle-jdk/blob/main/jeandle-docs/getting-started.md)文档中的内容获取到源码并构建出JDK

## JVM 标志
会使用到下面的标志
```JeandleDumpIR```
```UseJeandleCompiler```
```JeandleDumpDirectory```
如果你不清楚这些标志的含义，请点击[jeandle-flags](https://github.com/jeandle/jeandle-jdk/blob/main/jeandle-docs/jeandle-flags.md)

## 具体使用
An example of is as follows:Fibonacci
```java
public class Main {

    public static int fibonacci(int n) {
        if (n == 0) {
            return 0;
        } else if (n == 1) {
            return 1;
        } else {
            return fibonacci(n - 1) + fibonacci(n - 2);
        }
    }

    public static void main(String[] args) {
        int num = 10;
        for (int i = 0; i < num; i++) {
            System.out.print(fibonacci(i) + " ");
        }
    }
}
```
这里启用 -XX:+JeandleDumpIR 和 -XX:JeandleDumpDirectory 使得编译器dump出IR并存储在指定路径
使用下面命令运行jeandle
```
javac Main.java
java -XX:-TieredCompilation -Xcomp \
     -XX:CompileCommand=compileonly,Main::fibonacci \
     -XX:+UseJeandleCompiler \
     -XX:+JeandleDumpIR \
     -XX:JeandleDumpDirectory=/tmp/jeandle_ir Main
```
你会在你设置的存储目录中得到类似Main_fibonacci_1766477713319_optimized.ll的文件

## 中端优化复现
中端优化复现使用opt工具
复现命令如下：
```
opt -S -passes='rewrite-statepoints-for-gc,default<O3>' [Method].ll -o manual_optimized.ll
```

## 后端复现
后端复现使用llc工具
复现命令如下：
```
llc -O1 -filetype=obj -mtriple=x86_64-linux-gnu [Method]_optimized.ll -o manual_replay.o
```

## 结果验证
jeandle后端的一个核心约束是java调用点必须4字节对齐
检查命令：
```
llvm-objdump -d manual_replay.o
```
验证示例
观察反汇编中的地址：

```text
     13: 90                            nop
     14: 0f 1f 00                      nopl    (%rax)
     17: 0f 1f 44 00 08                nopl    0x8(%rax,%rax)
     1c: 89 44 24 18                   movl    %eax, 0x18(%rsp)
```
判定: NOP 片段结束后的下一条指令起始地址为 0x1c (28)。
结果: 28 % 4 == 0，对齐正确。
