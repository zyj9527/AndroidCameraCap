/*
 * Copyright (C) 2009 The Android Open Source Project
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


#ifndef ANDROID_UI_EGLUTILS_H
#define ANDROID_UI_EGLUTILS_H

#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <system/window.h>
#include <utils/Errors.h>
#include <utils/Log.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

// ----------------------------------------------------------------------------
namespace android {
// ----------------------------------------------------------------------------
#define LITERAL_TO_STRING_INTERNAL(x)    #x
#define LITERAL_TO_STRING(x) LITERAL_TO_STRING_INTERNAL(x)

#define CHECK(condition)                                \
    LOG_ALWAYS_FATAL_IF(                                \
            !(condition),                               \
            "%s",                                       \
            __FILE__ ":" LITERAL_TO_STRING(__LINE__)    \
            " CHECK(" #condition ") failed.")

#define CHECK_EGL_ERROR CHECK(EGL_SUCCESS == eglGetError())
#define CHECK_GL_ERROR CHECK(GLenum(GL_NO_ERROR) == glGetError())

    class EGLUtils {
    public:

        static inline const char *strerror(EGLint err);

        static inline status_t selectConfigForPixelFormat(
                EGLDisplay dpy,
                EGLint const *attrs,
                int32_t format,
                EGLConfig *outConfig);

        static inline status_t selectConfigForNativeWindow(
                EGLDisplay dpy,
                EGLint const *attrs,
                EGLNativeWindowType window,
                EGLConfig *outConfig);

        static inline GLuint createProgram(const char *pVertexSource, const char *pFragmentSource);

        static inline GLuint loadShader(GLenum shaderType, const char *pSource);

        static inline void
        orthoM(float *m, int mOffset, float left, float right, float bottom, float top, float near, float far);

        static inline void
        frustumM(float *m, int offset, float left, float right, float bottom, float top, float near, float far);

        static inline void
        setLookAtM(float *rm, int rmOffset, float eyeX, float eyeY, float eyeZ, float centerX, float centerY,
                   float centerZ, float upX, float upY, float upZ);

        static inline void translateM(float *tm, int tmOffset, float *m, int mOffset, float x, float y, float z);

        static inline void translateM(float *m, int mOffset, float x, float y, float z);

        static inline float length(float x, float y, float z);

        static inline void multiplyMM(float *r, const float *lhs, const float *rhs);

        static inline void setIdentityM(float *sm, int smOffset);
    };

    void EGLUtils::setIdentityM(float *sm, int smOffset) {
        for (int i = 0; i < 16; i++) {
            sm[smOffset + i] = 0;
        }
        for (int i = 0; i < 16; i += 5) {
            sm[smOffset + i] = 1.0f;
        }
    }

    void
    EGLUtils::orthoM(float *m, int mOffset, float left, float right, float bottom, float top, float near, float far) {
        float r_width = 1.0f / (right - left);
        float r_height = 1.0f / (top - bottom);
        float r_depth = 1.0f / (far - near);
        float x = 2.0f * (r_width);
        float y = 2.0f * (r_height);
        float z = -2.0f * (r_depth);
        float tx = -(right + left) * r_width;
        float ty = -(top + bottom) * r_height;
        float tz = -(far + near) * r_depth;
        m[mOffset + 0] = x;
        m[mOffset + 5] = y;
        m[mOffset + 10] = z;
        m[mOffset + 12] = tx;
        m[mOffset + 13] = ty;
        m[mOffset + 14] = tz;
        m[mOffset + 15] = 1.0f;
        m[mOffset + 1] = 0.0f;
        m[mOffset + 2] = 0.0f;
        m[mOffset + 3] = 0.0f;
        m[mOffset + 4] = 0.0f;
        m[mOffset + 6] = 0.0f;
        m[mOffset + 7] = 0.0f;
        m[mOffset + 8] = 0.0f;
        m[mOffset + 9] = 0.0f;
        m[mOffset + 11] = 0.0f;
    }

    void
    EGLUtils::frustumM(float *m, int offset, float left, float right, float bottom, float top, float near, float far) {
        float r_width = 1.0f / (right - left);
        float r_height = 1.0f / (top - bottom);
        float r_depth = 1.0f / (near - far);
        float x = 2.0f * (near * r_width);
        float y = 2.0f * (near * r_height);
        float A = (right + left) * r_width;
        float B = (top + bottom) * r_height;
        float C = (far + near) * r_depth;
        float D = 2.0f * (far * near * r_depth);
        m[offset + 0] = x;
        m[offset + 5] = y;
        m[offset + 8] = A;
        m[offset + 9] = B;
        m[offset + 10] = C;
        m[offset + 14] = D;
        m[offset + 11] = -1.0f;
        m[offset + 1] = 0.0f;
        m[offset + 2] = 0.0f;
        m[offset + 3] = 0.0f;
        m[offset + 4] = 0.0f;
        m[offset + 6] = 0.0f;
        m[offset + 7] = 0.0f;
        m[offset + 12] = 0.0f;
        m[offset + 13] = 0.0f;
        m[offset + 15] = 0.0f;
    }

    void EGLUtils::translateM(float *tm, int tmOffset, float *m, int mOffset, float x, float y, float z) {
        for (int i = 0; i < 12; i++) {
            tm[tmOffset + i] = m[mOffset + i];
        }
        for (int i = 0; i < 4; i++) {
            int tmi = tmOffset + i;
            int mi = mOffset + i;
            tm[12 + tmi] = m[mi] * x + m[4 + mi] * y + m[8 + mi] * z + m[12 + mi];
        }
    }

    void EGLUtils::translateM(float *m, int mOffset, float x, float y, float z) {
        for (int i = 0; i < 4; i++) {
            int mi = mOffset + i;
            m[12 + mi] += m[mi] * x + m[4 + mi] * y + m[8 + mi] * z;
        }
    }


    float EGLUtils::length(float x, float y, float z) {
        return (float) sqrt(x * x + y * y + z * z);
    }

    void EGLUtils::setLookAtM(float *rm, int rmOffset, float eyeX, float eyeY, float eyeZ, float centerX, float centerY,
                              float centerZ, float upX, float upY, float upZ) {
        float fx = centerX - eyeX;
        float fy = centerY - eyeY;
        float fz = centerZ - eyeZ;

        float rlf = 1.0f / length(fx, fy, fz);
        fx *= rlf;
        fy *= rlf;
        fz *= rlf;

        float sx = fy * upZ - fz * upY;
        float sy = fz * upX - fx * upZ;
        float sz = fx * upY - fy * upX;

        float rls = 1.0f / length(sx, sy, sz);
        sx *= rls;
        sy *= rls;
        sz *= rls;

        float ux = sy * fz - sz * fy;
        float uy = sz * fx - sx * fz;
        float uz = sx * fy - sy * fx;

        rm[rmOffset + 0] = sx;
        rm[rmOffset + 1] = ux;
        rm[rmOffset + 2] = -fx;
        rm[rmOffset + 3] = 0.0f;

        rm[rmOffset + 4] = sy;
        rm[rmOffset + 5] = uy;
        rm[rmOffset + 6] = -fy;
        rm[rmOffset + 7] = 0.0f;

        rm[rmOffset + 8] = sz;
        rm[rmOffset + 9] = uz;
        rm[rmOffset + 10] = -fz;
        rm[rmOffset + 11] = 0.0f;

        rm[rmOffset + 12] = 0.0f;
        rm[rmOffset + 13] = 0.0f;
        rm[rmOffset + 14] = 0.0f;
        rm[rmOffset + 15] = 1.0f;

        translateM(rm, rmOffset, -eyeX, -eyeY, -eyeZ);
    }

#define I(_i, _j) ((_j)+ 4*(_i))

    void EGLUtils::multiplyMM(float *r, const float *lhs, const float *rhs) {
        for (int i = 0; i < 4; i++) {
            register const float rhs_i0 = rhs[I(i, 0)];
            register float ri0 = lhs[I(0, 0)] * rhs_i0;
            register float ri1 = lhs[I(0, 1)] * rhs_i0;
            register float ri2 = lhs[I(0, 2)] * rhs_i0;
            register float ri3 = lhs[I(0, 3)] * rhs_i0;
            for (int j = 1; j < 4; j++) {
                register const float rhs_ij = rhs[I(i, j)];
                ri0 += lhs[I(j, 0)] * rhs_ij;
                ri1 += lhs[I(j, 1)] * rhs_ij;
                ri2 += lhs[I(j, 2)] * rhs_ij;
                ri3 += lhs[I(j, 3)] * rhs_ij;
            }
            r[I(i, 0)] = ri0;
            r[I(i, 1)] = ri1;
            r[I(i, 2)] = ri2;
            r[I(i, 3)] = ri3;
        }
    }

    GLuint EGLUtils::loadShader(GLenum shaderType, const char *pSource) {
        GLuint shader = glCreateShader(shaderType);
        CHECK_GL_ERROR;
        //ASSERT_EQ(GLenum(GL_NO_ERROR), glGetError());
        if (shader) {
            glShaderSource(shader, 1, &pSource, NULL);
            //ASSERT_EQ(GLenum(GL_NO_ERROR), glGetError());
            glCompileShader(shader);
            //ASSERT_EQ(GLenum(GL_NO_ERROR), glGetError());
            GLint compiled = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
            //ASSERT_EQ(GLenum(GL_NO_ERROR), glGetError());
            if (!compiled) {
                GLint infoLen = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
                // ASSERT_EQ(GLenum(GL_NO_ERROR), glGetError());
                if (infoLen) {
                    char *buf = (char *) malloc(infoLen);
                    if (buf) {
                        glGetShaderInfoLog(shader, infoLen, NULL, buf);
                        //printf("Shader compile log:\n%s\n", buf);
                        free(buf);
                        //FAIL();
                    }
                } else {
                    char *buf = (char *) malloc(0x1000);
                    if (buf) {
                        glGetShaderInfoLog(shader, 0x1000, NULL, buf);
                        //printf("Shader compile log:\n%s\n", buf);
                        free(buf);
                        //FAIL();
                    }
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
        return shader;
    }

    GLuint EGLUtils::createProgram(const char *pVertexSource, const char *pFragmentSource) {
        GLuint vertexShader, fragmentShader;
        vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
        if (vertexShader == 0)
            return 0;

        fragmentShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
        if (fragmentShader == 0)
            return 0;

        GLuint program = glCreateProgram();
        //ASSERT_EQ(GLenum(GL_NO_ERROR), glGetError());
        if (program) {
            glAttachShader(program, vertexShader);
            //ASSERT_EQ(GLenum(GL_NO_ERROR), glGetError());
            glAttachShader(program, fragmentShader);
            //ASSERT_EQ(GLenum(GL_NO_ERROR), glGetError());
            glLinkProgram(program);
            GLint linkStatus = GL_FALSE;
            glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
            if (linkStatus != GL_TRUE) {
                GLint bufLength = 0;
                glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
                if (bufLength) {
                    char *buf = (char *) malloc(bufLength);
                    if (buf) {
                        glGetProgramInfoLog(program, bufLength, NULL, buf);
                        //printf("Program link log:\n%s\n", buf);
                        free(buf);
                        //FAIL();
                    }
                }
                glDeleteProgram(program);
                program = 0;
            }
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return program;
    }

    const char *EGLUtils::strerror(EGLint err) {
        switch (err) {
            case EGL_SUCCESS:
                return "EGL_SUCCESS";
            case EGL_NOT_INITIALIZED:
                return "EGL_NOT_INITIALIZED";
            case EGL_BAD_ACCESS:
                return "EGL_BAD_ACCESS";
            case EGL_BAD_ALLOC:
                return "EGL_BAD_ALLOC";
            case EGL_BAD_ATTRIBUTE:
                return "EGL_BAD_ATTRIBUTE";
            case EGL_BAD_CONFIG:
                return "EGL_BAD_CONFIG";
            case EGL_BAD_CONTEXT:
                return "EGL_BAD_CONTEXT";
            case EGL_BAD_CURRENT_SURFACE:
                return "EGL_BAD_CURRENT_SURFACE";
            case EGL_BAD_DISPLAY:
                return "EGL_BAD_DISPLAY";
            case EGL_BAD_MATCH:
                return "EGL_BAD_MATCH";
            case EGL_BAD_NATIVE_PIXMAP:
                return "EGL_BAD_NATIVE_PIXMAP";
            case EGL_BAD_NATIVE_WINDOW:
                return "EGL_BAD_NATIVE_WINDOW";
            case EGL_BAD_PARAMETER:
                return "EGL_BAD_PARAMETER";
            case EGL_BAD_SURFACE:
                return "EGL_BAD_SURFACE";
            case EGL_CONTEXT_LOST:
                return "EGL_CONTEXT_LOST";
            default:
                return "UNKNOWN";
        }
    }

    status_t EGLUtils::selectConfigForPixelFormat(
            EGLDisplay dpy,
            EGLint const *attrs,
            int32_t format,
            EGLConfig *outConfig) {
        EGLint numConfigs = -1, n = 0;

        if (!attrs)
            return BAD_VALUE;

        if (outConfig == NULL)
            return BAD_VALUE;

        // Get all the "potential match" configs...
        if (eglGetConfigs(dpy, NULL, 0, &numConfigs) == EGL_FALSE)
            return BAD_VALUE;

        EGLConfig *const configs = (EGLConfig *) malloc(sizeof(EGLConfig) * numConfigs);
        if (eglChooseConfig(dpy, attrs, configs, numConfigs, &n) == EGL_FALSE) {
            free(configs);
            return BAD_VALUE;
        }

        int i;
        EGLConfig config = NULL;
        for (i = 0; i < n; i++) {
            EGLint nativeVisualId = 0;
            eglGetConfigAttrib(dpy, configs[i], EGL_NATIVE_VISUAL_ID, &nativeVisualId);
            if (nativeVisualId > 0 && format == nativeVisualId) {
                config = configs[i];
                break;
            }
        }

        free(configs);

        if (i < n) {
            *outConfig = config;
            return NO_ERROR;
        }

        return NAME_NOT_FOUND;
    }

    status_t EGLUtils::selectConfigForNativeWindow(
            EGLDisplay dpy,
            EGLint const *attrs,
            EGLNativeWindowType window,
            EGLConfig *outConfig) {
        int err;
        int format;

        if (!window)
            return BAD_VALUE;

        if ((err = window->query(window, NATIVE_WINDOW_FORMAT, &format)) < 0) {
            return err;
        }

        return selectConfigForPixelFormat(dpy, attrs, format, outConfig);
    }

// ----------------------------------------------------------------------------
}; // namespace android
// ----------------------------------------------------------------------------

#endif /* ANDROID_UI_EGLUTILS_H */
