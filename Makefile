obj-m += rnd_gen.o

rnd_gen-objs := ./src/rnd_gen.o 

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean


