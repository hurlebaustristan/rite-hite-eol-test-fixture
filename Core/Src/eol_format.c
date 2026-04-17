#include "eol_format.h"

#include <stdint.h>
#include <string.h>

static bool appendChar(char* dst, size_t dstLen, size_t* index, char ch)
{
    if ((dst == 0) || (index == 0) || (*index >= (dstLen - 1u)))
    {
        return false;
    }

    dst[(*index)++] = ch;
    dst[*index] = '\0';
    return true;
}

static bool appendLiteral(char* dst, size_t dstLen, size_t* index, const char* text)
{
    size_t i = 0u;

    if ((dst == 0) || (index == 0) || (text == 0))
    {
        return false;
    }

    while (text[i] != '\0')
    {
        if (!appendChar(dst, dstLen, index, text[i]))
        {
            return false;
        }
        i++;
    }

    return true;
}

static bool appendUnsigned(char* dst, size_t dstLen, size_t* index, uint32_t value)
{
    char digits[10];
    size_t count = 0u;

    if ((dst == 0) || (index == 0))
    {
        return false;
    }

    do
    {
        digits[count++] = (char)('0' + (value % 10u));
        value /= 10u;
    } while ((value != 0u) && (count < sizeof(digits)));

    while (count > 0u)
    {
        if (!appendChar(dst, dstLen, index, digits[--count]))
        {
            return false;
        }
    }

    return true;
}

bool EOL_FormatFixedOneDecimal(char* dst, size_t dstLen, float value)
{
    int32_t scaled;
    uint32_t magnitude;
    uint32_t wholePart;
    uint32_t fractionalPart;
    size_t index = 0u;

    if ((dst == 0) || (dstLen < 4u))
    {
        return false;
    }

    dst[0] = '\0';

    scaled = (value >= 0.0f)
           ? (int32_t)(value * 10.0f + 0.5f)
           : (int32_t)(value * 10.0f - 0.5f);

    if (scaled < 0)
    {
        if (!appendChar(dst, dstLen, &index, '-'))
        {
            return false;
        }
        magnitude = (uint32_t)(-scaled);
    }
    else
    {
        magnitude = (uint32_t)scaled;
    }

    wholePart = magnitude / 10u;
    fractionalPart = magnitude % 10u;

    if (!appendUnsigned(dst, dstLen, &index, wholePart))
    {
        return false;
    }
    if (!appendChar(dst, dstLen, &index, '.'))
    {
        return false;
    }
    if (!appendChar(dst, dstLen, &index, (char)('0' + fractionalPart)))
    {
        return false;
    }

    return true;
}

bool EOL_FormatAnalogExpected(bool isCurrent, float expected, float tolerance, char* dst, size_t dstLen)
{
    char numberBuf[16];
    size_t index = 0u;

    if ((dst == 0) || (dstLen == 0u))
    {
        return false;
    }

    dst[0] = '\0';

    if (!EOL_FormatFixedOneDecimal(numberBuf, sizeof(numberBuf), expected))
    {
        return false;
    }
    if (!appendLiteral(dst, dstLen, &index, numberBuf))
    {
        return false;
    }
    if (!appendLiteral(dst, dstLen, &index, isCurrent ? "mA +/-" : "V +/-"))
    {
        return false;
    }
    if (!EOL_FormatFixedOneDecimal(numberBuf, sizeof(numberBuf), tolerance))
    {
        return false;
    }
    if (!appendLiteral(dst, dstLen, &index, numberBuf))
    {
        return false;
    }

    return true;
}

bool EOL_FormatAnalogActual(bool isCurrent, float actual, char* dst, size_t dstLen)
{
    char numberBuf[16];
    size_t index = 0u;

    if ((dst == 0) || (dstLen == 0u))
    {
        return false;
    }

    dst[0] = '\0';

    if (!EOL_FormatFixedOneDecimal(numberBuf, sizeof(numberBuf), actual))
    {
        return false;
    }
    if (!appendLiteral(dst, dstLen, &index, numberBuf))
    {
        return false;
    }
    if (!appendLiteral(dst, dstLen, &index, isCurrent ? "mA" : "V"))
    {
        return false;
    }

    return true;
}
