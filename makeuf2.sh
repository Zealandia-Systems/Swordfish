#!/bin/bash

for CONFIG in "Release"; do
	for MACHINE in "P2" "P3" "P4"; do
		./uf2conv.py -c -b 0x4000 -o "Swordfish-${CONFIG}-${MACHINE}.uf2" "${CONFIG} ${MACHINE}/Marlin.bin"
	done
done
