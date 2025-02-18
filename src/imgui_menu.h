#ifndef __IMGUI_MENU_H__
#define __IMGUI_MENU_H__

#include "AudioSourcePA.h"

void draw_window();
void draw_perf(int freshSamples);
void draw_audiosource(AudioSourcePA& audioSource);

#endif // __IMGUI_MENU_H__