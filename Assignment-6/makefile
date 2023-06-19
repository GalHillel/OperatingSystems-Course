obj-m += char_device.o
obj-m += sysfs_entry.o

all:
	gcc -o main main.c
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	rm -f main
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	
