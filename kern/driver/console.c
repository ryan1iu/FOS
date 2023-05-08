#include <inc/console.h>
#include <inc/stdio.h>

static void move_cursor() {
    uint16_t location = cursor_y * 80 + cursor_x;
    outb(0x3D4, 14);
    // 发送高8位
    outb(0x3D5, location >> 8);
    outb(0x3D4, 15);
    // 发送低8位
    outb(0x3D5, location);
}

static void scroll() {
    uint8_t attribute_byte = (0 /*black*/ << 4) | (15 /*white*/ & 0x0F);
    uint16_t blank = 0x20 /* space */ | (attribute_byte << 8);

    if (cursor_y >= 25) {
        int i;
        for (i = 0 * 80; i < 24 * 80; i++) {
            vga_ptr[i] = vga_ptr[i + 80];
        }

        for (i = 24 * 80; i < 25 * 80; i++) {
            vga_ptr[i] = blank;
        }
        cursor_y = 24;
    }
}

static void vga_putc(char c, uint8_t color) {
    uint16_t *location;
    uint8_t attribute_byte = (COLOR_BLACK << 4) | (color & 0x0f);
    uint16_t attribute = attribute_byte << 8;

    // 如果是backspace
    if (c == 0x08 && cursor_x) {
        cursor_x--;
    } else if (c == 0x09) {
        cursor_x = (cursor_x + 8) & ~(8 - 1);
    }

    // 处理回车
    else if (c == '\r') {
        cursor_x = 0;
    }

    // 通过将光标向左移动并增加行来处理换行
    else if (c == '\n') {
        cursor_x = 0;

        cursor_y++;
    }

    // 处理任何其他可打印字符。
    else if (c >= ' ') {
        location = vga_ptr + (cursor_y * 80 + cursor_x);

        *location = c | attribute;
        cursor_x++;
    }

    // 检查我们是否需要插入一个新行，因为我们已经到了末尾
    if (cursor_x >= 80) {
        cursor_x = 0;

        cursor_y++;
    }

    scroll();

    // 移动光标
    move_cursor();
}

static void clear_screen() {
    uint8_t attribute_byte = (0 /*black*/ << 4) | (15 /*white*/ & 0x0F);
    uint16_t blank = 0x20 /* space */ | (attribute_byte << 8);

    int i;
    for (i = 0; i < 80 * 25; i++) {
        vga_ptr[i] = blank;
    }

    cursor_x = 0;
    cursor_y = 0;

    move_cursor();
}

void console_init() { clear_screen(); }

void console_putc(int c, uint8_t color) { vga_putc(c, color); }
