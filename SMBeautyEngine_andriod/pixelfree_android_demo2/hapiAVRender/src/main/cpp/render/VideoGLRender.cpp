//
// Created by 1 on 2022/3/28.
//

#include "VideoGLRender.h"
#include "LogUtil.h"
#include "gtc/matrix_transform.hpp"
#include "GLUtils.h"

//static char vShaderStr[] =
//        "#version 300 es\n"
//        "layout(location = 0) in vec4 a_position;\n"
//        "layout(location = 1) in vec2 a_texCoord;\n"
//        "uniform mat4 u_MVPMatrix;\n"
//        "out vec2 v_texCoord;\n"
//        "void main()\n"
//        "{\n"
//        "    gl_Position = u_MVPMatrix * a_position;\n"
//        "    v_texCoord = a_texCoord;\n"
//        "}";

static char vShaderStr[] =
        "attribute vec4 a_position;\n"
        "attribute vec2 a_texCoord;\n"
        "uniform mat4 u_MVPMatrix;\n"
        "varying vec2 v_texCoord;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = u_MVPMatrix * a_position;\n"
        "    v_texCoord = a_texCoord;\n"
        "}";

//static char fShaderStr[] =
//        "#version 300 es\n"
//        "precision highp float;\n"
//        "in vec2 v_texCoord;\n"
//        "layout(location = 0) out vec4 outColor;\n"
//        "uniform sampler2D s_texture0;\n"
//        "uniform sampler2D s_texture1;\n"
//        "uniform sampler2D s_texture2;\n"
//        "uniform int u_nImgType;// 1:RGBA, 2:NV21, 3:NV12, 4:I420\n"
//        "\n"
//        "void main()\n"
//        "{\n"
//        "\n"
//        "    if(u_nImgType == 1) //RGBA\n"
//        "    {\n"
//        "        outColor = texture(s_texture0, v_texCoord);\n"
//        "    }\n"
//        "    else if(u_nImgType == 2) //NV21\n"
//        "    {\n"
//        "        vec3 yuv;\n"
//        "        yuv.x = texture(s_texture0, v_texCoord).r;\n"
//        "        yuv.y = texture(s_texture1, v_texCoord).a - 0.5;\n"
//        "        yuv.z = texture(s_texture1, v_texCoord).r - 0.5;\n"
//        "        highp vec3 rgb = mat3(1.0,       1.0,     1.0,\n"
//        "        0.0, \t-0.344, \t1.770,\n"
//        "        1.403,  -0.714,     0.0) * yuv;\n"
//        "        outColor = vec4(rgb, 1.0);\n"
//        "\n"
//        "    }\n"
//        "    else if(u_nImgType == 3) //NV12\n"
//        "    {\n"
//        "        vec3 yuv;\n"
//        "        yuv.x = texture(s_texture0, v_texCoord).r;\n"
//        "        yuv.y = texture(s_texture1, v_texCoord).r - 0.5;\n"
//        "        yuv.z = texture(s_texture1, v_texCoord).a - 0.5;\n"
//        "        highp vec3 rgb = mat3(1.0,       1.0,     1.0,\n"
//        "        0.0, \t-0.344, \t1.770,\n"
//        "        1.403,  -0.714,     0.0) * yuv;\n"
//        "        outColor = vec4(rgb, 1.0);\n"
//        "    }\n"
//        "    else if(u_nImgType == 4) //I420\n"
//        "    {\n"
//        "        vec3 yuv;\n"
//        "        yuv.x = texture(s_texture0, v_texCoord).r;\n"
//        "        yuv.y = texture(s_texture1, v_texCoord).r - 0.5;\n"
//        "        yuv.z = texture(s_texture2, v_texCoord).r - 0.5;\n"
//        "        highp vec3 rgb = mat3(1.0,       1.0,     1.0,\n"
//        "                              0.0, \t-0.344, \t1.770,\n"
//        "                              1.403,  -0.714,     0.0) * yuv;\n"
//        "        outColor = vec4(rgb, 1.0);\n"
//        "    }\n"
//        "    else\n"
//        "    {\n"
//        "        outColor = vec4(1.0);\n"
//        "    }\n"
//        "}";
static char fShaderStr[] =
        "precision highp float;\n"
        "varying vec2 v_texCoord;\n"
        "uniform sampler2D s_texture0;\n"
        "uniform sampler2D s_texture1;\n"
        "uniform sampler2D s_texture2;\n"
        "uniform int u_nImgType; // 1:RGBA, 2:NV21, 3:NV12, 4:I420\n"
        "\n"
        "void main()\n"
        "{\n"
        "    if(u_nImgType == 1) //RGBA\n"
        "    {\n"
        "        gl_FragColor = texture2D(s_texture0, v_texCoord);\n"
        "    }\n"
        "    else if(u_nImgType == 2) //NV21\n"
        "    {\n"
        "        vec3 yuv;\n"
        "        yuv.x = texture2D(s_texture0, v_texCoord).r;\n"
        "        yuv.y = texture2D(s_texture1, v_texCoord).a - 0.5;\n"
        "        yuv.z = texture2D(s_texture1, v_texCoord).r - 0.5;\n"
        "        highp vec3 rgb = mat3(1.0,       1.0,     1.0,\n"
        "        0.0, \t-0.344, \t1.770,\n"
        "        1.403,  -0.714,     0.0) * yuv;\n"
        "        gl_FragColor = vec4(rgb, 1.0);\n"
        "    }\n"
        "    else if(u_nImgType == 3) //NV12\n"
        "    {\n"
        "        vec3 yuv;\n"
        "        yuv.x = texture2D(s_texture0, v_texCoord).r;\n"
        "        yuv.y = texture2D(s_texture1, v_texCoord).r - 0.5;\n"
        "        yuv.z = texture2D(s_texture1, v_texCoord).a - 0.5;\n"
        "        highp vec3 rgb = mat3(1.0,       1.0,     1.0,\n"
        "        0.0, \t-0.344, \t1.770,\n"
        "        1.403,  -0.714,     0.0) * yuv;\n"
        "        gl_FragColor = vec4(rgb, 1.0);\n"
        "    }\n"
        "    else if(u_nImgType == 4) //I420\n"
        "    {\n"
        "        vec3 yuv;\n"
        "        yuv.x = texture2D(s_texture0, v_texCoord).r;\n"
        "        yuv.y = texture2D(s_texture1, v_texCoord).r - 0.5;\n"
        "        yuv.z = texture2D(s_texture2, v_texCoord).r - 0.5;\n"
        "        highp vec3 rgb = mat3(1.0,       1.0,     1.0,\n"
        "                              0.0, \t-0.344, \t1.770,\n"
        "                              1.403,  -0.714,     0.0) * yuv;\n"
        "        gl_FragColor = vec4(rgb, 1.0);\n"
        "    }\n"
        "    else\n"
        "    {\n"
        "        gl_FragColor = vec4(1.0);\n"
        "    }\n"
        "}";

