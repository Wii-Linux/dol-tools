#
# DOL Tools - Top-level Makefile
# Copyright (C) 2025 - Techflash
#


dol-info_src := main.c
dol-info_obj := $(patsubst %.c,build/dol-info/%.o,$(dol-info_src))
dol-info_out := bin/dol-info

dol-run_src := main.c mem.c emu.c dummyData.c timer.c
dol-run_src +=
dol-run_src += cpu/common.c cpu/init.c cpu/mfspr.c cpu/mtspr.c
dol-run_src += cpu/mtmsr_mfmsr.c cpu/mtsr_mfsr.c cpu/rfi.c
dol-run_src +=
dol-run_src += mmio/forward.c mmio/chipset.c mmio/flipper.c
dol-run_src += mmio/pi.c mmio/mi.c mmio/vi.c mmio/dsp.c mmio/ai.c

dol-run_obj := $(patsubst %.c,build/dol-run/%.o,$(dol-run_src))
dol-run_out := bin/dol-run

CFLAGS := -Wall -Wextra -Wformat=2 -std=gnu89 -g

# Place our .text @ 128M, and hog lomem
# We need to mmap RAM @ 0 for MEM1, so that general region
# needs to be clear, otherwise our code vanishes beneath us.
# Problem.  This doesn't help any with PIC and/or PIE enabled.
# Just disable those, right?..... well no, then libc breaks.
# So, we need a non-PIC, non-PIE, .text-adjusted,
# statically-linked, executable.  Ick.
# We also can't quite hog lomem @ 0, GCC & Binutils try to put
# actually important crap there that we can't easily do anything about.
# -Wl,--section-start=.mem1c_hog=0x80000000 -Wl,--section-start=.mem1u_hog=0xc0000000
dol-run_LDFLAGS := -Wl,--section-start=.text=0x08000000 -Wl,--section-start=.lomem_hog=0x00010000 -Wl,--section-start=.init=0x20000000 -fno-pic -fno-pie -static

.PHONY: dol-info dol-run
all: dol-info dol-run

dol-info: $(dol-info_out)
dol-run: $(dol-run_out)

$(dol-info_out): $(dol-info_obj)
	@mkdir -p $(@D)
	@$(CC) $(dol-info_LDFLAGS) $(LDFLAGS) $^ -o $@
	$(info $s  LD    $@)

$(dol-run_out): $(dol-run_obj)
	@mkdir -p $(@D)
	@$(CC) $(dol-run_LDFLAGS) $(LDFLAGS) $^ -o $@
	$(info $s  LD    $@)

build/%.o: src/%.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $< -o $@
	$(info $s  CC    $<)

clean:
	rm -rf bin build
