/**
 * @brief Symbolic execution assignment using KLEE
 *
 * Please use KLEE to analyze the target function.
 *
 * Guide:
 * 1. Construct a symbolization driver for the function.
 * 2. Define appropriate symbolic inputs using KLEE APIs, such as klee_make_symbolic.
 * 3. Execute KLEE to explore program paths and generate test cases.
 * 4. Analyze the generated test cases and coverage results.
 *
 * Question:
 * 1. How many paths does KLEE explore, and why?
 * 2. Is it possible to achieve 100% statement coverage and branch coverage? Why?
 * 3. Any bug found?
 *
 * You may refer to the following tutorial to seek help:
 * https://klee-se.org/tutorials/testing-function/
 */

int target(int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            sum += i;
        } else {
            sum -= i;
        }

        if (i % 3 == 0)
            sum += 2 * i;
        else
            sum -= i;
    }
    return sum;
}
