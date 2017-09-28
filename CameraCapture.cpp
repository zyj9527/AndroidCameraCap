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

#include <stdint.h>
#include <sys/types.h>
#include <fcntl.h>
#include <poll.h>
#include <cutils/properties.h>

#include <androidfw/AssetManager.h>
#include <binder/IPCThreadState.h>

#include <ui/PixelFormat.h>
#include <ui/Region.h>
#include <ui/DisplayInfo.h>
#include <ui/FramebufferNativeWindow.h>

#include <gui/ISurfaceComposer.h>
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>

#include <GLES/gl.h>

#include "EGLUtils.h"
#include "CameraCapture.h"


#define EXIT_PROP_NAME "service.bootanim.exit"

#define CAMERA_TEST 1
const char CamVertexShader[] =
        "uniform mat4 model;\n"
                "uniform mat4 projection;\n"
                "attribute vec4 vertex;\n"
                "varying vec2 TexCoords;\n"
                "void main() {\n"
                "  TexCoords = vertex.zw;\n"
                "  gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);\n"
                "}\n";

const char CamFragmentShader[] =
        "#extension GL_OES_EGL_image_external : require\n"
                "precision mediump float;\n"
                "varying vec2 TexCoords;\n"
                "uniform samplerExternalOES image;\n"
                "void main() {\n"
                "  gl_FragColor = texture2D(image, TexCoords);\n"
                "  gl_FragColor.a = 1.0;\n"
                "}\n";

namespace android {

// ---------------------------------------------------------------------------

    CameraCapture::CameraCapture() : Thread(false) {
        ALOGI("%s\n", __FUNCTION__);
        mSession = new SurfaceComposerClient();
#if !CAMERA_TEST
        mCam = new CameraIn();
#else
        mCam = NULL;
#endif
        Renderer = NULL;
    }

    CameraCapture::~CameraCapture() {
        ALOGI("%s\n", __FUNCTION__);
        ResourceManager::Clear();

        if (mCam) {
            delete mCam;
            mCam = NULL;
        }
        if (Renderer) {
            delete Renderer;
            Renderer = NULL;
        }
    }

    void CameraCapture::onFirstRef() {
        ALOGI("%s\n", __FUNCTION__);
        status_t err = mSession->linkToComposerDeath(this);
        if (err == NO_ERROR) {
            run("CameraCapture", PRIORITY_DISPLAY);
        }
    }

    sp<SurfaceComposerClient> CameraCapture::session() const {
        return mSession;
    }

    void CameraCapture::binderDied(const wp<IBinder> &who) {
        ALOGE("%s\n", __FUNCTION__);
        kill(getpid(), SIGKILL);
        requestExit();
    }

    status_t CameraCapture::readyToRun() {
        ALOGI("%s\n", __FUNCTION__);
        mAssets.addDefaultAssets();

        sp<IBinder> dtoken(SurfaceComposerClient::getBuiltInDisplay(ISurfaceComposer::eDisplayIdMain));
        DisplayInfo dinfo;
        status_t status = SurfaceComposerClient::getDisplayInfo(dtoken, &dinfo);
        if (status)
            return -1;

        sp<SurfaceControl> control = session()->createSurface(String8("CameraCapture"), dinfo.w, dinfo.h,
                                                              PIXEL_FORMAT_RGB_565);

        SurfaceComposerClient::openGlobalTransaction();
        control->setLayer(0x40000000);
        SurfaceComposerClient::closeGlobalTransaction();

        sp<Surface> s = control->getSurface();

        EGLint context_attribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
        EGLint s_configAttribs[] = {
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                EGL_NONE
        };

        EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (display == EGL_NO_DISPLAY)
            return -1;

        EGLint majorVersion;
        EGLint minorVersion;
        EGLBoolean ret;
        ret = eglInitialize(display, &majorVersion, &minorVersion);
        if (ret != EGL_TRUE)
            return -1;

        EGLNativeWindowType window = android_createDisplaySurface();
        EGLConfig myConfig = {0};
        ret = EGLUtils::selectConfigForNativeWindow(display, s_configAttribs, window, &myConfig);
        if (ret)
            return -1;

        EGLSurface surface = eglCreateWindowSurface(display, myConfig, s.get(), NULL);
        if (surface == EGL_NO_SURFACE)
            return -1;

        EGLContext context = eglCreateContext(display, myConfig, EGL_NO_CONTEXT, context_attribs);
        if (context == EGL_NO_CONTEXT)
            return -1;

        ret = eglMakeCurrent(display, surface, surface, context);
        if (ret != EGL_TRUE)
            return -1;

        EGLint w, h;
        eglQuerySurface(display, surface, EGL_WIDTH, &w);
        eglQuerySurface(display, surface, EGL_HEIGHT, &h);

        mDisplay = display;
        mContext = context;
        mSurface = surface;
        mWidth = w;
        mHeight = h;
        mFlingerSurfaceControl = control;
        mFlingerSurface = s;
#if !CAMERA_TEST
        if (v4l2Init() != 0) {
            return -1;
        }
#else
        if (testInit() != 0) {
            return -1;
        }
#endif
        if (setupEGL() != 0)
            return -1;

        return NO_ERROR;
    }

