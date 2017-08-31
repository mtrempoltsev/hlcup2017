#pragma once

#include <cstdint>

char* write(char* buffer, char c);
char* write(char* buffer, const char* str);
char* write(char* buffer, int32_t i);
char* write(char* buffer, uint32_t u);
char* write(char* buffer, double d);
