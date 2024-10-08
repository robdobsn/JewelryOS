#pragma once
#include <stdint.h>
namespace Font5x5
{
    constexpr uint8_t height = 5;
    constexpr uint8_t start = 0x20;
    constexpr uint8_t end = 0x7a;
    // Characters (first value in array is width)
    const uint8_t font[][6] = 
    {
        { 2, 0x00, 0x00, 0x00, 0x00, 0x00, }, // 0x20
        { 1, 0x80, 0x80, 0x80, 0x00, 0x80, }, // 0x21
        { 3, 0xa0, 0xa0, 0x00, 0x00, 0x00, }, // 0x22
        { 5, 0x50, 0xf8, 0x50, 0xf8, 0x50, }, // 0x23
        { 5, 0x70, 0xa0, 0x70, 0x28, 0x70, }, // 0x24
        { 5, 0xc8, 0xd0, 0x20, 0x58, 0x98, }, // 0x25
        { 5, 0x68, 0x90, 0xb0, 0xc8, 0x70, }, // 0x26
        { 1, 0x80, 0x80, 0x00, 0x00, 0x00, }, // 0x27
        { 2, 0x40, 0x80, 0x80, 0x80, 0x40, }, // 0x28
        { 2, 0x80, 0x40, 0x40, 0x40, 0x80, }, // 0x29
        { 5, 0xa8, 0x70, 0xf8, 0x70, 0xa8, }, // 0x2a
        { 4, 0x00, 0x20, 0x70, 0x20, 0x00, }, // 0x2b
        { 2, 0x00, 0x00, 0x00, 0x40, 0x80, }, // 0x2c
        { 3, 0x00, 0x00, 0xe0, 0x00, 0x00, }, // 0x2d
        { 1, 0x00, 0x00, 0x00, 0x00, 0x80, }, // 0x2e
        { 3, 0x00, 0x20, 0x40, 0x80, 0x00, }, // 0x2f
        { 5, 0x70, 0x88, 0xa8, 0x88, 0x70, }, // 0x30
        { 3, 0x40, 0xc0, 0x40, 0x40, 0xe0, }, // 0x31
        { 5, 0x70, 0x88, 0x30, 0xc0, 0xf8, }, // 0x32
        { 5, 0x70, 0x88, 0x18, 0x88, 0x70, }, // 0x33
        { 5, 0x10, 0x30, 0x50, 0xf8, 0x10, }, // 0x34
        { 5, 0xf8, 0x80, 0xf0, 0x08, 0xf0, }, // 0x35
        { 5, 0x38, 0x60, 0x98, 0x88, 0x70, }, // 0x36
        { 5, 0xf8, 0x08, 0x10, 0x20, 0x40, }, // 0x37
        { 5, 0x70, 0x88, 0x70, 0x88, 0x70, }, // 0x38
        { 5, 0x70, 0x88, 0xc8, 0x30, 0xe0, }, // 0x39
        { 0, 0x00, 0x00, 0x00, 0x00, 0x00, }, // 0x3a
        { 0, 0x00, 0x00, 0x00, 0x00, 0x00, }, // 0x3b
        { 0, 0x00, 0x00, 0x00, 0x00, 0x00, }, // 0x3c
        { 0, 0x00, 0x00, 0x00, 0x00, 0x00, }, // 0x3d
        { 0, 0x00, 0x00, 0x00, 0x00, 0x00, }, // 0x3e
        { 0, 0x00, 0x00, 0x00, 0x00, 0x00, }, // 0x3f
        { 5, 0x70, 0x88, 0x98, 0x98, 0x70, }, // 0x40
        { 4, 0x60, 0x90, 0xf0, 0x90, 0x90, }, // 0x41
        { 4, 0xe0, 0x90, 0xe0, 0x90, 0xf0, }, // 0x42
        { 4, 0x60, 0x90, 0x80, 0x90, 0x60, }, // 0x43
        { 4, 0xe0, 0x90, 0x90, 0x90, 0xe0, }, // 0x44
        { 3, 0xe0, 0x80, 0xc0, 0x80, 0xe0, }, // 0x45
        { 3, 0xe0, 0x80, 0xc0, 0x80, 0x80, }, // 0x46
        { 4, 0x60, 0x90, 0x80, 0xb0, 0x70, }, // 0x47
        { 4, 0x90, 0x90, 0xf0, 0x90, 0x90, }, // 0x48
        { 1, 0x80, 0x80, 0x80, 0x80, 0x80, }, // 0x49
        { 4, 0xf0, 0x20, 0x20, 0xa0, 0x40, }, // 0x4a
        { 4, 0x90, 0xa0, 0xc0, 0xa0, 0x90, }, // 0x4b
        { 3, 0x80, 0x80, 0x80, 0x80, 0xe0, }, // 0x4c
        { 5, 0xa8, 0xf8, 0xd8, 0x88, 0x88, }, // 0x4d
        { 4, 0x90, 0xd0, 0xb0, 0x90, 0x00, }, // 0x4e
        { 4, 0x60, 0x90, 0x90, 0x90, 0x60, }, // 0x4f
        { 4, 0xe0, 0x90, 0xe0, 0x80, 0x80, }, // 0x50
        { 4, 0x60, 0x90, 0x90, 0xb0, 0x70, }, // 0x51
        { 4, 0xe0, 0x90, 0xe0, 0xa0, 0x90, }, // 0x52
        { 4, 0x60, 0x80, 0x60, 0x10, 0x60, }, // 0x53
        { 3, 0xe0, 0x40, 0x40, 0x40, 0x40, }, // 0x54
        { 4, 0x90, 0x90, 0x90, 0x90, 0xf0, }, // 0x55
        { 4, 0x90, 0x90, 0x90, 0xa0, 0x40, }, // 0x56
        { 5, 0x88, 0x88, 0xd8, 0xf8, 0xa8, }, // 0x57
        { 5, 0x88, 0x50, 0x20, 0x50, 0x88, }, // 0x58
        { 4, 0x90, 0x90, 0x60, 0x20, 0x20, }, // 0x59
        { 3, 0xe0, 0x20, 0x40, 0x80, 0xe0, }, // 0x5a
        { 3, 0xe0, 0x80, 0x80, 0x80, 0xe0, }, // 0x5b
        { 3, 0x00, 0x20, 0x40, 0x80, 0x00, }, // 0x5c
        { 3, 0xe0, 0x20, 0x20, 0x20, 0xe0, }, // 0x5d
        { 3, 0x40, 0xa0, 0x00, 0x00, 0x00, }, // 0x5e
        { 3, 0x00, 0x00, 0x00, 0x00, 0xe0, }, // 0x5f
        { 0, 0x00, 0x00, 0x00, 0x00, 0x00, }, // 0x60
        { 4, 0x60, 0x90, 0xf0, 0x90, 0x90, }, // 0x61
        { 4, 0xe0, 0x90, 0xe0, 0x90, 0xf0, }, // 0x62
        { 4, 0x60, 0x90, 0x80, 0x90, 0x60, }, // 0x63
        { 4, 0xe0, 0x90, 0x90, 0x90, 0xe0, }, // 0x64
        { 3, 0xe0, 0x80, 0xc0, 0x80, 0xe0, }, // 0x65
        { 3, 0xe0, 0x80, 0xc0, 0x80, 0x80, }, // 0x66
        { 4, 0x60, 0x90, 0x80, 0xb0, 0x70, }, // 0x67
        { 4, 0x90, 0x90, 0xf0, 0x90, 0x90, }, // 0x68
        { 1, 0x80, 0x80, 0x80, 0x80, 0x80, }, // 0x69
        { 4, 0xf0, 0x20, 0x20, 0xa0, 0x40, }, // 0x6a
        { 4, 0x90, 0xa0, 0xc0, 0xa0, 0x90, }, // 0x6b
        { 3, 0x80, 0x80, 0x80, 0x80, 0xe0, }, // 0x6c
        { 5, 0xa8, 0xf8, 0xd8, 0x88, 0x88, }, // 0x6d
        { 4, 0x90, 0xd0, 0xb0, 0x90, 0x00, }, // 0x6e
        { 4, 0x60, 0x90, 0x90, 0x90, 0x60, }, // 0x6f
        { 4, 0xe0, 0x90, 0xe0, 0x80, 0x80, }, // 0x70
        { 4, 0x60, 0x90, 0x90, 0xb0, 0x70, }, // 0x71
        { 4, 0xe0, 0x90, 0xe0, 0xa0, 0x90, }, // 0x72
        { 4, 0x60, 0x80, 0x60, 0x10, 0x60, }, // 0x73
        { 3, 0xe0, 0x40, 0x40, 0x40, 0x40, }, // 0x74
        { 4, 0x90, 0x90, 0x90, 0x90, 0xf0, }, // 0x75
        { 4, 0x90, 0x90, 0x90, 0xa0, 0x40, }, // 0x76
        { 5, 0x88, 0x88, 0xd8, 0xf8, 0xa8, }, // 0x77
        { 5, 0x88, 0x50, 0x20, 0x50, 0x88, }, // 0x78
        { 4, 0x90, 0x90, 0x60, 0x20, 0x20, }, // 0x79
        { 3, 0xe0, 0x20, 0x40, 0x80, 0xe0, }, // 0x7a
    };
}