    int CameraCapture::setupEGL() {
        //Create Orthographic Projection
        glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(mWidth), static_cast<GLfloat>(mHeight), 0.0f,
                                          -1.0f, 1.0f);
        //Create ResourceManager with Shader and set parameters
        ResourceManager::LoadShader(CamVertexShader, CamFragmentShader, "camera");
        ResourceManager::GetShader("camera").Use().SetInteger("image", 0);
        ResourceManager::GetShader("camera").SetMatrix4("projection", projection);
        //Create a Texture with name 'camera'
        ResourceManager::LoadTextureExternal("camera");
        Renderer = new CamRenderer(ResourceManager::GetShader("camera"));

        //Create Android Surface with  'camera' Texture to drawpicture
        mBQ = new BufferQueue();
        mST = new GLConsumer(mBQ, ResourceManager::GetTexture("camera").ID);
        mSTC = new Surface(mBQ);
        mANW = mSTC;

        return 0;
    }

    bool CameraCapture::threadLoop() {
        ALOGI("%s\n", __FUNCTION__);
#if !CAMERA_TEST
        int r = v4l2Loop();
#else
        int r = testLoop();
#endif
        // No need to force exit anymore
        property_set(EXIT_PROP_NAME, "0");

        eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(mDisplay, mContext);
        eglDestroySurface(mDisplay, mSurface);
        mFlingerSurface.clear();
        mFlingerSurfaceControl.clear();
        eglTerminate(mDisplay);
        IPCThreadState::self()->stopProcess();
        return r;
    }

    void CameraCapture::checkExit() {
        char value[PROPERTY_VALUE_MAX];
        property_get(EXIT_PROP_NAME, value, "0");
        int exitnow = atoi(value);
        if (exitnow) {
            ALOGI("%s:%d\n", EXIT_PROP_NAME, exitnow);
            requestExit();
        }
    }

    int CameraCapture::v4l2Init() {
        ALOGI("%s\n", __FUNCTION__);
        if (mCam == NULL)
            return -1;
        if (mCam->openDev("/dev/video0") != 0)
            return -1;

        if (mCam->setupBuffer() != 0)
            return -1;
        return 0;
    }

    bool CameraCapture::v4l2Loop() {
        ALOGI("%s\n", __FUNCTION__);
        if ((mCam == NULL) || (mCam->startCapture() != 0))
            return false;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        int index = 0, err = 0;
        mCamWidth = mCam->getWidth();
        mCamHeight = mCam->getHeight();

#if 0
        struct timeval t1, t2;
#endif
        struct pollfd fdListen[1];

        while (1) {
            memset(fdListen, 0, sizeof(fdListen));
            fdListen[0].fd = mCam->getFd();
            fdListen[0].events = POLLIN;

            int n = poll(fdListen, 1, 1000);

            if (n <= 0)
                continue;

            for (int i = 0; i < 1 && n > 0; i++) {
                if (fdListen[i].revents & POLLIN) {
                    n--;
                    if (fdListen[i].fd == mCam->getFd()) {
#if 0
                        gettimeofday(&t1, NULL);
#endif
                        unsigned char *frame = mCam->getFrame(&index);
                        if (frame == NULL)
                            continue;
                        native_window_set_buffers_geometry(mANW.get(), mCamWidth, mCamHeight, HAL_PIXEL_FORMAT_YV12);
                        native_window_set_usage(mANW.get(), GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN);

                        ANativeWindowBuffer *anb = NULL;
                        err = native_window_dequeue_buffer_and_wait(mANW.get(), &anb);
                        if ((err != NO_ERROR) || (anb == NULL)) {
                            mCam->releaseFrame(index);
                            continue;
                        }
                        sp<GraphicBuffer> buf(new GraphicBuffer(anb, false));
                        uint8_t *img = NULL;
                        buf->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void **) (&img));
                        if (img == NULL) {
                            mCam->releaseFrame(index);
                            continue;
                        }
                        memcpy(img, frame, mCam->getImageSize());
                        buf->unlock();
                        mCam->releaseFrame(index);
                        mANW->queueBuffer(mANW.get(), buf->getNativeBuffer(), -1);
                        mST->updateTexImage();
                    }
                }
            }

            glClear(GL_COLOR_BUFFER_BIT);
            Renderer->DrawSprite(ResourceManager::GetTexture("camera"), glm::vec2(50, 5),
                                 glm::vec2(mCamWidth, mCamHeight), 0);

            EGLBoolean res = eglSwapBuffers(mDisplay, mSurface);
            if (res == EGL_FALSE) {
                //mCam->releaseFrame(index);
                break;
            }