GLfloat verticesCoords[] = {
        -1.0f, 1.0f, 0.0f,  // Position 0
        -1.0f, -1.0f, 0.0f,  // Position 1
        1.0f, -1.0f, 0.0f,  // Position 2
        1.0f, 1.0f, 0.0f,  // Position 3
};

GLfloat textureCoords[] = {
        0.0f, 0.0f,        // TexCoord 0
        0.0f, 1.0f,        // TexCoord 1
        1.0f, 1.0f,        // TexCoord 2
        1.0f, 0.0f         // TexCoord 3
};

GLushort indices[] = {0, 1, 2, 0, 2, 3};

VideoGLRender::VideoGLRender() {
    m_FrameIndex = 0;
    UpdateMVPMatrix(0, 0, 1.0f, 1.0f);
}


void VideoGLRender::RenderVideoFrame(NativeImage *image) {
    if (image == nullptr) return;

    int width = image->width;
    int height = image->height;
    int pixel_stride = image->pixelStride;
    int row_padding = image->rowPadding;
    int rotation_degrees = image->rotationDegrees;
    int dataLen = image->pLineSize[0];
    int format = image->format;

    std::lock_guard<std::mutex> lock(m_RenderImageMutex);

    if (m_RenderImage != nullptr) {
        if (width != m_RenderImage->width
            || height != m_RenderImage->height
            || format != m_RenderImage->format
                ) {
            delete m_RenderImage;
            m_RenderImage = nullptr;
        }
    }

    if (image->textureID <= 0) {
        if (image->ppPlane[0] == nullptr && dataLen > 0) return;

        switch (format) {
            case IMAGE_FORMAT_I420:
                if (m_RenderImage == nullptr) {
                    m_RenderImage = new NativeImage();
                    m_RenderImage->pLineSize[0] = dataLen;
                    uint8_t *buf = static_cast<uint8_t *>(malloc(dataLen));
                    if (!buf) return;
                    m_RenderImage->ppPlane[0] = buf;
                }
                memcpy(m_RenderImage->ppPlane[0], image->ppPlane[0], dataLen);
                m_RenderImage->ppPlane[1] = m_RenderImage->ppPlane[0] + width * height;
                m_RenderImage->ppPlane[2] = m_RenderImage->ppPlane[1] + width * height / 4;
                break;
            case IMAGE_FORMAT_NV12:
            case IMAGE_FORMAT_NV21:
                if (m_RenderImage == nullptr) {
                    m_RenderImage = new NativeImage();
                    m_RenderImage->pLineSize[0] = dataLen;
                    uint8_t *buf = static_cast<uint8_t *>(malloc(dataLen));
                    if (!buf) return;
                    m_RenderImage->ppPlane[0] = buf;
                }
                memcpy(m_RenderImage->ppPlane[0], image->ppPlane[0], dataLen);
                m_RenderImage->ppPlane[1] = m_RenderImage->ppPlane[0] + width * height;
                break;
            default:
                //rgba
                int effective_stride = (pixel_stride > 0) ? pixel_stride : 4;
                int lenNew = width * height * effective_stride;
                if (lenNew <= 0) return;

                if (m_RenderImage == nullptr || mUseTexture) {
                    mUseTexture = false;
                    m_RenderImage = new NativeImage();
                    m_RenderImage->pLineSize[0] = lenNew;
                    uint8_t *buf = static_cast<uint8_t *>(malloc(lenNew));
                    if (!buf) return;
                    m_RenderImage->ppPlane[0] = buf;
                }

                if (m_RenderImage->pLineSize[0] != lenNew) {
                    m_RenderImage->pLineSize[0] = lenNew;
                    free(m_RenderImage->ppPlane[0]);
                    uint8_t *buf = static_cast<uint8_t *>(malloc(lenNew));
                    if (!buf) return;
                    m_RenderImage->ppPlane[0] = buf;
                }
                if (row_padding > 0) {
                    int row = height;
                    int srcStride = width * (pixel_stride > 0 ? pixel_stride : effective_stride) + row_padding;
                    int line = width * effective_stride;
                    if (line > srcStride) line = srcStride;
                    for (int i = 0; i < row; i++) {
                        int start = (width * effective_stride) * i;
                        int start2 = srcStride * i;
                        memcpy(m_RenderImage->ppPlane[0] + start, image->ppPlane[0] + start2, (size_t)line);
                    }
                } else {
                    size_t copyLen = (size_t)(dataLen < lenNew ? dataLen : lenNew);
                    memcpy(m_RenderImage->ppPlane[0], image->ppPlane[0], copyLen);
                }
                break;
        }
    } else {
        if (m_RenderImage == nullptr || !mUseTexture) {
            m_RenderImage = new NativeImage();
            mUseTexture = true;
        }
        m_RenderImage->textureID = image->textureID;
    }
    m_RenderImage->rotationDegrees = rotation_degrees;
    m_RenderImage->width = width;
    m_RenderImage->height = height;
    m_RenderImage->format = format;
}

