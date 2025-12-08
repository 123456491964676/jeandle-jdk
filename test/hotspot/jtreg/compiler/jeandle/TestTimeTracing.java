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

/**
 * @test
 * @summary Verify that Jeandle time tracing works with CITime on release builds
 * @library /test/lib
 * @run main/othervm -Xbootclasspath/a:. -XX:+UnlockDiagnosticVMOptions
 *      -XX:+UseJeandleCompiler
 *      -XX:+CITime
 *      -XX:-TieredCompilation -Xcomp
 *      -XX:CompileCommand=compileonly,TestTimeTracing::*
 *      -XX:CompileCommand=exclude,TestTimeTracing::main
 *      TestTimeTracing
 */

/**
 * @test
 * @summary Verify that Jeandle time tracing works with CITimeEach/CITimeVerbose on debug builds
 * @requires vm.debug
 * @library /test/lib
 * @run main/othervm -Xbootclasspath/a:. -XX:+UnlockDiagnosticVMOptions
 *      -XX:+UseJeandleCompiler
 *      -XX:+CITimeEach -XX:+CITimeVerbose
 *      -XX:-TieredCompilation -Xcomp
 *      -XX:CompileCommand=compileonly,TestTimeTracing::*
 *      -XX:CompileCommand=exclude,TestTimeTracing::main
 *      TestTimeTracing
 */

/**
 * @test
 * @summary Verify that Jeandle time tracing works with CITimeEach/CITimeVerbose on debug builds
 * @requires vm.debug
 * @library /test/lib
 * @run main/othervm -Xbootclasspath/a:. -XX:+UnlockDiagnosticVMOptions
 *      -XX:+UseJeandleCompiler
 *      -XX:+CITime
 *      -XX:+CITimeEach -XX:+CITimeVerbose
 *      -XX:-TieredCompilation -Xcomp
 *      -XX:CompileCommand=compileonly,TestTimeTracing::*
 *      -XX:CompileCommand=exclude,TestTimeTracing::main
 *      TestTimeTracing
 */

public class TestTimeTracing {
    public static int fibonacci(int n) {
        if (n == 0) {
            return 0;
        } else if (n == 1) {
            return 1;
        } else {
            return fibonacci(n - 1) + fibonacci(n - 2);
        }
    }
    public static int add() {
        int total = 0;
        for (int i = 1; i < 1000; i++) {
            total = total + i;
        }
        return total;
    }
    public static int subtraction() {
        int total = 1000;
        while (total > 0) {
            total = total - 1;
        }
        return total;
    }
    public static void main(String[] args) {
        int fib = 0;
        for (int i = 0; i < 10; i++) {
            fib = fibonacci(i);
        }
        int sum = add();
        int sub = subtraction();

        int expectedFib = 34;      // fibonacci(9)
        int expectedSum = 499500;  // sum(1..999)
        int expectedSub = 0;       // 1000 - 1 - 1 - ... - 1

        if (fib != expectedFib || sum != expectedSum || sub != expectedSub) {
            throw new RuntimeException(
                String.format("Test failed: fib=%d (expected %d), " +
                             "sum=%d (expected %d), sub=%d (expected %d)",
                             fib, expectedFib, sum, expectedSum, sub, expectedSub));
        }
        System.out.println("TEST PASSED");
    }
}
