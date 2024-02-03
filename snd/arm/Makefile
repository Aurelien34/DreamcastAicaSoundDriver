
TARGET = AicaD.drv
OBJS = crt0.o AicaDriver.o MessageQueue.o CommandQueue.o WaveManager.o MusicManager.o BasicFunctions.o Aica.o ChannelManager.o 

ADDITIONAL_CFLAGS = -DARM7 -DARM -I/home/bouz/mp3/fixpt/pub -I/home/bouz/mp3/fixpt/real -I/home/bouz/mp3/fixpt/real/arm


target:
	make $(TARGET)

all:
	make clean
	make $(TARGET)

$(TARGET): AicaD.elf
	$(DC_ARM_OBJCOPY) -O binary AicaD.elf $(TARGET)

$(OBJS): *.h ../AicaCommon.h

AicaD.elf: $(OBJS)
	$(DC_ARM_CC) -Wl,-Ttext,0x00000000 -nostartfiles -nostdlib -o AicaD.elf $(OBJS) -lgcc

%.o: %.c
	$(DC_ARM_CC) $(DC_ARM_CFLAGS) $(DC_ARM_INCS) $(ADDITIONAL_CFLAGS) -c $< -o $@

%.o: %.s
	$(DC_ARM_AS) $(DC_ARM_AFLAGS) $< -o $@

clean:
	-rm -f $(TARGET) $(OBJS) AicaD.elf
	-rm -f *~


