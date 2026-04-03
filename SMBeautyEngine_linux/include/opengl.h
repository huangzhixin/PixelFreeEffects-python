/*
 * Copyright (C) 2019 Trinity. All rights reserved.
 * Copyright (C) 2019 Wang LianJie <wlanjie888@gmail.com>
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

#ifndef TRINITY_OPENGL_H
#define TRINITY_OPENGL_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "opengl_observer.h"

namespace pixelfree {

enum RenderFrame {
    SQUARE = 0,
    FIT,
    CROP
};

enum TextureType {
    TEXTURE_OES = 0,
    TEXTURE_2D
};

class OpenGL {
 public:
    OpenGL();
    OpenGL(int width, int height);
    OpenGL(const char* vertex, const char* fragment);
    OpenGL(int width, int height, const char* vertex, const char* fragment);
    virtual ~OpenGL();
    void InitCoordinates();
    void SetOnGLObserver(OnGLObserver* observer);
    void SetTextureType(TextureType type = TEXTURE_2D);
    void Init(const char* vertex, const char* fragment);
    void SetFrame(int source_width, int source_height, int target_width, int target_height, RenderFrame frame_type = FIT);
    void SetOutput(int width, int height);
    void SetInt(const char* name, int value);
    void SetFloat(const char* name, float value);
    void SetUniform4f(const char* name, float v0, float v1, float v2, float v3);
    void SetFloatVec2(const char* name, int size, const GLfloat* value);
    void SetFloatVec3(const char* name, int size, const GLfloat* value);
    void SetFloatVec4(const char* name, int size, const GLfloat* value);
    void SetUniformMatrix3f(const char* name, int size, const GLfloat* matrix);
    void SetUniformMatrix4f(const char* name, int size, const GLfloat* matrix);

    void ActiveProgram();
    void ProcessImage(GLuint texture_id);
    void ProcessImage(GLuint texture_id, GLfloat* texture_matrix);
    void ProcessImage(GLuint texture_id, const GLfloat* vertex_coordinate, const GLfloat* texture_coordinate);
    void ProcessImage(GLuint texture_id, const GLfloat* vertex_coordinate, const GLfloat* texture_coordinate, GLfloat* texture_matrix);

 protected:
    virtual void RunOnDrawTasks();
    virtual void OnDrawArrays();

 private:
    void CreateProgram(const char* vertex, const char* fragment);
    void CompileShader(const char* shader_string, GLuint shader);
    void Link();

 private:
    OnGLObserver* gl_observer_;
    TextureType type_;
    GLuint program_;
    int width_;
    int height_;
    GLfloat* default_vertex_coordinates_;
    GLfloat* default_texture_coordinates_;
};

}  // namespace pixelfree

#endif  // TRINITY_OPENGL_H
