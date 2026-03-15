#include <stdlib.h> // exit
#include <unistd.h> // getopt
#include <error.h>

#include "myimgui.hpp"
#include "imgui_elements.h"

#include "testjack.hpp"

int main(int argc, char *argv[])
{
	unsigned frate = 0;

	int opt;
	int vsync = 1;
	char *title = (char*)"Dear jackd router";
	static char info[1024*4];
	
	const char* config_path = "config.conf";
	const char* wav_prefix = "filter/filter-center1-soft";

	while ((opt = getopt(argc, argv, "st:c:a:")) != -1) {
		switch (opt) {
		case 's':
			vsync = 0;
			break;
		case 't':
			title = optarg;
			break;
		case 'c':
			config_path = optarg;
			break;
		case 'a':
			wav_prefix = optarg;
			break;
		default:
			fprintf(stderr, "Usage: %s [-s] [-t title] [-c config.conf] [-a prefix]\n\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	if (int ret = mygui(title, vsync))
		return ret;

	//init_shaders();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable( GL_BLEND );

	ImVec4 clear_color = ImVec4(0, 0, 0, 1.00f);

	get_window_size();
	TestJack tj(8, 8, config_path, wav_prefix);
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
		char buf[32];
		for (int i=0; i<rows; i++) {
			ImGui::BeginGroup();
			for (int j=0; j<columns; j++) {
				sprintf(buf, "%dx%d", i, j);
				bool light = tj.is_current_in(i,j);
				if (light)
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.3f, 0.4f, 1.0f});
				if (ImGui::Button(buf, ImVec2(32,32))) {
					tj.set_current_in(i, j);
				}
				if (light)
					ImGui::PopStyleColor(1);
				ImGui::SameLine();
			}
			ImGui::EndGroup();
		}
		ImGui::EndGroup();
		ImGui::BeginGroup();
		for (int s = 0; s < 1; s++) {
			sprintf(buf, "%d", s);
			bool light = tj.is_special(s);
			if (light)
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.3f, 0.8f, 0.4f, 1.0f});
			if (ImGui::Button(buf, ImVec2(32,16))) {
				if (!light)
					tj.set_special(s);
				else
					tj.set_special(-1);
			}
			if (light)
				ImGui::PopStyleColor(1);
			ImGui::SameLine();
		}
		ImGui::EndGroup();

		if (frame_number % (60*6) == 0)
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

		ImGui::Render();
		glViewport(0, 0, sw, sh);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
		++frame_number;
	}

	endgui();
	tj.close();
	printf("exit\n");
	return 0;
}
