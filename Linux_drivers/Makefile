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
DTS_FILE := pl.dtsi
DTBO_FILE := $(DTS_FILE:.dtsi=.dtbo)
DTC_FLAGS ?= -@ -O dtb

BIT_STREAM_FILE := design_1_wrapper.bit.bin

# Default target
all: modules dtbo

# Build kernel modules
modules:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) \
	CONFIG_MODULE_SIG=y EXTRA_CFLAGS="-I$(PWD)" modules

# Compile device tree
dtbo: $(DTBO_FILE)

$(DTBO_FILE): $(DTS_FILE)
	dtc $(DTC_FLAGS) -o $@ $<

# Clean everything
clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
	rm -f *.dtbo

# Installation targets (ARM64 only)
install: check_arch install_bitstream install_modules install_dtbo

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
	cat $(DTBO_FILE) > /sys/kernel/config/device-tree/overlays/Infinite_ISP/dtbo

install_bitstream: $(BIT_STREAM_FILE)
	install -C -m 644 $(BIT_STREAM_FILE) /lib/firmware/

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

.PHONY: all modules dtbo clean install install_modules install_dtbo install_bitstream uninstall check_arch check_root