void VideoGLRender::UnInit() {
    std::lock_guard<std::mutex> lock(m_RenderImageMutex);
    if (m_RenderImage != nullptr) {
        delete m_RenderImage;
        m_RenderImage = nullptr;
    }
}

void VideoGLRender::UpdateMVPMatrix(int angleX, int angleY, float scaleX, float scaleY) {
    UpdateMVPMatrix(angleX, angleY, scaleX, scaleY, false, false);
}

void VideoGLRender::UpdateMVPMatrix(int angleX, int angleY, float scaleX, float scaleY, bool mirrorHorizontal, bool mirrorVertical) {
    angleX = angleX % 360;
    angleY = angleY % 360;

    //转化为弧度角
    auto radiansX = static_cast<float>(MATH_PI / 180.0f * angleX);
    auto radiansY = static_cast<float>(MATH_PI / 180.0f * angleY);
    
    // Projection matrix
    glm::mat4 Projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);

    // View matrix
    glm::mat4 View = glm::lookAt(
            glm::vec3(0, 0, 4), // Camera is at (0,0,1), in World Space
            glm::vec3(0, 0, 0), // and looks at the origin
            glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
    );

    // Model matrix
    glm::mat4 Model = glm::mat4(1.0f);
    
    // Apply mirroring
    if (mirrorHorizontal) {
        Model = glm::scale(Model, glm::vec3(-1.0f, 1.0f, 1.0f));
    }
    if (mirrorVertical) {
        Model = glm::scale(Model, glm::vec3(1.0f, -1.0f, 1.0f));
    }
    
    // Apply scaling
    Model = glm::scale(Model, glm::vec3(scaleX, scaleY, 1.0f));
    
    // Apply rotation
    Model = glm::rotate(Model, radiansX, glm::vec3(1.0f, 0.0f, 0.0f));
    Model = glm::rotate(Model, radiansY + static_cast<float>(MATH_PI), glm::vec3(0.0f, 1.0f, 0.0f));
    
    // Apply translation
    Model = glm::translate(Model, glm::vec3(0.0f, 0.0f, 0.0f));
    
    m_MVPMatrix = Projection * View * Model;
}


