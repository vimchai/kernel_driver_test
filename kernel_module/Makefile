KDIR := /lib/modules/`uname -r`/build
PWD := $(shell pwd)

obj-m := hello_module.o
hello_module-objs := hello.o \
					hello_test.o

default:
	make -C  $(KDIR) M=$(PWD)  modules 

clean:
	rm -rf *.o *.ko *.order *.mod.c .tmp_versions Module.symvers .*.cmd