#include <stdint.h>

#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../../SMBeautyEngine_linux/pixelfreeLib/Include/pixelFree_c.hpp"
#include "../../SMBeautyEngine_linux/include/opengl.h"

namespace {

thread_local std::string g_last_error;

void SetLastErrorMessage(const std::string& message) {
    g_last_error = message;
}

bool ReadBinaryFile(const char* path, std::vector<char>* out) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }

    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    if (size <= 0) {
        return false;
    }

    out->resize(static_cast<size_t>(size));
    return file.read(out->data(), size).good();
}

struct PixelFreeBridge {
    GLFWwindow* window = nullptr;
    PFPixelFree* handle = nullptr;
    std::unordered_map<int, std::string> string_params;
    std::unique_ptr<pixelfree::OpenGL> renderer;
};

void ClearGLErrors() {
    while (glGetError() != GL_NO_ERROR) {
    }
}

const char* DEFAULT_VERTEX_SHADER = R"(
attribute vec4 position;
attribute vec4 inputTextureCoordinate;
varying vec2 textureCoordinate;
void main() {
    gl_Position = position;
    textureCoordinate = inputTextureCoordinate.xy;
}
)";

const char* DEFAULT_FRAGMENT_SHADER = R"(
varying vec2 textureCoordinate;
uniform sampler2D inputImageTexture;
void main() {
    gl_FragColor = texture2D(inputImageTexture, textureCoordinate);
}
)";

GLuint CreateTextureFromRGBA(const uint8_t* rgba_data, int width, int height) {
    GLuint texture_id = 0;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texture_id;
}

bool RenderTextureToRGBA(
    pixelfree::OpenGL* renderer,
    GLuint source_texture,
    uint8_t* rgba_data,
    int width,
    int height
) {
    ClearGLErrors();
    if (!glIsTexture(source_texture)) {
        std::ostringstream oss;
        oss << "RenderTextureToRGBA invalid texture id: " << source_texture;
        SetLastErrorMessage(oss.str());
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, source_texture);
    GLenum bind_error = glGetError();
    if (bind_error != GL_NO_ERROR) {
        std::ostringstream oss;
        oss << "RenderTextureToRGBA bind source texture error: 0x" << std::hex
            << bind_error << " texture=" << source_texture;
        SetLastErrorMessage(oss.str());
        glBindTexture(GL_TEXTURE_2D, 0);
        return false;
    }
    GLint source_width = 0;
    GLint source_height = 0;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &source_width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &source_height);
    GLenum size_error = glGetError();
    glBindTexture(GL_TEXTURE_2D, 0);
    if (size_error != GL_NO_ERROR) {
        std::ostringstream oss;
        oss << "RenderTextureToRGBA query texture size error: 0x" << std::hex
            << size_error << " texture=" << source_texture;
        SetLastErrorMessage(oss.str());
        return false;
    }
    if (source_width <= 0 || source_height <= 0) {
        std::ostringstream oss;
        oss << "RenderTextureToRGBA invalid source texture size: "
            << source_width << "x" << source_height
            << " texture=" << source_texture;
        SetLastErrorMessage(oss.str());
        return false;
    }

    GLuint target_texture = 0;
    glGenTextures(1, &target_texture);
    glBindTexture(GL_TEXTURE_2D, target_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    GLuint framebuffer = 0;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target_texture, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::ostringstream oss;
        oss << "Framebuffer incomplete: 0x" << std::hex << status;
        SetLastErrorMessage(oss.str());
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &framebuffer);
        glDeleteTextures(1, &target_texture);
        return false;
    }

    renderer->SetOutput(width, height);
    renderer->ActiveProgram();
    ClearGLErrors();
    renderer->ProcessImage(source_texture);
    GLenum render_error = glGetError();
    if (render_error != GL_NO_ERROR) {
        std::ostringstream oss;
        oss << "RenderTextureToRGBA render error: 0x" << std::hex << render_error;
        SetLastErrorMessage(oss.str());
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &framebuffer);
        glDeleteTextures(1, &target_texture);
        return false;
    }

    glViewport(0, 0, width, height);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    ClearGLErrors();
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, rgba_data);
    GLenum read_error = glGetError();
    bool ok = read_error == GL_NO_ERROR;
    if (!ok) {
        std::ostringstream oss;
        oss << "RenderTextureToRGBA read error: 0x" << std::hex << read_error;
        SetLastErrorMessage(oss.str());
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteTextures(1, &target_texture);
    return ok;
}

}  // namespace

extern "C" __attribute__((visibility("default"))) const char* PFPy_GetLastError() {
    return g_last_error.c_str();
}