void VideoGLRender::UpdateMVPMatrix(TransformMatrix *pTransformMatrix) {
    //BaseGLRender::UpdateMVPMatrix(pTransformMatrix);
    float fFactorX = 1.0f;
    float fFactorY = 1.0f;

    if (pTransformMatrix->mirror == 1) {
        fFactorX = -1.0f;
    } else if (pTransformMatrix->mirror == 2) {
        fFactorY = -1.0f;
    }

    float fRotate = MATH_PI * pTransformMatrix->degree * 1.0f / 180;
    if (pTransformMatrix->mirror == 0) {
        if (pTransformMatrix->degree == 270) {
            fRotate = MATH_PI * 0.5;
        } else if (pTransformMatrix->degree == 180) {
            fRotate = MATH_PI;
        } else if (pTransformMatrix->degree == 90) {
            fRotate = MATH_PI * 1.5;
        }
    } else if (pTransformMatrix->mirror == 1) {
        if (pTransformMatrix->degree == 90) {
            fRotate = MATH_PI * 0.5;
        } else if (pTransformMatrix->degree == 180) {
            fRotate = MATH_PI;
        } else if (pTransformMatrix->degree == 270) {
            fRotate = MATH_PI * 1.5;
        }
    }

    glm::mat4 Projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f);
    glm::mat4 View = glm::lookAt(
            glm::vec3(0, 0, 1), // Camera is at (0,0,1), in World Space
            glm::vec3(0, 0, 0), // and looks at the origin
            glm::vec3(0, 1, 0) // Head is up (set to 0,-1,0 to look upside-down)
    );

    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::scale(Model, glm::vec3(fFactorX * pTransformMatrix->scaleX,
                                        fFactorY * pTransformMatrix->scaleY, 1.0f));
    Model = glm::rotate(Model, fRotate, glm::vec3(0.0f, 0.0f, 1.0f));

    Model = glm::translate(Model,
                           glm::vec3(pTransformMatrix->translateX, pTransformMatrix->translateY,
                                     0.0f));
    LOGCATE("VideoGLRender::UpdateMVPMatrix rotate %d,%.2f,%0.5f,%0.5f,%0.5f,%0.5f,",
            pTransformMatrix->degree, fRotate,
            pTransformMatrix->translateX, pTransformMatrix->translateY,
            fFactorX * pTransformMatrix->scaleX, fFactorY * pTransformMatrix->scaleY);

    m_MVPMatrix = Projection * View * Model;
}


void VideoGLRender::OnSurfaceCreated() {
    LOGCATE("VideoGLRender::OnSurfaceCreated");

    m_ProgramObj = GLUtils::CreateProgram(vShaderStr, fShaderStr);
    if (!m_ProgramObj) {
        LOGCATE("VideoGLRender::OnSurfaceCreated create program fail");
        return;
    }

    glGenTextures(TEXTURE_NUM, m_TextureIds);
    for (int i = 0; i < TEXTURE_NUM; ++i) {
        LOGCATE("VideoGLRender::glGenTextures %d", m_TextureIds[i]);
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_TextureIds[i]);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, GL_NONE);
    }

    // Generate VBO Ids and load the VBOs with data
    glGenBuffers(3, m_VboIds);
    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCoords), verticesCoords, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoords), textureCoords, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Generate VAO Id
    glGenVertexArrays(1, &m_VaoId);
    glBindVertexArray(m_VaoId);

    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (const void *) 0);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (const void *) 0);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[2]);

    glBindVertexArray(GL_NONE);

    m_TouchXY = vec2(0.5f, 0.5f);
}


