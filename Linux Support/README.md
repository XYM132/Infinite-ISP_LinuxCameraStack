# Infinite-ISP Support for PetaLinux

## Overview
 This project is part of the [Infinite-ISP Linux-Based](../README.md) Camera Stack, which aims to provide a complete camera stack package, from camera application development to hardware ISP implementation on FPGA. Application development and FPGA implementation are complete, while writing the driver and integrating Infinite-ISP into Linux as a module are still in progress.

**Note:** This guide is for developers who are interested in contributing to adding PetaLinux support to this project.

## Tasks for Contribution
There are two main tasks that need contributions.

1. Integrate the Infinite-ISP into the Linux operating system as a module.
2. Develop the driver for Infinite-ISP to enable seamless communication between the hardware and software layers. 

### Reference Project

There a present named [zynqmp_cam_isp_demo_linux](https://github.com/bxinquan/zynqmp_cam_isp_demo_linux.git) which we are using as a reference. This project includes:

- libcamera framework support
- A hardware ISP named ISP_Lite
- Driver for ISP_LIte
- AR1335 camera support

We are using this reference project for development. Currently, we have developed a custom camera appplication using the licamera framework, which runs with ISP_Lite. In the FPGA Implementation of this project, we have used Infinite-ISP instead of ISP_Lite. 


##  Contributing to Adding Infinite-ISP to PetaLinux
To integrate Infinite-ISP into the PetaLinux device tree, you need to generate an FPGA device tree overlay (.dtbo) and a .bin file.

### Resources
We have data present in [resources](/Linux%20Support/resources/) directory:
- [design_1_wrapper.xsa](/Linux%20Support/resources/design_1_wrapper.xsa)
- [design_1_wrapper.bit](/Linux%20Support/resources/design_1_wrapper.bit)
- [design_1_wrapper.bit.bin](/Linux%20Support/resources/design_1_wrapper.bit.bin)
- [bit2bin.py](/Linux%20Support/resources/bit2bin.py)
- [pl.dtsi](/Linux%20Support/resources/pl.dtsi)

### Bin File Generation
The .bin file is created from the bitstream. We have already generated the design_1_wrapper.bit.bin using a script, which can be used for development.

### Device-tree Modification
To update the device tree in PetaLinux, an overlay for the FPGA (pl.dtsi) must be generated and converted into a .dtbo file.

There are multiple ways to generate the pl.dtsi file. In our case, we have generated it from the .xsa file using the Device Tree Generator ([DTG](https://xilinx.github.io/kria-apps-docs/creating_applications/2022.1/build/html/docs/dtsi_dtbo_generation.html#using-xsct-dtg-and-dtc)).

***TODO:*** The next step is to modify pl.dtsi to include all the peripherals specified in the hardware. The [pl.dtsi](https://github.com/bxinquan/zynqmp_cam_isp_demo_linux/blob/main/linux-xlnx-xilinx-v2022.1/arch/arm64/boot/dts/xilinx/zynqmp-cam-isp-demo-pl.dtsi) from the reference project can be used as a guide for comparison.  

### How to Test
- Clone the reference project. 
    ```shell
    git clone https://github.com/bxinquan/zynqmp_cam_isp_demo_linux.git
    ```
- Execute its [README](https://github.com/bxinquan/zynqmp_cam_isp_demo_linux/blob/main/README.md) instructions and ensure you run it successfully.
- Replace the generated pl.dtbo and design_1_wrapper.bit.bin files with the ones located in /mnt/nfs_share/lib/firmware.
- Apply the overlay using the following commands (Note: You may need to reboot first)
     ```shell
    mkdir -p /configfs
    mount -t configfs configfs /configfs
    echo 0 > /sys/class/fpga_manager/fpga0/flags
    mkdir /configfs/device-tree/overlays/full
    echo -n "pl.dtbo" > /configfs/device-tree/overlays/full/path
    ```
- Run the command to check for new modules listed in the device tree.
    ```shell
    cat /proc/interrupts
    ```
If the following modules appear as result of last command shown in the figure below, then the pl.dtbo has been generated correctly.

![](/Linux%20Support/resources/PL-Modules.png)

## License 
This project is licensed under Apache 2.0 (see [LICENSE](LICENSE) file).

## Contact
For any inquiries or feedback, feel free to reach out.

Email: isp@10xengineers.ai

Website: http://www.10xengineers.ai

LinkedIn: https://www.linkedin.com/company/10x-engineers/