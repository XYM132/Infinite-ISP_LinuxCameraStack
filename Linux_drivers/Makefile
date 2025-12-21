# System detection
UNAME_M := $(shell uname -m)
ARCH ?= arm64
PWD := $(shell pwd)

# Architecture-specific settings
ifeq ($(UNAME_M),x86_64)
    # Cross-compilation for ARM64 from x86
    KERNEL_DIR ?= linux_headers/usr/src/linux-headers-5.15.0-1046-xilinx-zynqmp
    CROSS_COMPILE ?= aarch64-linux-gnu-
else ifeq ($(UNAME_M),aarch64)
    # Native compilation on ARM64
    KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build
    CROSS_COMPILE ?= 
else
    $(error Unsupported architecture: $(UNAME_M))
endif

# Kernel modules
obj-m := xil-isp-lite.o
obj-m += xil-vip.o
obj-m += mipi-rx-to-video.o
obj-m += ar1335.o
obj-m += xlnx_imx219.o

# Device Tree files
DTS_FILE := infinite_isp.dtsi
DTBO_FILE := $(DTS_FILE:.dtsi=.dtbo)
DTC_FLAGS ?= -@ -O dtb

BIT_STREAM_FILE ?= design_1_wrapper_imx219.bin

# Default target
all: modules dtbo

# Build kernel modules
modules:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) \
	CONFIG_MODULE_SIG=y EXTRA_CFLAGS="-I$(PWD)" modules

# Compile device tree
dtbo: check_bitstream $(DTBO_FILE)

infinite_isp-overlay.dts: dts/$(DTS_FILE)
	sed 's|@BIT_STREAM_FILE@|$(BIT_STREAM_FILE)|g' $< > dts/$@

$(DTBO_FILE): infinite_isp-overlay.dts
	dtc $(DTC_FLAGS) -o dts/$@ dts/$<

# Clean everything
clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
	rm -f *.dtbo
	rm -f dts/infinite_isp-overlay.dts infinite_isp.dtbo

# Installation targets (ARM64 only)
install: check_arch check_bitstream install_bitstream install_modules install_dtbo

install_modules: modules
	insmod xil-isp-lite.ko
	insmod xil-vip.ko
	insmod mipi-rx-to-video.ko
	insmod ar1335.ko
	insmod xlnx_imx219.ko

install_dtbo: dtbo
	-rmdir /sys/kernel/config/device-tree/overlays/*
	mkdir /sys/kernel/config/device-tree/overlays/Infinite_ISP
	echo 0 > /sys/class/fpga_manager/fpga0/flags
	cat dts/$(DTBO_FILE) > /sys/kernel/config/device-tree/overlays/Infinite_ISP/dtbo

install_bitstream: check_bitstream bitstream/$(BIT_STREAM_FILE)
	install -C -m 644 bitstream/$(BIT_STREAM_FILE) /lib/firmware/

uninstall: check_arch
	rmmod xil-isp-lite xil-vip mipi-rx-to-video ar1335 || true
	rm -f /lib/firmware/$(DTBO_FILE)

# Check for root privileges
check_root:
ifeq ($(shell id -u),0)
    # We're root, continue
else
	$(error This target must be run as root. Use 'sudo make install' or 'sudo make uninstall')
endif

# Check architecture for install/uninstall
check_arch:
ifneq ($(UNAME_M),aarch64)
	$(error This target can only be run on aarch64 architecture)
endif

check_bitstream:
ifeq ($(wildcard bitstream/$(BIT_STREAM_FILE)),)
	$(error BIT_STREAM_FILE not found: bitstream/$(BIT_STREAM_FILE))
endif

.PHONY: all modules dtbo clean install install_modules install_dtbo install_bitstream uninstall check_arch check_root