void VideoGLRender::OnSurfaceChanged(int w, int h) {
    LOGCATE("VideoGLRender::OnSurfaceChanged [w, h]=[%d, %d]", w, h);
    m_ScreenSize.x = w;
    m_ScreenSize.y = h;
    glViewport(0, 0, w, h);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}


void VideoGLRender::OnDrawFrame() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Copy necessary data under lock to minimize lock duration
    int textureID = -1000;
    int format = 0;
    int width = 0;
    int height = 0;
    int rotationDegrees = 0;
    uint8_t *ppPlane[3] = {nullptr, nullptr, nullptr};
    GLuint textureIds[TEXTURE_NUM];
    for (int i = 0; i < TEXTURE_NUM; ++i) {
        textureIds[i] = m_TextureIds[i];
    }

    {
        std::lock_guard<std::mutex> lock(m_RenderImageMutex);
        if (m_RenderImage == nullptr) {
            return;
        }
        m_FrameIndex++;
        textureID = m_RenderImage->textureID;
        format = m_RenderImage->format;
        width = m_RenderImage->width;
        height = m_RenderImage->height;
        rotationDegrees = m_RenderImage->rotationDegrees;
        if (textureID <= 0) {
            ppPlane[0] = m_RenderImage->ppPlane[0];
            ppPlane[1] = m_RenderImage->ppPlane[1];
            ppPlane[2] = m_RenderImage->ppPlane[2];
        } else {
            textureIds[0] = m_RenderImage->textureID;
        }
    }

    // Perform GL operations outside the lock
    if (textureID > 0) {
        m_TextureIds[0] = textureIds[0];
    } else {
        switch (format) {
            case IMAGE_FORMAT_RGBA:
                if (ppPlane[0] != nullptr) {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, m_TextureIds[0]);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
                                 0, GL_RGBA, GL_UNSIGNED_BYTE, ppPlane[0]);
                    glBindTexture(GL_TEXTURE_2D, GL_NONE);
                }
                break;
            case IMAGE_FORMAT_NV21:
            case IMAGE_FORMAT_NV12:
                if (ppPlane[0] != nullptr && ppPlane[1] != nullptr) {
                    //upload Y plane data
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, m_TextureIds[0]);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height,
                                 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, ppPlane[0]);
                    glBindTexture(GL_TEXTURE_2D, GL_NONE);

                    //update UV plane data
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, m_TextureIds[1]);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, width >> 1,
                                 height >> 1, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, ppPlane[1]);
                    glBindTexture(GL_TEXTURE_2D, GL_NONE);
                }
                break;
            case IMAGE_FORMAT_I420:
                if (ppPlane[0] != nullptr && ppPlane[1] != nullptr && ppPlane[2] != nullptr) {
                    //upload Y plane data
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, m_TextureIds[0]);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height,
                                 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, ppPlane[0]);
                    glBindTexture(GL_TEXTURE_2D, GL_NONE);

                    //update U plane data
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, m_TextureIds[1]);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width >> 1,
                                 height >> 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, ppPlane[1]);
                    glBindTexture(GL_TEXTURE_2D, GL_NONE);

                    //update V plane data
                    glActiveTexture(GL_TEXTURE2);
                    glBindTexture(GL_TEXTURE_2D, m_TextureIds[2]);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width >> 1,
                                 height >> 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, ppPlane[2]);
                    glBindTexture(GL_TEXTURE_2D, GL_NONE);
                }
                break;
            default:
                break;
        }
    }
    // Use the program object
    glUseProgram(m_ProgramObj);
    glBindVertexArray(m_VaoId);
    glm::mat4 Model = glm::mat4(1.0f);
    auto radiansZ = -static_cast<float>(MATH_PI / 180.0f * rotationDegrees);
    Model = glm::rotate(Model, radiansZ, glm::vec3(0.0f, 0.0f, 1.0f));
    GLUtils::setMat4(m_ProgramObj, "u_MVPMatrix", m_MVPMatrix * Model);

    for (int i = 0; i < TEXTURE_NUM; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_TextureIds[i]);
        char samplerName[64] = {0};
        sprintf(samplerName, "s_texture%d", i);
        GLUtils::setInt(m_ProgramObj, samplerName, i);
    }

    float offset = (sin(m_FrameIndex * MATH_PI / 40) + 1.0) / 2.0f;
    GLUtils::setFloat(m_ProgramObj, "u_Offset", offset);
    GLUtils::setVec2(m_ProgramObj, "u_TexSize", vec2(width, height));
    GLUtils::setInt(m_ProgramObj, "u_nImgType", format);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *) 0);

}