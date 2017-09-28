/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "CameraCap"

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <poll.h>

#include "CameraIn.h"

namespace android {

// ---------------------------------------------------------------------------

    CameraIn::CameraIn() {
        ALOGI("%s\n",__FUNCTION__);
        mFd = -1;
        mStd = 0;
        mWidth = 0;
        mHeight = 0;
        memset(mCapBuf, 0, sizeof(struct CaptureBuf) * NO_OF_CAPTURE_BUF);
        memset(&mFormat, 0, sizeof(struct v4l2_format));
        mStreamOn = false;
    }

    CameraIn::~CameraIn() {
        ALOGI("%s\n",__FUNCTION__);
        if (mFd >= 0) {
            close(mFd);
            mFd = -1;
        }
    }

    int CameraIn::openDev(char *path) {
        ALOGI("%s:path=%s\n", __FUNCTION__, path);
        mFd = open(path, O_RDWR, 0);
        if (mFd < 0) {
            ALOGE("open dev %s failed\n", path);
            return -1;
        }

        struct v4l2_capability cap;
        int err;
        err = ioctl(mFd, VIDIOC_QUERYCAP, &cap);
        if (err < 0) {
            ALOGE("VIDIOC_QUERYCAP failed\n");
            return -1;
        }

        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
            ALOGE("V4L2_CAP_VIDEO_CAPTURE failed\n");
            return -1;
        }

        if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
            ALOGE("V4L2_CAP_STREAMING failed\n");
            return -1;
        }

        struct v4l2_dbg_chip_ident chip;
        err = ioctl(mFd, VIDIOC_DBG_G_CHIP_IDENT, &chip);
        if (err != 0) {
            ALOGE("VIDIOC_DBG_G_CHIP_IDENT failed\n");
            return -1;
        }

#if 0
            struct v4l2_fmtdesc vid_fmtdesc;
            int iindex = 0;
            while (err == 0)
            {
                vid_fmtdesc.index = iindex;
                vid_fmtdesc.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                err               = ioctl(mFd, VIDIOC_ENUM_FMT, &vid_fmtdesc);
                printf("index:%d,ret:%d, format:%c%c%c%c\r\n", iindex, err,
                             vid_fmtdesc.pixelformat & 0xFF,
                             (vid_fmtdesc.pixelformat >> 8) & 0xFF,
                             (vid_fmtdesc.pixelformat >> 16) & 0xFF,
                             (vid_fmtdesc.pixelformat >> 24) & 0xFF);
            }
#endif

        ALOGI("Camera chip %s\n", chip.match.name);
#if 0
        int g_input = 1;
        err = ioctl(mFd, VIDIOC_S_INPUT, &g_input);
        if (err < 0)
            return -1;
#endif

        int maxWait = 6;
        do {
            err = ioctl(mFd, VIDIOC_G_STD, &mStd);
            if (err < 0) {
                ALOGE("VIDIOC_G_STD failed %d\n", 6 - maxWait);
                sleep(1);
            }
            maxWait--;
        } while ((err != 0) || (maxWait <= 0));
#if 0
        if ((mStd != V4L2_STD_PAL) && (mStd != V4L2_STD_NTSC))
        {
            printf("err 5\r\n");
            return -1;
        }
