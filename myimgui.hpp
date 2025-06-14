#pragma once


#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers


extern GLFWwindow* window;
extern int sw, sh;

bool get_window_size();
int mygui(char *title, int vsync);
void endgui();