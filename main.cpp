#include <stdlib.h> // exit
#include <unistd.h> // getopt
#include <error.h>

#include "myimgui.hpp"
#include "imgui_elements.h"

#include "jackaudioio.hpp"


class TestJack: public JackCpp::AudioIO {
	int m_in = 0;
	const int i0, j0;
	public:
		virtual int audioCallback(jack_nframes_t nframes, 
				audioBufVector inBufs, audioBufVector outBufs) {
			for(unsigned int j = 0; j < nframes; j++) {
				if (special != -1) {
					int last = i0*j0*2;
					outBufs[0][j] = inBufs[last+special+0][j];
					outBufs[1][j] = inBufs[last+special+1][j];
				}
				else {
					int cur = curi*i0 + curj;
					outBufs[0][j] = inBufs[cur*2][j];
					outBufs[1][j] = inBufs[cur*2+1][j];
				}
			}
			return 0;
		}
		bool is_special(int s) {
			return special == s;
		}
		void set_special(int s) {
			special = s;
		}
		bool is_current_in(int i, int j) {
			return curi == i && curj == j;
		}
		void set_current_in(int i, int j) {
			curi = i;
			curj = j;
		}
		TestJack(int i, int j) :i
			JackCpp::AudioIO("imjack-test", 0, 0),
			i0(i), j0(j)
			{
				reserveInPorts(i0*j0*2+2);
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
				// nb. left right naming
				addInPort("special-0-1");
				addInPort("special-0-2");
		}
		int curi = 0, curj = 0;
		int special = -1;
};

int main(int argc, char *argv[])
{
	unsigned frate = 0;

	int opt;
	int vsync = 1;
	int artarg = -1;
	bool shuffle_mode = false;
	char *title = "Dear ImGui jackd router";
	static char info[1024*4];

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
				"-s		   disable vsync\n"
				"-t title	 set window title",
					argv[0]);
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