#endif

        err = ioctl(mFd, VIDIOC_S_STD, &mStd);
        if (err < 0) {
            ALOGE("VIDIOC_S_STD failed\n");
            return -1;
        }

        int index = 0;
        char TmpStr[20];
        struct v4l2_frmsizeenum vid_frmsize;
        struct v4l2_frmivalenum vid_frmval;
        int max_area = 0;
        int max_width = 0, max_height = 0;
        int max_deno = 0, max_numer = 0;
        while (err == 0) {
            memset(TmpStr, 0, 20);
            memset(&vid_frmsize, 0, sizeof(struct v4l2_frmsizeenum));
            vid_frmsize.index = index++;
            vid_frmsize.pixel_format = CAPTURE_PIX_FORMAT;
            err = ioctl(mFd, VIDIOC_ENUM_FRAMESIZES, &vid_frmsize);
            if (err == 0) {
                ALOGI("enum frame size w:%d, h:%d\n", vid_frmsize.discrete.width, vid_frmsize.discrete.height);
                memset(&vid_frmval, 0, sizeof(struct v4l2_frmivalenum));
                vid_frmval.index = 0;
                vid_frmval.pixel_format = vid_frmsize.pixel_format;
                vid_frmval.width = vid_frmsize.discrete.width;
                vid_frmval.height = vid_frmsize.discrete.height;

                err = ioctl(mFd, VIDIOC_ENUM_FRAMEINTERVALS, &vid_frmval);
                if (err == 0) {
                    ALOGI("vid_frmval denominator:%d, numeraton:%d\r\n", vid_frmval.discrete.denominator,
                          vid_frmval.discrete.numerator);
                    if ((vid_frmval.discrete.denominator / vid_frmval.discrete.numerator) >= 15) {
                        int area = vid_frmsize.discrete.width * vid_frmsize.discrete.height;
                        if (area > max_area) {
                            max_area = area;
                            max_width = vid_frmsize.discrete.width;
                            max_height = vid_frmsize.discrete.height;
                            max_deno = vid_frmval.discrete.denominator;
                            max_numer = vid_frmval.discrete.numerator;
                        }
                    }
#if 0
                    if ((vid_frmsize.discrete.width > 1920) ||
                        (vid_frmsize.discrete.height > 1080)) {
                        vid_frmval.discrete.denominator = 15;
                        vid_frmval.discrete.numerator   = 1;
                    }
                    else {
                        vid_frmval.discrete.denominator = 30;
                        vid_frmval.discrete.numerator   = 1;
                    }
#endif
                }
            }
        } // end while
        if ((max_width == 0) || (max_height == 0) || (max_deno == 0) || (max_numer == 0)) {
            ALOGE("err width height deno and nmber\n");
            return -1;
        }

        ALOGI("We us %d x %d %f fps\n", max_width, max_height, (float) max_deno / max_numer);

        int input = 1;
        err = ioctl(mFd, VIDIOC_S_INPUT, &input);
        if (err < 0) {
            ALOGE("VIDIOC_S_INPUT filed\n");
            return -1;
        }

        struct v4l2_streamparm param;
        memset(&param, 0, sizeof(param));
        param.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        param.parm.capture.timeperframe.numerator = max_numer;
        param.parm.capture.timeperframe.denominator = max_deno;
        param.parm.capture.capturemode = 0;
        err = ioctl(mFd, VIDIOC_S_PARM, &param);
        if (err < 0) {
            printf("VIDIOC_S_PARM failed\r\n");
            return -1;
        }

        //struct v4l2_format fmt;
        memset(&mFormat, 0, sizeof(mFormat));
        mFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        mFormat.fmt.pix.width = max_width & 0xFFFFFFF8;
        mFormat.fmt.pix.height = max_height & 0xFFFFFFF8;
        mFormat.fmt.pix.pixelformat = CAPTURE_PIX_FORMAT;
        mFormat.fmt.pix.priv = 0;
        mFormat.fmt.pix.sizeimage = 0;
        mFormat.fmt.pix.bytesperline = 0;

        if (mFormat.fmt.pix.pixelformat == v4l2_fourcc('Y', 'V', '1', '2')) {
            int stride = (max_width + 31) / 32 * 32;
            int c_stride = (stride / 2 + 15) / 16 * 16;
            mFormat.fmt.pix.bytesperline = stride;
            mFormat.fmt.pix.sizeimage = stride * max_height + c_stride * max_height;
            ALOGI("Special handling for YV12 on Stride %d, size %d\n", mFormat.fmt.pix.bytesperline,
                  mFormat.fmt.pix.sizeimage);
        }

        err = ioctl(mFd, VIDIOC_S_FMT, &mFormat);
        if (err < 0) {
            ALOGE("VIDIOC_S_FMT failed\r\n");
            return -1;
        }

        mWidth = max_width;
        mHeight = max_height;
        return 0;
    }

    int CameraIn::setupBuffer() {
        ALOGI("%s\n",__FUNCTION__);
        struct v4l2_requestbuffers bufreq;
        bufreq.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        bufreq.memory = V4L2_MEMORY_MMAP;
        bufreq.count = NO_OF_CAPTURE_BUF;

        if (ioctl(mFd, VIDIOC_REQBUFS, &bufreq) < 0)
            return -1;

        for (int i = 0; i < NO_OF_CAPTURE_BUF; i++) {
            struct v4l2_buffer buf;
            memset(&buf, 0, sizeof(buf));
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;
            if (ioctl(mFd, VIDIOC_QUERYBUF, &buf) < 0)
                return -1;

            mCapBuf[i].length = buf.length;
            mCapBuf[i].offset = (size_t) buf.m.offset;
            mCapBuf[i].start = (unsigned char *) mmap(NULL, mCapBuf[i].length, PROT_READ | PROT_WRITE, MAP_SHARED, mFd,
                                                      mCapBuf[i].offset);
            //memset(mCapBuf[i].start, 0xFF, mCapBuf[i].length);
        }

        return 0;
    }

    int CameraIn::startCapture() {
        ALOGI("%s\n",__FUNCTION__);
        int i;
        struct v4l2_buffer buf;

        for (i = 0; i < NO_OF_CAPTURE_BUF; i++) {
            memset(&buf, 0, sizeof(buf));
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;
            buf.m.offset = mCapBuf[i].offset;
            if (ioctl(mFd, VIDIOC_QBUF, &buf) < 0)
                return -1;
        }

        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (ioctl(mFd, VIDIOC_STREAMON, &type) < 0)
            return -1;

        mStreamOn = true;

        return 0;
    }

    int CameraIn::stopCapture() {
        ALOGI("%s\n",__FUNCTION__);
        if (mStreamOn) {
            enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            int err = ioctl(mFd, VIDIOC_STREAMOFF, &type);
            if (err < 0)
                return -1;
            mStreamOn = false;
        }

        for (int i = 0; i < NO_OF_CAPTURE_BUF; i++)
            munmap(mCapBuf[i].start, mCapBuf[i].length);

        return 0;
    }

    unsigned char *CameraIn::pollFrame(int *index) {
        if (index == NULL)
            return NULL;

        struct pollfd fdListen;
        memset(&fdListen, 0, sizeof(fdListen));
        fdListen.fd = mFd;
        fdListen.events = POLLIN;

        int n = poll(&fdListen, 1, MAX_DEQUEUE_WAIT_TIME);

        if (n <= 0)
            return NULL;

        if (fdListen.revents & POLLIN) {
            struct v4l2_buffer cfilledbuffer;
            memset(&cfilledbuffer, 0, sizeof(cfilledbuffer));
            cfilledbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            cfilledbuffer.memory = V4L2_MEMORY_MMAP;

            int err = ioctl(mFd, VIDIOC_DQBUF, &cfilledbuffer);
            if (err < 0)
                return NULL;

            *index = cfilledbuffer.index;
            //printf("get frame %d 0x%x\r\n", cfilledbuffer.index, mCapBuf[cfilledbuffer.index].start);
            return mCapBuf[cfilledbuffer.index].start;
        }
        return NULL;
    }

    unsigned char *CameraIn::getFrame(int *index) {
        if (index == NULL)
            return NULL;

        struct v4l2_buffer cfilledbuffer;
        memset(&cfilledbuffer, 0, sizeof(cfilledbuffer));
        cfilledbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        cfilledbuffer.memory = V4L2_MEMORY_MMAP;

        int err = ioctl(mFd, VIDIOC_DQBUF, &cfilledbuffer);
        if (err < 0)
            return NULL;

        *index = cfilledbuffer.index;
        //printf("get frame %d 0x%x\r\n", cfilledbuffer.index, mCapBuf[cfilledbuffer.index].start);
        return mCapBuf[cfilledbuffer.index].start;
    }

    int CameraIn::releaseFrame(int index) {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = index;
        buf.m.offset = mCapBuf[index].offset;
        if (ioctl(mFd, VIDIOC_QBUF, &buf) < 0)
            return -1;
        return 0;
    }

};
