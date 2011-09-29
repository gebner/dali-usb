.PHONY: all flash clean

CC = avr-gcc
CFLAGS += -Wall -mmcu=attiny45 -DF_CPU=16500000UL

CFLAGS += -Os -Wl,--relax,--gc-sections \
	-ffunction-sections -fdata-sections

CFILES = $(wildcard *.c vusb/*.c vusb/*.S)
HEADERS = $(wildcard *.h vusb/*.h)

all: dali-usb.elf dali-usb.hex

%.s: %.c
	$(CC) -S -fverbose-asm $(CFLAGS) -c $<

dali-usb.elf: $(CFILES) $(HEADERS)
	$(CC) $(CFLAGS) -I. -Ivusb $(CFILES) -o $@

dali-usb.hex: dali-usb.elf
	avr-objcopy -O ihex $< $@

flash: dali-usb.hex
	avrdude -p t45 -c buspirate -P /dev/ttyUSB0 -V -U dali-usb.hex

clean:
	$(RM) dali-usb.elf dali-usb.hex *.s *.o
