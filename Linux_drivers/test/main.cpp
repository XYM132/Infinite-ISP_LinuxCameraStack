#include <iostream>
#include <csignal>
#include <cerrno>
#include <atomic>
#include <thread>
#include <chrono>
#include <opencv2/opencv.hpp>
#include "V4L2Subdev.hpp"
#include "V4L2Stream.hpp"

static std::atomic<bool> g_running{true};
static std::atomic<uint64_t> g_sensor_frames{0};
static std::atomic<uint64_t> g_ispin_frames{0};
static std::atomic<uint64_t> g_ispout_frames{0};
static void sigint_handler(int){ g_running = false; }

int setSubDevFmt() {
    std::cout << "=== V4L2 Subdev Format Setter ===" << std::endl;

    // -------------------------
    // 1. Sensor (imx219)
    // -------------------------
    V4L2Subdev sensor("/dev/v4l-subdev5");
    if (!sensor.isValid()) return -1;

    std::cout << "Sensor name: " << sensor.getName() << std::endl;

    sensor.setFormat(
        0,                              // pad
        MEDIA_BUS_FMT_SRGGB10_1X10,     // raw Bayer 10bit
        1992, 1152,                     // resolution
        V4L2_FIELD_NONE
    );
    std::cout << "Sensor format set OK." << std::endl;

    // -------------------------
    // 2. MIPI RX
    // -------------------------
    V4L2Subdev mipi("/dev/v4l-subdev4");
    if (!mipi.isValid()) return -1;

    std::cout << "MIPI name: " << mipi.getName() << std::endl;

    // pad0 (sink)
    mipi.setFormat(0, MEDIA_BUS_FMT_SRGGB10_1X10, 1992, 1152);
    // pad1 (source)
    mipi.setFormat(1, MEDIA_BUS_FMT_SRGGB10_1X10, 1992, 1152);
    std::cout << "MIPI formats set OK." << std::endl;

    // -------------------------
    // 3. Subset Converter (ISP block)
    // -------------------------
    V4L2Subdev conv("/dev/v4l-subdev3");
    if (!conv.isValid()) return -1;

    std::cout << "SubsetConv name: " << conv.getName() << std::endl;

    // pad0: input RAW
    conv.setFormat(0, MEDIA_BUS_FMT_SRGGB10_1X10, 1992, 1152);
    // pad1: output Y10
    conv.setFormat(1, MEDIA_BUS_FMT_Y10_1X10,     1992, 1152);
    std::cout << "SubsetConv formats set OK." << std::endl;

    V4L2Subdev infiniteISP("/dev/v4l-subdev0");   // a0060000.infinite_isp
    V4L2Subdev vip4000("/dev/v4l-subdev1");       // a0064000.xil_vip
    V4L2Subdev vip6000("/dev/v4l-subdev2");       // a0066000.xil_vip

    if (!infiniteISP.isValid() || !vip4000.isValid() || !vip6000.isValid()) return -1;

    printf("Infinite-ISP name : %s\n", infiniteISP.getName().c_str());
    printf("VIP 4000 name     : %s\n", vip4000.getName().c_str());
    printf("VIP 6000 name     : %s\n", vip6000.getName().c_str());

    infiniteISP.setFormat(0, MEDIA_BUS_FMT_Y10_1X10, 1992, 1152);
    vip4000.setFormat(1, MEDIA_BUS_FMT_UYVY8_1X16, 1920, 1080);

    // -------------------------
    // Read back formats
    // -------------------------
    v4l2_mbus_framefmt fmt {};

    std::cout << "\n=== Read back formats ===\n";

    if (sensor.getFormat(0, fmt)) {
        std::cout << "Sensor pad0: " << fmt.width << "x" << fmt.height
                  << " code=0x" << std::hex << fmt.code << std::dec << std::endl;
    }

    if (mipi.getFormat(1, fmt)) {
        std::cout << "MIPI pad1: " << fmt.width << "x" << fmt.height
                  << " code=0x" << std::hex << fmt.code << std::dec << std::endl;
    }

    if (conv.getFormat(1, fmt)) {
        std::cout << "SubsetConv pad1: " << fmt.width << "x" << fmt.height
                  << " code=0x" << std::hex << fmt.code << std::dec << std::endl;
    }

    if (infiniteISP.getFormat(0, fmt)) {
        std::cout << "Infinite-ISP pad0: " << fmt.width << "x" << fmt.height
                  << " code=0x" << std::hex << fmt.code << std::dec << std::endl;
    }

    if (vip4000.getFormat(1, fmt)) {
        std::cout << "VIP4000 pad1: " << fmt.width << "x" << fmt.height
                  << " code=0x" << std::hex << fmt.code << std::dec << std::endl;
    }

    std::cout << "=== Done ===" << std::endl;
    return 0;
}