extern "C" __attribute__((visibility("default"))) void* PFPy_CreateEngine(const char* auth_path, const char* filter_path) {
    SetLastErrorMessage("");

    auto bridge = std::make_unique<PixelFreeBridge>();

    if (!glfwInit()) {
        SetLastErrorMessage("glfwInit failed");
        return nullptr;
    }

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    bridge->window = glfwCreateWindow(16, 16, "PixelFreeHidden", nullptr, nullptr);
    if (!bridge->window) {
        SetLastErrorMessage("glfwCreateWindow failed");
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(bridge->window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        SetLastErrorMessage("gladLoadGLLoader failed");
        glfwDestroyWindow(bridge->window);
        glfwTerminate();
        return nullptr;
    }

    bridge->handle = PF_NewPixelFree();
    if (bridge->handle == nullptr) {
        SetLastErrorMessage("PF_NewPixelFree failed");
        glfwDestroyWindow(bridge->window);
        glfwTerminate();
        return nullptr;
    }

    std::vector<char> auth_buffer;
    std::vector<char> filter_buffer;
    if (!ReadBinaryFile(auth_path, &auth_buffer)) {
        SetLastErrorMessage(std::string("Failed to read auth file: ") + auth_path);
        PF_DeletePixelFree(bridge->handle);
        glfwDestroyWindow(bridge->window);
        glfwTerminate();
        return nullptr;
    }
    if (!ReadBinaryFile(filter_path, &filter_buffer)) {
        SetLastErrorMessage(std::string("Failed to read filter file: ") + filter_path);
        PF_DeletePixelFree(bridge->handle);
        glfwDestroyWindow(bridge->window);
        glfwTerminate();
        return nullptr;
    }

    PF_createBeautyItemFormBundle(bridge->handle, auth_buffer.data(), static_cast<int>(auth_buffer.size()), PFSrcTypeAuthFile);
    PF_createBeautyItemFormBundle(bridge->handle, filter_buffer.data(), static_cast<int>(filter_buffer.size()), PFSrcTypeFilter);
    bridge->renderer = std::make_unique<pixelfree::OpenGL>(16, 16, DEFAULT_VERTEX_SHADER, DEFAULT_FRAGMENT_SHADER);

    return bridge.release();
}

extern "C" __attribute__((visibility("default"))) int PFPy_SetFloatParam(void* engine, int key, float value) {
    SetLastErrorMessage("");
    auto* bridge = reinterpret_cast<PixelFreeBridge*>(engine);
    if (bridge == nullptr || bridge->handle == nullptr) {
        SetLastErrorMessage("Engine is null");
        return 0;
    }

    glfwMakeContextCurrent(bridge->window);
    PF_pixelFreeSetBeautyFilterParam(bridge->handle, key, &value);
    return 1;
}

extern "C" __attribute__((visibility("default"))) int PFPy_SetIntParam(void* engine, int key, int value) {
    SetLastErrorMessage("");
    auto* bridge = reinterpret_cast<PixelFreeBridge*>(engine);
    if (bridge == nullptr || bridge->handle == nullptr) {
        SetLastErrorMessage("Engine is null");
        return 0;
    }

    glfwMakeContextCurrent(bridge->window);
    PF_pixelFreeSetBeautyFilterParam(bridge->handle, key, &value);
    return 1;
}

extern "C" __attribute__((visibility("default"))) int PFPy_SetStringParam(void* engine, int key, const char* value) {
    SetLastErrorMessage("");
    auto* bridge = reinterpret_cast<PixelFreeBridge*>(engine);
    if (bridge == nullptr || bridge->handle == nullptr) {
        SetLastErrorMessage("Engine is null");
        return 0;
    }

    glfwMakeContextCurrent(bridge->window);
    bridge->string_params[key] = value ? value : "";
    PF_pixelFreeSetBeautyFilterParam(
        bridge->handle,
        key,
        (void*)bridge->string_params[key].c_str()
    );
    return 1;
}

extern "C" __attribute__((visibility("default"))) int PFPy_ProcessImageRGBA(
    void* engine,
    uint8_t* rgba_data,
    int width,
    int height,
    int stride,
    int rotation_mode
) {
    SetLastErrorMessage("");
    auto* bridge = reinterpret_cast<PixelFreeBridge*>(engine);
    if (bridge == nullptr || bridge->handle == nullptr) {
        SetLastErrorMessage("Engine is null");
        return 0;
    }
    if (rgba_data == nullptr) {
        SetLastErrorMessage("rgba_data is null");
        return 0;
    }

    glfwMakeContextCurrent(bridge->window);
    ClearGLErrors();
    glfwSetWindowSize(bridge->window, width, height);
    GLuint input_texture = CreateTextureFromRGBA(rgba_data, width, height);
    if (input_texture == 0) {
        SetLastErrorMessage("Failed to create input texture");
        return 0;
    }

    PFImageInput image = {};
    image.textureID = static_cast<int>(input_texture);
    image.wigth = width;
    image.height = height;
    image.p_data0 = rgba_data;
    image.p_data1 = nullptr;
    image.p_data2 = nullptr;
    image.stride_0 = stride;
    image.stride_1 = 0;
    image.stride_2 = 0;
    image.format = PFFORMAT_IMAGE_TEXTURE;
    image.rotationMode = static_cast<PFRotationMode>(rotation_mode);

    int output_texture = PF_processWithBuffer(bridge->handle, image);
    ClearGLErrors();
    if (output_texture <= 0) {
        glDeleteTextures(1, &input_texture);
        SetLastErrorMessage("PF_processWithBuffer failed");
        return 0;
    }

    if (!RenderTextureToRGBA(bridge->renderer.get(), static_cast<GLuint>(output_texture), rgba_data, width, height)) {
        glDeleteTextures(1, &input_texture);
        if (g_last_error.empty()) {
            SetLastErrorMessage("Failed to read rendered framebuffer");
        }
        return 0;
    }

    glDeleteTextures(1, &input_texture);
    glFinish();
    return 1;
}

extern "C" __attribute__((visibility("default"))) void PFPy_DestroyEngine(void* engine) {
    auto* bridge = reinterpret_cast<PixelFreeBridge*>(engine);
    if (bridge == nullptr) {
        return;
    }

    if (bridge->window) {
        glfwMakeContextCurrent(bridge->window);
    }
    if (bridge->handle) {
        PF_DeletePixelFree(bridge->handle);
    }
    if (bridge->window) {
        glfwDestroyWindow(bridge->window);
    }
    glfwTerminate();
    delete bridge;
}
