/*
 * PSn00bSDK stdlib div() implementation
 * (C) 2023 saxbophone - MPL licensed
 */

#include <stdlib.h>

div_t div(int x, int y) {
    div_t result = {0};
    /*
     * the MIPS R3000 DIV instruction calculates both quotient and remainder in
     * one go, so we take advantage of it using inline assembly, for speed.
     */
    /*
     * volatile might not be needed here
     * looks like optimiser is smart enough to tell that mflo/hi are not constant
     */
    __asm__ /*volatile*/ (
        "div $0, %2, %3;" // divide x by y (dst is set to $0 to request manual division mode)
                          // NOTE: on the R3000, division by zero doesn't trap in this mode
        "mflo %0;"        // copy quotient out
        "mfhi %1;"        // copy remainder out
      : "=r" (result.quot), "=r" (result.rem)
      : "r" (x), "r" (y)
    );
    return result;
}
