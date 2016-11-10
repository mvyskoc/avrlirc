# MCU name
MCU = attiny25

# Processor frequency.
F_CPU = 16000000
F_CPU_MHZ = $(shell echo "$(F_CPU)/1000000" | bc)
# Target file name (without extension).
TARGET = avrlirc_$(MCU)_$(F_CPU_MHZ)MHz

# List any extra directories to look for include files here.
#     Each directory must be seperated by a space.
#     Use forward slashes for directory separators.
#     For a directory that has spaces, enclose it in quotes.
EXTRAINCDIRS = 

# List any extra directories to look for compile source files.
EXTRASRCDIRS = 

# List of C-source files. From these files is linked target application.
# Each file must be separated by a space 
CSRC = avrlirc.c

# Target files directory
TARGET_DIR = bin

COPY_DIR = ../bin/firmware

# Output format. (can be srec, ihex, binary)
FORMAT = ihex

# Object files directory
#     To put object files in current directory, use a dot (.), do NOT make
#     this an empty or blank macro!
OBJDIR = obj

# Optimization level, can be [0, 1, 2, 3, s]. 
#     0 = turn off optimization. s = optimize for size.
#     (Note: 3 is not always the best optimization level. See avr-libc FAQ.)
OPT = s

# Debugging format.
#     Native formats for AVR-GCC's -g are dwarf-2 [default] or stabs.
#     AVR Studio 4.10 requires dwarf-2.
#     AVR [Extended] COFF format requires stabs, plus an avr-objcopy run.
DEBUG = dwarf-2

# Path where MAKE command will find other source files
# directory names are separated by colons or blanks (On MS-Windows, 
# semi-colons are used as separators of directory names in VPATH
VPATH = $(EXTRAINCDIRS) $(EXTRASRCDIRS) 

# Compiler flag to set the C Standard level.
#     c89   = "ANSI" C
#     gnu89 = c89 plus GCC extensions
#     c99   = ISO C99 standard (not yet fully implemented)
#     gnu99 = c99 plus GCC extensions
CSTANDARD = -std=gnu99

# Place -D or -U options here for C sources
CDEFS = -DF_CPU=$(F_CPU)UL

#---------------- Compiler Options C ----------------
#  -g*:          generate debugging information
#  -O*:          optimization level
#  -f...:        tuning, see GCC manual and avr-libc documentation
#  -Wall...:     warning level
#  -Wa,...:      tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing
CFLAGS = -g$(DEBUG)
CFLAGS += $(CDEFS)
CFLAGS += -O$(OPT)
CFLAGS += -funsigned-char
CFLAGS += -funsigned-bitfields
CFLAGS += -fpack-struct
CFLAGS += -fshort-enums
CFLAGS += -Wall
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wno-unused-function 
# suppress warnings for not used functions
# direct compiler to generate a section for each function and variable, 
# allowing these to be left out on an individual basis, rather than just at a file level
CFLAGS += -ffunction-sections		
CFLAGS += -fdata-sections
#CFLAGS += -mshort-calls
#CFLAGS += -fno-unit-at-a-time
#CFLAGS += -Wundef
#CFLAGS += -Wunreachable-code
#CFLAGS += -Wsign-compare
#CFLAGS += -Wa,-adhlns=$(<:%.c=$(OBJDIR)/%.lst)
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))
CFLAGS += $(CSTANDARD)
VERSION = $(shell date +%y%m%d-%H%M)
CFLAGS += -DAVRLIRC_VERSION="\"$(VERSION)\""


#---------------- Library Options ----------------
# List any extra directories to look for libraries here.
#     Each directory must be seperated by a space.
#     Use forward slashes for directory separators.
#     For a directory that has spaces, enclose it in quotes.
EXTRALIBDIRS = 


#---------------- Linker Options ----------------
#  -Wl,...:     tell GCC to pass this to linker.
#    -Map:      create map file
#    --cref:    add cross reference to  map file
LDFLAGS = -Wl,-Map=$(TARGET_DIR)/$(TARGET).map,--cref
LDFLAGS += -Wl,-gc-sections
LDFLAGS += $(patsubst %,-L%,$(EXTRALIBDIRS))

# Compiler flags to generate dependency files.
GENDEPFLAGS = -MMD -MP -MF $(OBJDIR)/$(@F).d

# Combine all necessary flags and optional flags.
# Add target processor to flags.
ALL_CFLAGS = -mmcu=$(MCU) -I. $(CFLAGS) $(GENDEPFLAGS)


# Define programs and commands.
SHELL = sh
CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size
REMOVE = rm -f
REMOVEDIR = rm -rf
MOVE = mv -f
COPY = cp
MKDIR = mkdir -p
NM = avr-nm

.PHONY: all clean copy size binary progavr

TARGETS = $(addprefix $(TARGET_DIR)/$(TARGET)., elf hex eep lss sym bin map)

# Default action
all: elf hex binary lss eep sym size copy

$(TARGET_DIR):
	$(MKDIR) $@

$(OBJDIR): 
	$(MKDIR) $@

elf:  $(TARGET_DIR)/$(TARGET).elf
hex:  $(TARGET_DIR)/$(TARGET).hex
binary: $(TARGET_DIR)/$(TARGET).bin
lss:  $(TARGET_DIR)/$(TARGET).lss
eep:  $(TARGET_DIR)/$(TARGET).eep
sym:  $(TARGET_DIR)/$(TARGET).sym

clean:
	$(REMOVE) $(OBJDIR)/*.o $(OBJDIR)/*.o.d
	$(REMOVE) $(TARGET_DIR)/$(TARGET).lss.old
	$(REMOVE) $(TARGETS)

copy:
	$(COPY) $(TARGET_DIR)/$(TARGET).hex $(COPY_DIR)


$(TARGETS): | $(TARGET_DIR)



flash: $(TARGET_DIR)/$(TARGET).hex
	avrdude -c usbasp -p t25 -u -U flash:w:$(TARGET_DIR)/$(TARGET).hex

fuse: 
	avrdude -c usbasp -p t25 -u -U lfuse:w:0xee:m -U hfuse:w:0xdd:m -U efuse:w:0xff:m
	
# Create final output files (.hex, .eep) from ELF output file.
%.hex: %.elf
	$(OBJCOPY) -R .eeprom -R .fuse -R .lock -R .signature -O $(FORMAT) $< $@

%.bin: %.elf
	$(OBJCOPY) -R .eeprom -R .fuse -R .lock -R .signature -O binary $< $@

%.eep: %.elf
	-$(OBJCOPY) --no-change-warnings -j .eeprom --change-section-lma .eeprom=0 -O $(FORMAT) $< $@

# Create extended listing file from ELF output file.
%.lss: %.elf
	$(COPY) $@ $@.old || :
	$(OBJDUMP) -h -S -z $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
	$(NM) -n $< > $@

# Size of generated code.
size: $(TARGET_DIR)/$(TARGET).elf
	$(SIZE) $<


OBJS = $(addprefix $(OBJDIR)/, $(CSRC:.c=.o) )

$(OBJS): | $(OBJDIR)
$(TARGET_DIR)/$(TARGET).elf: $(OBJS) 
	$(CC) $(ALL_CFLAGS) $^ --output $@ $(LDFLAGS)

$(OBJDIR)/%.o : %.c
	$(CC) $(ALL_CFLAGS) -c $< -o $@

# Include the dependency files.
-include $(OBJS:.o=.o.d)



