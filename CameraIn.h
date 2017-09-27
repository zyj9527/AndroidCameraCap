#ifndef ANDROID_CAMERAIN_H
#define ANDROID_CAMERAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <asm/types.h>
#include <linux/videodev2.h>
#include <sys/mman.h>
#include <math.h>
#include <string.h>
#include <malloc.h>
#include <sys/time.h>

//#include <linux/mxcfb.h>
#include <linux/mxc_v4l2.h>
//#include <linux/ipu.h>

#include <cutils/properties.h>
#include <utils/Log.h>

//#define FRAME_RATE		30
#define NO_OF_CAPTURE_BUF		4
#define MAX_DEQUEUE_WAIT_TIME	1000

#define CAPTURE_PIX_FORMAT		v4l2_fourcc('Y', 'V', '1', '2')

namespace android {

struct CaptureBuf
{
	unsigned char *start;
	size_t offset;
	unsigned int length;
};

class CameraIn
{
public:
                CameraIn();
    virtual     ~CameraIn();
	int openDev(char *path);
	int setupBuffer();
	int startCapture();
	int stopCapture();
	unsigned char *pollFrame(int *index);
	unsigned char *getFrame(int *index);
	int releaseFrame(int index);
	
	int getWidth() { return mWidth; }
	int getHeight() { return mHeight; }
	
	int getStride() { return mFormat.fmt.pix.bytesperline; }
	int getImageSize() { return mFormat.fmt.pix.sizeimage; }
	int getFd() { return mFd; }
	
private:
	int mFd;
	int mWidth;
	int mHeight;
	bool mStreamOn;
	
	v4l2_std_id mStd;
	struct v4l2_format mFormat;
	struct CaptureBuf mCapBuf[NO_OF_CAPTURE_BUF];
};

}; // namespace android

#endif
