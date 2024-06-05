# Project toolchain variables for build system Makefile
#
# The main Makefile located in the root of this repo includes this file for
# access to the target systems toolchain.
#
# Toolchain and processor specific compiler flags are also defined in this file.
#
# To reduce duplication, compiler flags are defined with the following convention:
#
#   - TOOL_FLAG_COMMON contains compiler flags that are common to the C compiler,
#     C++ compiler, and assembler. This variable will not be used in the final
#     definition of the linker's flags. If the linker and compiler share flags
#     they must be duplicated
#
#   - TOOL_CFLAGS contains compiler flags specifically for the C compiler
#
#   - TOOL_CXXFLAGS contains compiler flags specifically for the C++ compiler
#
#   - TOOL_ASFLAGS contains flags specifically for the assembler
#
#   - TOOL_LDFLAGS contains flags specifically for the linker
#
# Inputs:
#   None
#
# Outputs:
#   CC
#   CXX
#   LD
#   AS
#   AR
#   OBJCOPY
#   OBJDUMP
#   SIZE
#   NM
#   CXXFILT
#   TOOL_FLAG_COMMON
#   TOOL_CFLAGS
#   TOOL_CXX_FLAGS
#   TOOL_ASFLAGS
#   TOOL_LDFLAGS
#

_TOOL_PREFIX := arm-none-eabi-
CC      := $(_TOOL_PREFIX)gcc.exe -c -xc
CXX     := $(_TOOL_PREFIX)gcc.exe -c -xc++
AS      := $(_TOOL_PREFIX)gcc.exe -c -xassembler-with-cpp
LD      := $(_TOOL_PREFIX)gcc.exe
AR      := $(_TOOL_PREFIX)ar.exe
OBJCOPY := $(_TOOL_PREFIX)objcopy.exe
OBJDUMP := $(_TOOL_PREFIX)objdump.exe
SIZE    := $(_TOOL_PREFIX)size.exe
NM      := $(_TOOL_PREFIX)nm.exe
CXXFILT := $(_TOOL_PREFIX)c++filt.exe
