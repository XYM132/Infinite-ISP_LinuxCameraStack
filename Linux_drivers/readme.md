# Build and Deployment Guide

## Environment Setup

To build this project, you need to prepare the development environment for your target platform. This guide covers both native compilation on **KV260** boards and **cross-compilation** on x86-based PCs, all using **Ubuntu 22.04**.
For Windows users, **WSL2** is also supported.

------

### Native Compilation on KV260

Install the required build tools directly on the KV260 board:

```
sudo apt update
sudo apt install build-essential linux-headers-$(uname -r)
```

------

###  Cross-Compilation on x86 (for KV260 target)

#### 1. Install Dependencies

```
sudo apt update
sudo apt install gcc-11-aarch64-linux-gnu g++-11-aarch64-linux-gnu \
    build-essential device-tree-compiler
```

Configure `gcc` alternatives (optional but recommended):

```
sudo update-alternatives --install /usr/bin/aarch64-linux-gnu-gcc aarch64-linux-gnu-gcc /usr/bin/aarch64-linux-gnu-gcc-11 110
sudo update-alternatives --install /usr/bin/aarch64-linux-gnu-g++ aarch64-linux-gnu-g++ /usr/bin/aarch64-linux-gnu-g++-11 110
```

#### 2. Extract Linux Headers

Since x86 hosts cannot install ARM `.deb` packages directly, you need to extract them manually:

```
dpkg-deb -x ./debs/linux-headers-5.15.0-1046-xilinx-zynqmp_5.15.0-1046.50_arm64.deb linux_headers
dpkg-deb -x ./debs/linux-xilinx-zynqmp-headers-5.15.0-1046_5.15.0-1046.50_all.deb linux_headers
```

> ⚠️ These `.deb` packages are specific to kernel version `5.15.0-1046-xilinx-zynqmp`.
>  To confirm your board’s kernel version, run `uname -r` on the KV260.

If you're using a different kernel version, refer to [How to Obtain Kernel Headers](#How to Obtain Kernel Headers) below.

------



## Building and Installing Kernel Modules & Device Tree Overlays

To compile and install kernel modules (`.ko`) and device tree overlays (`.dtbo`):

```
make -j8
sudo make install
```

> ⚠️ On KV260, running `make install` as root on the **aarch64 target** will **remove all overlays** under `/sys/kernel/config/device-tree/overlays/`, effectively **unloading any preloaded bitstream** (typically loaded via DTBO). 
>
> Please ensure this behavior is acceptable in your workflow before proceeding.

------



## *How to Obtain Kernel Headers*

There are two main ways to get the kernel headers:

### Method 1: Download on KV260

```
apt download linux-headers-$(uname -r)
apt download linux-xilinx-zynqmp-headers-$(uname -r | sed 's/-xilinx-zynqmp//')
```

### Method 2: Build from Source on PC

You can also build the kernel headers manually by compiling the Xilinx kernel source. Follow these steps:

#### Step 1: Install Required Packages

```
sudo apt install gcc-11-aarch64-linux-gnu g++-11-aarch64-linux-gnu \
    build-essential libncurses-dev flex bison libssl-dev bc \
    kmod cpio pahole dwarfdump device-tree-compiler
```

#### Step 2: Clone the Xilinx Kernel Source

```
git clone https://git.launchpad.net/~canonical-kernel/ubuntu/+source/linux-xilinx-zynqmp/+git/jammy
cd jammy
git tag
git checkout <matching-tag>  # Example: Ubuntu-xilinx-zynqmp-5.15.0-1046.50
```

> Use the tag that matches your board's kernel version (`uname -r` on KV260).

#### Step 3: Build the Kernel

```
export ARCH=arm64
export $(dpkg-architecture -aarm64)
export CROSS_COMPILE=aarch64-linux-gnu-

fakeroot debian/rules clean
do_tools=false fakeroot debian/rules binary
```