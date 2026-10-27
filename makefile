CC = gcc
CFLAGS_EMU = -O2 -lwiringPi -lSDL2
CFLAGS_HANDLER = -O2 -lwiringPi
FILE_EMU = C-GBoy.c
FILE_HANDLER = ROM_selector.c
EMU = C-Gboy
EMU_HANDLER = C-Gboy_handler


build:
	mkdir -p ROMs
	$(CC) $(FILE_EMU) -o $(EMU) $(CFLAGS_EMU)
	$(CC) $(FILE_HANDLER) -o $(EMU_HANDLER) $(CFLAGS_HANDLER)