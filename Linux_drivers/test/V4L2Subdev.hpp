#pragma once
#include <linux/v4l2-subdev.h>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cstring>
#include <string>
#include <iostream>

class V4L2Subdev {
public:
    explicit V4L2Subdev(const std::string& dev)
        : devPath(dev), fd(-1)
    {
        fd = open(dev.c_str(), O_RDWR);
        if (fd < 0) {
            perror(("Failed to open " + dev).c_str());
        }
    }

    ~V4L2Subdev() {
        if (fd >= 0) close(fd);
    }

    bool isValid() const { return fd >= 0; }

    // ---------------------------------------------------
    // Get Subdev Name
    // ---------------------------------------------------
    std::string getName() {
        if (fd < 0) return "";

        // Fallback: read from sysfs
        std::string sysfs = "/sys/class/video4linux/" + devPath.substr(devPath.find_last_of('/')+1) + "/name";

        FILE* f = fopen(sysfs.c_str(), "r");
        if (!f) return "(unknown)";

        char buf[256] = {0};
        fgets(buf, sizeof(buf), f);
        fclose(f);

        // Remove newline
        std::string name(buf);
        name.erase(std::remove(name.begin(), name.end(), '\n'), name.end());
        return name;
    }

    // ---------------------------------------------------
    // Get format on a pad
    // ---------------------------------------------------
    bool getFormat(uint32_t pad, v4l2_mbus_framefmt& fmt) {
        if (fd < 0) return false;

        v4l2_subdev_format fmtReq {};
        memset(&fmtReq, 0, sizeof(fmtReq));
        fmtReq.pad = pad;
        fmtReq.which = V4L2_SUBDEV_FORMAT_ACTIVE;

        if (ioctl(fd, VIDIOC_SUBDEV_G_FMT, &fmtReq) < 0) {
            perror("VIDIOC_SUBDEV_G_FMT");
            return false;
        }

        fmt = fmtReq.format;
        return true;
    }

    // ---------------------------------------------------
    // Set format on a pad
    // ---------------------------------------------------
    bool setFormat(uint32_t pad,
                   uint32_t mbus_code,
                   uint32_t width,
                   uint32_t height,
                   uint32_t field = V4L2_FIELD_NONE)
    {
        if (fd < 0) return false;

        v4l2_subdev_format fmtReq {};
        memset(&fmtReq, 0, sizeof(fmtReq));

        fmtReq.which = V4L2_SUBDEV_FORMAT_ACTIVE;
        fmtReq.pad   = pad;

        fmtReq.format.width  = width;
        fmtReq.format.height = height;
        fmtReq.format.code   = mbus_code;
        fmtReq.format.field  = field;

        if (ioctl(fd, VIDIOC_SUBDEV_S_FMT, &fmtReq) < 0) {
            perror("VIDIOC_SUBDEV_S_FMT");
            return false;
        }

        return true;
    }

private:
    std::string devPath;
    int fd;
};
