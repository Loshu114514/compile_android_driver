obj-m += AIqudong.o

KDIR ?= /lib/modules/$(shell uname -r)/build

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

install: AIqudong.ko
	sudo insmod AIqudong.ko

uninstall:
	sudo rmmod AIqudong