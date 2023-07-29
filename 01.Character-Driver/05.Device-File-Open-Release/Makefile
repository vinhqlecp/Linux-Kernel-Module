MODULE_NAME = vchar_driver
KDIR = /lib/modules/$(shell uname -r)/build

all:
	make -C $(KDIR) M=$(PWD)
	cc -o user_test user_test.c

clean:
	make -C $(KDIR) M=$(PWD) clean
	rm user_test

install:
	sudo insmod $(MODULE_NAME).ko

uninstall:
	sudo rmmod $(MODULE_NAME)