#ifndef EOL_FORMAT_H
#define EOL_FORMAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

bool EOL_FormatFixedOneDecimal(char* dst, size_t dstLen, float value);
bool EOL_FormatAnalogExpected(bool isCurrent, float expected, float tolerance, char* dst, size_t dstLen);
bool EOL_FormatAnalogActual(bool isCurrent, float actual, char* dst, size_t dstLen);

#ifdef __cplusplus
}
#endif

#endif /* EOL_FORMAT_H */
