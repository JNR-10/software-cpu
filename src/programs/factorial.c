/**
 * factorial.c - Reference Implementation for Factorial Function
 *
 * This is a reference C implementation of the recursive factorial function
 * that will be translated to assembly language for the Software CPU.
 *
 * Author: Jainil (The Algorithmist)
 * Phase: 2 - Function Calls, Stack Frames, and Recursion
 * Purpose:
 *   - Serve as a specification for the assembly implementation
 *   - Document the algorithm and expected behavior
 *   - Provide test cases and expected results
 */

#include <stdio.h>

/**
 * multiply - Software multiplication (corresponds to math.asm)
 *
 * In the actual assembly implementation, this will be the multiply
 * subroutine from math.asm.
 *
 * @param a - First operand
 * @param b - Second operand
 * @return Product of a and b
 */
int multiply(int a, int b) {
  int result = 0;

  // Shift-and-add algorithm
  while (b > 0) {
    // If b is odd, add a to result
    if (b & 1) {
      result += a;
    }
    // Shift a left (multiply by 2)
    a <<= 1;
    // Shift b right (divide by 2)
    b >>= 1;
  }

  return result;
}

/**
 * factorial - Recursive factorial calculation
 *
 * Mathematical definition:
 *   factorial(n) = n! = n × (n-1) × (n-2) × ... × 2 × 1
 *
 *   Base cases:
 *     factorial(0) = 1
 *     factorial(1) = 1
 *
 *   Recursive case:
 *     factorial(n) = n × factorial(n-1)  for n > 1
 *
 * Assembly Translation Notes:
 *   - Argument n is passed in R0
 *   - Return value is placed in R0
 *   - Must save n before recursive call (use stack)
 *   - After recursive call, multiply n with result
 *   - Use the multiply subroutine from math.asm
 *
 * @param n - Non-negative integer to compute factorial of
 * @return n! (factorial of n)
 */
int factorial(int n) {
  // =========================================================================
  // BASE CASE: If n <= 1, return 1
  // =========================================================================
  // This handles both factorial(0) = 1 and factorial(1) = 1
  // In assembly, this will be implemented using CMP and conditional jumps

  if (n <= 1) {
    return 1;
  }

  // =========================================================================
  // RECURSIVE CASE: n > 1
  // =========================================================================
  // Compute: n! = n × (n-1)!
  //
  // Assembly steps:
  // 1. Save current value of n (push to stack)
  // 2. Compute n-1 and place in R0
  // 3. Recursively call factorial(n-1)
  // 4. Save result of factorial(n-1)
  // 5. Restore original n value
  // 6. Call multiply(n, factorial(n-1))
  // 7. Return result

  // Recursive call to get (n-1)!
  int factorial_n_minus_1 = factorial(n - 1);

  // Multiply n by (n-1)! to get n!
  int result = multiply(n, factorial_n_minus_1);

  return result;
}

/**
 * main - Test driver for factorial function
 *
 * Tests the factorial function with various inputs and displays results.
 */
int main(void) {
  printf("=================================================\n");
  printf("Factorial Function - Reference Implementation\n");
  printf("=================================================\n\n");

  // Test cases
  int test_cases[] = {0, 1, 2, 3, 4};
  int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);

  printf("Testing factorial function:\n\n");

  for (int i = 0; i < num_tests; i++) {
    int n = test_cases[i];
    int result = factorial(n);
    printf("  factorial(%d) = %d\n", n, result);
  }

  printf("\n=================================================\n");
  printf("Expected Results:\n");
  printf("=================================================\n");
  printf("  factorial(0) = 1\n");
  printf("  factorial(1) = 1\n");
  printf("  factorial(2) = 2\n");
  printf("  factorial(3) = 6\n");
  printf("  factorial(4) = 24\n");
  printf("=================================================\n\n");

  // Special focus on factorial(4) which is the target for Phase 2
  printf("Phase 2 Target: factorial(4)\n");
  printf("  Input:  n = 4\n");
  printf("  Output: 4! = %d\n", factorial(4));
  printf("  Expected: 24\n");
  printf("\n");

  // Show the recursive calls for factorial(4)
  printf("Recursive call trace for factorial(4):\n");
  printf("  factorial(4) calls factorial(3)\n");
  printf("    factorial(3) calls factorial(2)\n");
  printf("      factorial(2) calls factorial(1)\n");
  printf("        factorial(1) returns 1 (base case)\n");
  printf("      factorial(2) = multiply(2, 1) = 2\n");
  printf("    factorial(3) = multiply(3, 2) = 6\n");
  printf("  factorial(4) = multiply(4, 6) = 24\n");
  printf("\n");

  return 0;
}

/**
 * Compilation and Execution:
 *   gcc -o factorial factorial.c
 *   ./factorial
 *
 * Notes for Assembly Implementation:
 *
 * 1. Stack Frame Management:
 *    - Each recursive call creates a new stack frame
 *    - Must save return address (done automatically by CALL)
 *    - Must save registers that will be overwritten
 *    - Must restore registers before returning
 *
 * 2. Register Usage:
 *    - R0: Argument n (input), result (output)
 *    - R1: Used by multiply as second argument
 *    - R2: Temporary storage
 *    - R3: Frame pointer (if needed)
 *
 * 3. Calling Convention:
 *    - Arguments passed in R0
 *    - Return value in R0
 *    - Caller-saved: R0, R1, R2
 *    - Callee-saved: R3 (FP)
 *
 * 4. Maximum Recursion Depth:
 *    - Stack size: 28 KB (0x1000 to 0x7FFF)
 *    - Each call: ~6-8 bytes (return addr + saved n + saved registers)
    //    - Maximum depth: ~3500-4600 calls
    //    - factorial(4): only 4 calls - well within limits
 *
 * 5. Testing Strategy:
    //    - Test base cases: factorial(0), factorial(1)
    //    - Test small values: factorial(2), factorial(3)
    //    - Test target: factorial(4) = 24
 *    - Verify stack is properly managed (SP returns to initial value)
 */
