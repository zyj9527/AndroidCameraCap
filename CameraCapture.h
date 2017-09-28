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

#ifndef ANDROID_CAMERACAPTURE_H
#define ANDROID_CAMERACAPTURE_H

#include <stdint.h>
#include <sys/types.h>

#include <androidfw/AssetManager.h>
#include <utils/threads.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <gui/GLConsumer.h>
#include <ui/GraphicBuffer.h>

#include "CameraIn.h"


#include "resource_manager.h"
#include "cam_renderer.h"

class SkBitmap;

namespace android {

    class Surface;

    class SurfaceComposerClient;

    class SurfaceControl;

#define TEX_TARGET        123
// ---------------------------------------------------------------------------

    class CameraCapture : public Thread, public IBinder::DeathRecipient {
    public:
        CameraCapture();

        virtual     ~CameraCapture();

        sp<SurfaceComposerClient> session() const;

    private:
        virtual bool threadLoop();

        virtual status_t readyToRun();

        virtual void onFirstRef();

        virtual void binderDied(const wp<IBinder> &who);

        void checkExit();

        int setupEGL();

        int v4l2Init();

        bool v4l2Loop();

        int testInit();

        bool testLoop();

        sp<SurfaceComposerClient> mSession;
        AssetManager mAssets;

        int mWidth;
        int mHeight;
        float_t mRatio;
        EGLDisplay mDisplay;
        EGLDisplay mContext;
        EGLDisplay mSurface;
        sp<SurfaceControl> mFlingerSurfaceControl;
        sp<Surface> mFlingerSurface;
        bool mAndroidAnimation;

        sp<BufferQueue> mBQ;
        sp<GLConsumer> mST;
        sp<Surface> mSTC;
        sp<ANativeWindow> mANW;

        GLuint mTextureID;
        GLuint mProgram;
        GLint muMVPMatrixHandle;
        GLint muSTMatrixHandle;
        GLint muCRatioHandle;
        GLint maPositionHandle;
        GLint maTextureHandle;

        float mMVPMatrix[16];
        float mProjMatrix[16];
        float mMMatrix[16];
        float mVMatrix[16];
        float mSTMatrix[16];
        float mPos[3];

        CameraIn *mCam;
        int mCamWidth;
        int mCamHeight;
        float mCamRatio;

        CamRenderer *Renderer;
    };

// ---------------------------------------------------------------------------

}; // namespace android

#endif // ANDROID_CAMERACAPTURE_H
