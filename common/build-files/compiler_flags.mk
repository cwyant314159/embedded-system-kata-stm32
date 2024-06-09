# Compiler flags
#
# The main Makefile located in the root of this repo includes this file for
# the compiler flags passed to the toolchain during builds.
#
# This file requires the LDSCRIPT (linker script), INC_FLAGS (space separated
# list of -I/include/path), BINARY_DIR (output binary folder), and BINARY_NAME
# (compiled binary name with no extension) variables defined.
#
# Inputs:
#   INC_FLAGS
#   BINARY_DIR
#   BINARY_NAME
#
# Outputs:
#   CFLAGS
#   CXXFLAGS
#   ASFLAGS
#   LDFLAGS
#

# flags common to compilation and linking #
_COMMON_FLAGS := -mcpu=cortex-m3
_COMMON_FLAGS += -mthumb
_COMMON_FLAGS += -mfloat-abi=soft
_COMMON_FLAGS += --specs=nosys.specs

# flags common to all compilation steps #
_COMPILE_FLAGS := $(_COMMON_FLAGS)
_COMPILE_FLAGS += -MMD
_COMPILE_FLAGS += -MP
_COMPILE_FLAGS += -ggdb3
_COMPILE_FLAGS += -fstack-usage
_COMPILE_FLAGS += -ffunction-sections
_COMPILE_FLAGS += -fdata-sections
_COMPILE_FLAGS += -fno-common
_COMPILE_FLAGS += -fsigned-char
_COMPILE_FLAGS += -O0

# warnings #
_COMPILE_FLAGS += -Wall
_COMPILE_FLAGS += -Wextra
_COMPILE_FLAGS += -Wundef
_COMPILE_FLAGS += -Wshadow
_COMPILE_FLAGS += -Wredundant-decls
_COMPILE_FLAGS += -Wswitch-enum
_COMPILE_FLAGS += -Wno-unused-parameter
_COMPILE_FLAGS += -pedantic

# defines #
_COMPILE_FLAGS += -DF_CPU_HZ=72000000
_COMPILE_FLAGS += -DF_HSE_HZ=8000000
_COMPILE_FLAGS += -DF_LSE_HZ=32768
_COMPILE_FLAGS += -DF_HSI_HZ=8000000
_COMPILE_FLAGS += -DF_LSI_HZ=40000
_COMPILE_FLAGS += -DSTM32F103xB

# header path includes flags #
_COMPILE_FLAGS += $(INC_FLAGS)

# C compiler flags #
CFLAGS := $(_COMPILE_FLAGS)
CFLAGS += -Wimplicit-function-declaration
CFLAGS += -Wmissing-prototypes
CFLAGS += -Wstrict-prototypes
CFLAGS += -std=c11

# C++ compiler flags #
CXXFLAGS := $(_COMPILE_FLAGS)
CXXFLAGS += -Weffc++
CXXFLAGS += -fno-exceptions
CXXFLAGS += -fno-unwind-tables
CXXFLAGS += -fno-rtti
CXXFLAGS += -fno-use-cxa-atexit
CXXFLAGS += -std=c++11

# assembler flags #
ASFLAGS := $(_COMPILE_FLAGS) 

# linker flags #
LDFLAGS := $(_COMMON_FLAGS)
LDFLAGS += -T$(LDSCRIPT)
LDFLAGS += -Wl,-Map="$(BINARY_DIR)/$(BINARY_NAME).map"
LDFLAGS += -Wl,-print-memory-usage
LDFLAGS += -static
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -nostdlib
LDFLAGS += -Wl,--no-warn-rwx-segments
LDFLAGS += -fno-exceptions
LDFLAGS += -fno-unwind-tables
LDFLAGS += -fno-rtti
