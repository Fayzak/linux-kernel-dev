PWD := $(shell pwd)
KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build

DRV_NAME := kernel_timer

.PHONY: build load unload clean format check

build:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

load:
	insmod $(PWD)/$(DRV_NAME).ko

unload:
	rmmod $(DRV_NAME)

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean

format:
	find . -name '*.c' -o -name '*.h' | xargs clang-format -i --style=LLVM

check:
	dmesg -w
