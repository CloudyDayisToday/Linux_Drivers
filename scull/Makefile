# If KERNELRELEASE is not defined, we called directly
# from the command line; invoke the kernel build system
ifeq ($(KERNELRELEASE),)

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

.PHONY: modules clean test

default: test modules

test: test_scull_drivers.c 
	gcc test_scull_drivers.c -o test_scull_drivers

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(RM) -r *.o *.symvers .*.cmd *.mod* *.order *.ko

# Otherwise we've been invoked from the kernel build system
# and can use its language
else 

obj-m := scull.o

endif



