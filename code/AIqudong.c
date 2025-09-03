#include "AIqudong.h"

static driver_buffer_t dev_buffer = {0};
static driver_buffer_t proc_buffer = {0};

static atomic_t hook_counter = ATOMIC_INIT(0);

int base64_encode(const unsigned char *src, int len, char *dst) {
    int i, j;
    for (i = 0, j = 0; i < len; i += 3, j += 4) {
        unsigned char a = src[i];
        unsigned char b = (i + 1 < len) ? src[i + 1] : 0;
        unsigned char c = (i + 2 < len) ? src[i + 2] : 0;
        
        dst[j] = base64_table[a >> 2];
        dst[j + 1] = base64_table[((a & 0x03) << 4) | (b >> 4)];
        dst[j + 2] = (i + 1 < len) ? base64_table[((b & 0x0f) << 2) | (c >> 6)] : '=';
        dst[j + 3] = (i + 2 < len) ? base64_table[c & 0x3f] : '=';
    }
    dst[j] = '\0';
    return j;
}

int base64_decode(const char *src, unsigned char *dst) {
    int len = strlen(src);
    int i, j;
    for (i = 0, j = 0; i < len; i += 4, j += 3) {
        unsigned char a = strchr(base64_table, src[i]) - base64_table;
        unsigned char b = strchr(base64_table, src[i + 1]) - base64_table;
        unsigned char c = (src[i + 2] == '=') ? 0 : strchr(base64_table, src[i + 2]) - base64_table;
        unsigned char d = (src[i + 3] == '=') ? 0 : strchr(base64_table, src[i + 3]) - base64_table;
        
        dst[j] = (a << 2) | (b >> 4);
        dst[j + 1] = (b << 4) | (c >> 2);
        dst[j + 2] = (c << 6) | d;
    }
    dst[j] = '\0';
    return j;
}

static ssize_t device_write(struct file *file, const char __user *buffer, size_t length, loff_t *offset) {
    if (length > BUFFER_SIZE - dev_buffer.length)
        return -EINVAL;
    
    if (copy_from_user(dev_buffer.data + dev_buffer.length, buffer, length))
        return -EFAULT;
    
    dev_buffer.length += length;
    dev_buffer.data[dev_buffer.length] = '\0';
    
    return length;
}

static ssize_t device_read(struct file *file, char __user *buffer, size_t length, loff_t *offset) {
    char encoded_data[BUFFER_SIZE * 2];
    int encoded_len;
    
    if (*offset > 0)
        return 0;
    
    encoded_len = base64_encode(dev_buffer.data, dev_buffer.length, encoded_data);
    
    if (copy_to_user(buffer, encoded_data, encoded_len))
        return -EFAULT;
    
    *offset = encoded_len;
    return encoded_len;
}

static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    char temp_buffer[BUFFER_SIZE];
    unsigned char decoded_data[BUFFER_SIZE];
    int decoded_len;
    
    switch (cmd) {
        case 0x100:
            decoded_len = base64_decode(dev_buffer.data, decoded_data);
            printk(KERN_INFO "AIqudong: Decoded %d bytes\n", decoded_len);
            break;
        case 0x101:
            base64_encode(decoded_data, decoded_len, temp_buffer);
            printk(KERN_INFO "AIqudong: Encoded data: %s\n", temp_buffer);
            break;
        default:
            return -EINVAL;
    }
    
    return 0;
}

static int device_open(struct inode *inode, struct file *file) {
    atomic_inc(&hook_counter);
    printk(KERN_INFO "AIqudong: Device opened, hook count: %d\n", atomic_read(&hook_counter));
    return 0;
}

static int device_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "AIqudong: Device closed, hook count: %d\n", atomic_read(&hook_counter));
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = device_read,
    .write = device_write,
    .unlocked_ioctl = device_ioctl,
    .open = device_open,
    .release = device_release,
};

static struct miscdevice aiqudong_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_NAME,
    .fops = &fops,
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
static struct proc_ops proc_fops = {
    .proc_read = proc_read,
};
#else
static struct file_operations proc_fops = {
    .read = proc_read,
};
#endif

static ssize_t proc_read(struct file *file, char __user *buffer, size_t length, loff_t *offset) {
    char info[512];
    int len;
    
    if (*offset > 0)
        return 0;
    
    len = snprintf(info, sizeof(info), 
                  "AIqudong Driver Information:\n"
                  "Version: 3.1\n"
                  "Device buffer size: %d\n"
                  "Hook count: %d\n"
                  "Base64 operations: %lu\n",
                  dev_buffer.length, 
                  atomic_read(&hook_counter),
                  jiffies);
    
    if (copy_to_user(buffer, info, len))
        return -EFAULT;
    
    *offset = len;
    return len;
}

static int __init aiqudong_init(void) {
    int ret;
    
    ret = misc_register(&aiqudong_device);
    if (ret) {
        printk(KERN_ERR "AIqudong: Failed to register device\n");
        return ret;
    }
    
    proc_create(PROC_NAME, 0, NULL, &proc_fops);
    
    printk(KERN_INFO "AIqudong: Driver initialized successfully\n");
    printk(KERN_INFO "AIqudong: /dev/%s created\n", DEVICE_NAME);
    printk(KERN_INFO "AIqudong: /proc/%s created\n", PROC_NAME);
    
    return 0;
}

static void __exit aiqudong_exit(void) {
    misc_deregister(&aiqudong_device);
    remove_proc_entry(PROC_NAME, NULL);
    
    printk(KERN_INFO "AIqudong: Driver removed\n");
    printk(KERN_INFO "AIqudong: /dev/%s removed\n", DEVICE_NAME);
    printk(KERN_INFO "AIqudong: /proc/%s removed\n", PROC_NAME);
}

module_init(aiqudong_init);
module_exit(aiqudong_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DeepSeek");
MODULE_DESCRIPTION("AIqudong Virtual Kernel Driver with Hook");
MODULE_VERSION("3.1");