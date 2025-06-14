#include <cstdint>
#include <memory>
#include <stdlib.h> // exit
#include <unistd.h> // getopt
#include <error.h>

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1


#include "imgui.h"
#include "imgui_elements.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#include "jackaudioio.hpp"

static GLFWwindow* window;
static int sw = 380, sh = 480;

bool get_window_size() {
	
    int _w, _h;
    glfwGetFramebufferSize(window, &_w, &_h);
    bool resized = (_w != sw || _h != sh);
    sw = _w; sh = _h;

    return resized;
}

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

char info[1024*4];


class TestJack: public JackCpp::AudioIO {
    int m_in = 0;
    const int i0, j0;
	public:
		virtual int audioCallback(jack_nframes_t nframes, 
				audioBufVector inBufs, audioBufVector outBufs) {
			for(unsigned int j = 0; j < nframes; j++) {
				int cur = curi*i0 + curj;
				outBufs[0][j] = inBufs[cur*2][j];
				outBufs[1][j] = inBufs[cur*2+1][j];
			}
			return 0;
		}
		bool is_current_in(int i, int j) {
		    return curi == i && curj == j;
		}
		void set_current_in(int i, int j) {
		    curi = i;
			curj = j;
		}
		TestJack(int i, int j) :
			JackCpp::AudioIO("imjack-test", 0, 0),
			i0(i), j0(j)
			{
				reserveInPorts(i0*j0);
				reserveOutPorts(2);

				addOutPort("out-1");
				addOutPort("out-2");
				for (int ii=0; ii<i0; ii++) {
					for (int jj=0; jj<j0; jj++) {
					    char buf[32];
						sprintf(buf, "in-%d-%d-l", ii, jj);
	                    addInPort(buf);
	                    sprintf(buf, "in-%d-%d-r", ii, jj);
	                    addInPort(buf);
					}
				}
		}
		int curi = 0, curj = 0;
};

int main(int argc, char *argv[])
{
    unsigned frate = 0;

    int opt;
    int vsync = 1;
    int artarg = -1;
    bool shuffle_mode = false;
    char *title = "Dear ImGui jackd router";

    while ((opt = getopt(argc, argv, "sa:St:")) != -1) {
        switch (opt) {
        case 's':
            vsync = 0;
            break;
        case 't':
            title = optarg;
            break;
        default: /* '?' */
            fprintf(stderr, "Usage: %s [-s] [-a art_number] [-S]\n\n"
                "-s           disable vsync\n"
                "-t title     set window title",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }


    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    window = glfwCreateWindow(sw, sh, title, NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(vsync);


    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    //init_shaders();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable( GL_BLEND );

    ImVec4 clear_color = ImVec4(0, 0, 0, 1.00f);

    get_window_size();
    TestJack tj(8, 8);
    tj.start();

    while (!glfwWindowShouldClose(window))
    {
    	static int frame_number = 0;

        glfwPollEvents();

        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();


        ImGui::Begin("Test");

        
        ImGui::BeginGroup();
        int rows = 8, columns = 8;
        for (int i=0; i<rows; i++) {
            ImGui::BeginGroup();
            for (int j=0; j<columns; j++) {
                char buf[32];
                sprintf(buf, "%dx%d", i, j);
                bool light = tj.is_current_in(i,j);
                if (light)
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.3f, 0.4f, 1.0f});
                if (ImGui::Button(buf, ImVec2(32,32))) {
                    tj.set_current_in(i, j);
                }
                if (light)
                    ImGui::PopStyleColor(1);
            }
            ImGui::EndGroup();
            ImGui::SameLine();
        }
        ImGui::EndGroup();

        if (frame_number % 120 == 0)
        {
            char *ti = info;
            ti += cpu_load_text_now(info);
            ti += sprintf(ti, "\n%.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
        }
        ImGui::Text(info);

        ImGui::End();

        if (get_window_size()) {
            //resized(sw, sh);
        }

        //draw();

        ImGui::Render();

        glViewport(0, 0, sw, sh);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();


    glfwDestroyWindow(window);
    glfwTerminate();

    tj.close();
    printf("exit\n");
    return 0;
}
