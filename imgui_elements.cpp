#include "imgui.h"
#include "imgui_internal.h" // ImGui::SetItemKeyOwner

#include <sys/resource.h>
#include "timer.h"

int cpu_load_text_now(char * text)
{
    static double up = 0, sp = 0;
    static common::Timer old_t;
    static struct rusage old_usage;
    static double outt = 0, ostt = 0;
    static double ru_maxrss = 0;

    common::Timer t;

    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    double utt = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1e6;
    double stt = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1e6;

    double dt = (t - old_t).seconds();
    up = 100.0 * (utt - outt) / dt;
    sp = 100.0 * (stt - ostt) / dt;

    //printf("%.3f %.3f %.3f %.3f %.3f %.3f %.3f\n", dt,
    //        up, sp, utt, stt, outt, ostt);

    old_t = t;
    outt = utt;
    ostt = stt;

    ru_maxrss = usage.ru_maxrss / 1024;

    return sprintf(text,
        "Usr + Sys = %.2f + %.2f = %.2f\nmaxrss %.2f MB",
        up, sp, up+sp, ru_maxrss);
}

char * cpu_load_text()
{
    static char text[1024*4];
    static int c = 0;
    char *t;

    if (c % 120 == 0) {
        cpu_load_text_now(text);
    }
    c++;

    return text;
}

void cpu_load_gui()
{
    ImGui::Text(cpu_load_text());
}
