# If KERNELRELEASE is not defined, we called directly
# from the command line; invoke the kernel build system
ifeq ($(KERNELRELEASE),)

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

.PHONY: modules clean

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(RM) -r *.o *.symvers .*.cmd *.mod* *.order *.ko

# Otherwise we've been invoked from the kernel build system
# ad can use its language
else 

obj-m := hello.o

endif