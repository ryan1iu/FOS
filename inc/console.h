#ifndef INC_CONSOLE_H
#define INC_CONSOLE_H

#include <inc/memlayout.h>
#include <inc/x86.h>

// vga 缓冲区起始地址
static uint16_t *vga_ptr = (uint16_t *)(KERNBASE + 0xB8000);

#define COLOR_BLACK 0
#define COLOR_BLUE 1
#define COLOR_GREEN 2
#define COLOR_CYAN 3
#define COLOR_RED 4
#define COLOR_FUCHSINE 5
#define COLOR_BROWN 6
#define COLOR_WHITE 7

#define COLOR_LIGHT_BLACK 8
#define COLOR_LIGHT_BLUE 9
#define COLOR_LIGHT_GREEN 10
#define COLOR_LIGHT_CYAN 11
#define COLOR_LIGHT_RED 12
#define COLOR_LIGHT_FUCHSINE 13
#define COLOR_LIGHT_BROWN 14
#define COLOR_LIGHT_WHITE 15

// vga code
// 光标位置
static uint8_t cursor_x;
static uint8_t cursor_y;

void console_init(void);

void console_putc(int c, uint8_t color);

#endif
