ECOS_GLOBAL_CFLAGS = -mcpu=arm7tdmi -Wall -Wpointer-arith -Wstrict-prototypes -Winline -Wundef -Woverloaded-virtual -O2 -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -fvtable-gc -finit-priority
ECOS_GLOBAL_LDFLAGS = -mcpu=arm7tdmi -nostdlib -Wl,--gc-sections -Wl,-static
ECOS_COMMAND_PREFIX = arm-elf-
