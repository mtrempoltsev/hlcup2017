#include <cstdio>

#include "in_place_writer.h"

const char digit_pairs[201] = {
    "00010203040506070809"
    "10111213141516171819"
    "20212223242526272829"
    "30313233343536373839"
    "40414243444546474849"
    "50515253545556575859"
    "60616263646566676869"
    "70717273747576777879"
    "80818283848586878889"
    "90919293949596979899"
};

char* write(char* buffer, char c)
{
    *buffer++ = c;
    return buffer;
}

char* write(char* buffer, const char* str)
{
    while (char c = *str++)
        *buffer++ = c;
    return buffer;
}

uint32_t get_size(uint32_t value)
{
    if (value >= 10000)
    {
        if (value >= 10000000)
        {
            if (value >= 1000000000)
                return 10;
            else if (value >= 100000000)
                return 9;
            else
                return 8;
        }
        else
        {
            if (value >= 1000000)
                return 7;
            else if (value >= 100000)
                return 6;
            else
                return 5;
        }
    }
    else
    {
        if (value >= 100)
        {
            if (value >= 1000)
                return 4;
            else
                return 3;
        }
        else
        {
            if (value >= 10)
                return 2;
            else
                return 1;
        }
    }
}

void write(char* buffer, uint32_t size, uint32_t value)
{
    char* c = buffer + size - 1;

    while(value >= 100)
    {
        int32_t pos = value % 100;
        value /= 100;
        *(int16_t*)(c - 1) = *(int16_t*)(digit_pairs + 2 * pos);
        c -= 2;
    }

    while (value > 0)
    {
        *c-- = '0' + (value % 10);
        value /= 10;
    }
}

char* write(char* buffer, int32_t i)
{
    if (i == 0)
    {
        *buffer++ = '0';
        return buffer;
    }

    const int32_t sign = i < 0 ? -1 : 0;
    uint32_t value = (i ^ sign) - sign;

    const int32_t size = get_size(value) - sign;

    if (sign < 0)
        *buffer = '-';

    write(buffer, size, value);

    return buffer + size;
}

char* write(char* buffer, uint32_t u)
{
    if (u == 0)
    {
        *buffer++ = '0';
        return buffer;
    }

    const int size = get_size(u);

    write(buffer, size, u);

    return buffer + size;
}

char* write(char* buffer, double d)
{
    const auto n = sprintf(buffer, "%g", d);
    return buffer + n;
}