#if 0
            gettimeofday(&t2, NULL);
            double elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0 + (t2.tv_usec - t1.tv_usec) / 1000.0;
            ALOGE("%f ms\r\n",elapsedTime);
#endif
            //printf("%f ms\r\n",elapsedTime);

        }

        mCam->stopCapture();
        delete mCam;
        mCam = NULL;
        return false;
    }

    int CameraCapture::testInit() {
        ALOGI("%s\n", __FUNCTION__);
        return 0;
    }

    bool CameraCapture::testLoop() {
        ALOGI("%s\n", __FUNCTION__);
        int index = 0, err = 0;
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        mCamWidth = 720;
        mCamHeight = 480;
        while (1) {
            native_window_set_buffers_geometry(mANW.get(), mCamWidth, mCamHeight, HAL_PIXEL_FORMAT_YV12);
            native_window_set_usage(mANW.get(), GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN);
            ANativeWindowBuffer *anb = NULL;
            err = native_window_dequeue_buffer_and_wait(mANW.get(), &anb);
            if ((err != NO_ERROR) || (anb == NULL)) {
                ALOGE("native_window_dequeue_buffer_and_wait failed");
                continue;
            }
            sp<GraphicBuffer> buf(new GraphicBuffer(anb, false));
            uint8_t *img = NULL;
            buf->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void **) (&img));
            if (img == NULL) {
                ALOGE("GRALLOC_USAGE_SW_WRITE_OFTEN is NULL");
                continue;
            }
            memset(img, index++ & 0xff, mCamHeight * mCamWidth * 2);
            buf->unlock();
            mANW->queueBuffer(mANW.get(), buf->getNativeBuffer(), -1);
            mST->updateTexImage();

            glClear(GL_COLOR_BUFFER_BIT);
            Renderer->DrawSprite(ResourceManager::GetTexture("camera"), glm::vec2(0, 0),
                                 glm::vec2(mWidth, mHeight), 0);

            EGLBoolean res = eglSwapBuffers(mDisplay, mSurface);
            if (res == EGL_FALSE) {
                ALOGE("eglSwapBuffers failed");
                break;
            }
            usleep(1000 * 30);
            checkExit();
            if (exitPending()) {
                break;
            }
        }
        return false;
    }

}; // namespace android
