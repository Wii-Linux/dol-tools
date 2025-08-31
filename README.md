# DOL Tools
Tools for interacting with the "DOL" format used in titles for the Nintendo® GameCube® and Nintendo® Wii®, primarily for use under Linux®.  

## dol-info
Small tool written in relatively portable ANSI C89, to dump all of the relevant info about a DOL's header.

## dol-run
Tool written in relatively portable ANSI C89, to run a DOL on the host by means of userspace-implemented Problem-State virtualization.  
It is expected to be run on a GameCube® or Wii® under Linux®.  
Only very minimal PowerPC CPU emulation is performed, in order to handle supervisor-mode instructions that fail under userspace Linux®.  
This will **NOT** run at all under any other machine.  Even other PowerPC systems are not supported.  That includes the Wii U® as well, as the "GX" graphics core is not active whilst Linux® is running.  It also includes any non-Nintendo® hardware (such as from Apple®), as they do not offer the "Paired Singles" functionality that the Gekko and Broadway processors have.  
It passes through hardware ranges for some Flipper/Hollywood hardware blocks to the real underlying hardware, using /dev/mem.  That means that if your kernel is configured to prevent such access.... you're going to have a very bad time.  
It currently stubs out just enough to get basic devkitPPC/libogc applications stumbling into `main()`.  Most real functionality past that point (notably graphics) are unimplemented.  Many things can't "just" be passed through, because the hardware expects to take physical memory addresses, which would trample host memory when the guest sends *it's* view of the physical address space to the device.

## Copyright / Legal disclaimers
"Nintendo®" is a registered trademark of Nintendo of America Inc.
"GameCube®" is a registered trademark of Nintendo of America Inc.
"Wii®" is a registered trademark of Nintendo of America Inc.
"Wii U®" is a registered trademark of Nintendo of America Inc.
"Apple®" is a registered trademark of Apple Inc.
"Linux®" is the registered trademark of Linus Torvalds in the U.S. and other countries.

Code of DOL Tools is Copyright (C) 2025 Techflash and DOL Tools contributors.  Licensed under the GNU GPL v2
