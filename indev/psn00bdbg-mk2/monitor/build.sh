#!/bin/bash

armips -temp monitor.lst monitor.asm

cat patchinst.bin >patch.bin
cat patchcode.bin >>patch.bin