int ispPipelineRun() {
    V4L2Stream cap0("/dev/video0", false, 6);
    V4L2Stream out3("/dev/video3", true, 6);
    V4L2Stream cap2("/dev/video2", false, 6);
    // GstKmsViewer viewer("/dev/video2", 1920, 1080, "YUY2");

    cap0.openDevice();
    out3.openDevice();
    cap2.openDevice();

    cap0.setFormat(1992,1152,v4l2_fourcc('X','Y','1','0'));
    out3.setFormat(1992,1152,v4l2_fourcc('X','Y','1','0'));
    cap2.setFormat(1920,1080,V4L2_PIX_FMT_YUYV);

    cap0.initMMap();
    cap2.initMMap();

    cap0.exportAllDMABuf();
    out3.initDMABufImport(cap0.buffers.size());

    cap0.queueAllCapture();
    cap2.queueAllCapture();

    cap0.startStreaming();
    cap2.startStreaming();
    // viewer.start();

    IndexQueue ready_q(8);

    /* cap0 DQ thread */
    std::thread t0([&](){
        while (g_running) {
            int idx = cap0.dequeue(500);
            if (idx >= 0)
            {
                g_sensor_frames.fetch_add(1, std::memory_order_relaxed);
                ready_q.push(idx);
            }
        }
    });

    /* out3 worker */
    std::thread t1([&](){
        bool started = false;
        while (g_running) {
            int idx;
            if (!ready_q.pop(idx))
                continue;

            if (!started) {
                out3.startStreaming();
                started = true;
            }

            out3.queueDMABuf(
                idx,
                cap0.buffers[idx].fd,
                cap0.bufferLen(idx)
            );
            
            g_ispin_frames.fetch_add(1, std::memory_order_relaxed);
            int done = out3.dequeue(1000);
            if (done >= 0)
            {
                cap0.queueCapture(done);
            }
        }
    });

    /* cap2 display */
    std::thread t2([&](){
        cv::namedWindow("ISP", cv::WINDOW_NORMAL);
        while (g_running) {
            int idx = cap2.dequeue(500);
            if (idx >= 0) {
                g_ispout_frames.fetch_add(1, std::memory_order_relaxed);
                cv::Mat yuyv(1080, 1920, CV_8UC2,cap2.bufferPtr(idx));
                cv::Mat bgr;
                cv::cvtColor(yuyv,bgr,cv::COLOR_YUV2BGR_YUYV);
                cv::imshow("ISP",bgr);
                cv::waitKey(1);
                cap2.queueCapture(idx);
            }
        }
    });

    /* FPS monitor thread */
    std::thread monitor([&](){
        while (g_running) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            uint64_t s = g_sensor_frames.exchange(0, std::memory_order_relaxed);
            uint64_t in = g_ispin_frames.exchange(0, std::memory_order_relaxed);
            uint64_t out = g_ispout_frames.exchange(0, std::memory_order_relaxed);
            printf("FPS -> sensor: %llu, ispin: %llu, ispout: %llu\n",
                   (unsigned long long)s, (unsigned long long)in, (unsigned long long)out);
        }
    });

    while (g_running)
        std::this_thread::sleep_for(std::chrono::seconds(1));

    t0.join();
    t1.join();
    t2.join();
    monitor.join();
    // viewer.stop();
    ready_q.stop();
    return 0;
}

int main(){
    signal(SIGINT, sigint_handler);

    setSubDevFmt();
    ispPipelineRun();

    std::cout << "Exited\n";
    return 0;
}