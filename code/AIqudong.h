#ifndef AIQUDONG_H
#define AIQUDONG_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/proc_fs.h>
#include <linux/version.h>

#define DEVICE_NAME "aiqudong"
#define PROC_NAME "aiqudong_info"
#define BUFFER_SIZE 1024

static unsigned char base64_table[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

int base64_encode(const unsigned char *src, int len, char *dst);
int base64_decode(const char *src, unsigned char *dst);

typedef struct {
    char data[BUFFER_SIZE];
    int length;
} driver_buffer_t;

extern driver_buffer_t dev_buffer;
extern driver_buffer_t proc_buffer;

#endif