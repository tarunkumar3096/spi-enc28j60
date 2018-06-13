objects := $(patsubst src/%.c, %.o, $(wildcard src/*.c))

led: $(objects)
	arm-none-linux-gnueabi-gcc led.c $^ -I header/ -o $@

loopback: $(objects)
	arm-none-linux-gnueabi-gcc loopback.c $^ -I header/ -o $@

tx: $(objects)
	arm-none-linux-gnueabi-gcc tx_data.c $^ -I header/ -o $@

%.o: src/%.c
	arm-none-linux-gnueabi-gcc -c $^ -I header/

clean:
	rm *.o
	rm led tx loopback
