TARGET = test.elf
OBJS = Test.o ../snd/AicaDriver.o romdisk.o ../common/Controleur.o ../common/ControleurBase.o
ARM7DRIVER = AicaD.drv
ROMD = romdisk/$(ARM7DRIVER) romdisk/*
KOS_CFLAGS += -DSH4

all:
	rm -f romdisk/$(ARM7DRIVER)
	make $(TARGET)

rebuild:
	make clean
	make all

romdisk/$(ARM7DRIVER):
	make -C ../snd/arm $(ARM7DRIVER)
	cp ../snd/arm/$(ARM7DRIVER) romdisk/
    
include $(KOS_BASE)/Makefile.rules

clean:
	-rm -f $(TARGET) $(OBJS) romdisk.*
	-rm -f *~
	-rm -f romdisk/$(ARM7DRIVER)
rm-elf:
	-rm -f $(TARGET) romdisk.*

../common/commun.h.gch: ../common/commun.h
	$(KOS_CCPLUS) $(KOS_CFLAGS) ../common/commun.h

$(OBJS): ../snd/*.h ../common/*.h ../common/commun.h.gch

$(TARGET): $(OBJS)
	$(KOS_CCPLUS) $(KOS_CFLAGS) $(KOS_LDFLAGS) -o $(TARGET) $(KOS_START) $(OBJS) $(OBJEXTRA) -lm -lstdc++ -lkosutils $(KOS_LIBS)

romdisk.img: $(ROMD)
	$(KOS_GENROMFS) -f romdisk.img -d romdisk -v -x .svn -x *~

romdisk.o: romdisk.img
	$(KOS_BASE)/utils/bin2o/bin2o romdisk.img romdisk romdisk.o

run: $(TARGET)
	$(KOS_STRIP) $(TARGET)
	$(KOS_LOADER) $(TARGET)

dist:
	rm -f $(OBJS) romdisk.img
	$(KOS_STRIP) $(TARGET)
