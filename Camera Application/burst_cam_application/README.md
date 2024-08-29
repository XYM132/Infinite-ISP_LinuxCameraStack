# BurstCam Application
## Overview
libcamera is an open-source camera stack that streamlines the development of embedded camera applications across various platforms. One of its standout features is cross-platform compatibility. 

Using libcamera, we’ve created a custom camera application capable of capturing a single image or multiple consecutive frames. By leveraging essential components from the existing [simple-cam](https://git.libcamera.org/libcamera/simple-cam.git) and [cam](https://git.libcamera.org/libcamera) utilities, we've designed this application to be as simple as possible. 

We've successfully tested our application on multiple platforms that utilize the underlying hardware ISP, including:

* Raspberry Pi (using Camera Modules 2 and 3)
* Linux based laptops (using integrated and external webcams)
* Kria KV260 Board (using ISP-Lite)

## How to Build
### Build for Raspberry Pi and Linux System
To build this application on a Raspberry Pi and on a Linux system, follow these commands:
* Install the libcamera [dependencies](https://libcamera.org/getting-started.html#dependencies)   
* Clone the repo using
    ```shell
        https://github.com/10xEngineersTech/CustomCameraApplication.git

    ```
*  Open terminal, navigate to the project directory, and run these commands one by one:
    ```shell
        meson setup build
        ninja -C build
        ./build/simple-cam
    ```

### Build for the Kria Board
For the Kria board, it uses ISP-Lite from the [zynqmp_cam_isp_demo_linux](https://github.com/bxinquan/zynqmp_cam_isp_demo_linux.git) project.To run this application on Kria board, we have written a [tutorial](https://10xengineers.ai/updates/) in which we explained how to resolve all the dependencies and run this project. Additionally, we have provided a customized [PetaLinux](../petalinux_kria/) build that sets up the environment for libcamera and installs all necessary dependencies to run the application.

## License
This project is licensed under Apache 2.0 (see [LICENSE](LICENSE.txt) file).


## Contact
For any inquiries or feedback, feel free to reach out.

Email: isp@10xengineers.ai

Website: http://www.10xengineers.ai

LinkedIn: https://www.linkedin.com/company/10x-engineers/
