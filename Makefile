.PHONY: all clean

CC = avr-gcc
CFLAGS += -Os -Wall -mmcu=attiny45

all: dali-usb.elf dali-usb.hex

%.s: %.c
	$(CC) -S $(CFLAGS) -c $<

dali-usb.elf: $(wildcard *.c) $(wildcard *.h)
	$(CC) $(CFLAGS) $(wildcard *.c) -o $@

dali-usb.hex: dali-usb.elf
	avr-objcopy -O ihex $< $@

clean:
	$(RM) dali-usb.elf dali-usb.hex *.s *.o
