#!/bin/bash

# Assemble the MIPS code (big-endian, using 64-bit toolchain)
mips64-linux-gnu-as -32 -march=mips32 -EB -o example.o example.s

# Link using custom linker script to create ELF (big-endian)
mips64-linux-gnu-ld -melf32btsmip -T bare.ld -o example.elf example.o

# Extract pure binary (no headers)
mips64-linux-gnu-objcopy -O binary -j .text example.elf example.bin

# Verify the output
echo "Generated files:"
ls -la example.*

echo -e "\nBinary size:"
wc -c example.bin

echo -e "\nHex dump of pure binary (big-endian):"
hexdump -C example.bin

echo -e "\nDisassembly verification (big-endian):"
mips64-linux-gnu-objdump -D -b binary -m mips -EB example.bin
