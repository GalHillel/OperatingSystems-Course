# Encrypted Memory Char Device

This project implements a character device for encrypted memory in the Linux kernel.

## Build

To build the kernel module, execute the following command:

make

## Load the Module

To load the kernel module, execute the following command:

sudo insmod char_device.ko
sudo insmod sysfs_entry.ko

## Usage

To interact with the char device, you can use the provided `main.c` file.
You can change the key and adjust the buffer size as needed.
Compile and run the `main.c` file with the following commands:

make main  
./main

## Unload the Module

To unload the kernel module, execute the following command:

sudo rmmod sysfs_entry  
sudo rmmod char_device
