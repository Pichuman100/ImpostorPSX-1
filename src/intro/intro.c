#include "intro.h"

#include "../mem.h"
#include "../main.h"
#include "../timer.h"
#include "../io.h"
#include "../gfx.h"
#include "../audio.h"
#include "../pad.h"
#include "../archive.h"
#include "../mutil.h"
#include "../network.h"

#include "../font.h"
#include "../trans.h"
#include "../loadscr.h"

#include "../stage.h"
#include "../menu.h"
#include "../save.h"

Intro intro;

Gfx_Tex tex_intro;

static void Intro_Animation() {
	//FntPrint("%d\n", intro.timer);
	switch(intro.timer)
	{
		case 0:
			intro.logo1.tx = 1600;
			intro.logo1.ty = 1200;
			intro.logo1.x = 1600;
			intro.logo1.y = -1200;
			intro.logo1.w = 126;
			intro.logo1.h = 113;
			break;
		case 100:
			intro.logo1.w = 157;
			intro.logo1.h = 141;
			break;
		case 150:
			intro.logo1.tx = 650;
			break;
		case 220:
			intro.logo2.tx = 2230;
			intro.logo2.ty = 1150;
			intro.logo2.x = 1400;
			intro.logo2.y = 1600;
			intro.logo2.w = 195 * 5;
			intro.logo2.h = 122 * 5;
			break;
		case 300:
			gameloop = GameLoop_Menu;
			Menu_Load(MenuPage_Opening);
	}
	intro.timer++;
}

static void Draw_logo2() {
	RECT logo2_src = {0, 114, 195, 122};
	
	intro.logo2.x = lerp(intro.logo2.x, intro.logo2.tx, FIXED_DEC(1,10));
	intro.logo2.y = lerp(intro.logo2.y, intro.logo2.ty, FIXED_DEC(1,10));
	intro.logo2.w = lerp(intro.logo2.w, 195, FIXED_DEC(1,10));
	intro.logo2.h = lerp(intro.logo2.h, 122, FIXED_DEC(1,10));
	
	RECT logo2_dst = {
		intro.logo2.x / 10 - (intro.logo2.w / 2),
		intro.logo2.y / 10 - (intro.logo2.h / 2),
		intro.logo2.w,
		intro.logo2.h,
	};
	
	Gfx_DrawTex(&tex_intro, &logo2_src, &logo2_dst);
}

void Intro_Load() {
	Gfx_SetClear(0, 0, 0);
	
	Gfx_LoadTex(&tex_intro, IO_Read("\\INTRO\\INTRO.TIM;1"), GFX_LOADTEX_FREE);
	
	intro.logo1.y = -1200;
}

void Intro_Tick() {
	Intro_Animation();
	
	RECT logo1_src = {0, 0, 126, 113};
	
	if(intro.timer > 220)
		Draw_logo2();

	intro.logo1.x = lerp(intro.logo1.x, intro.logo1.tx, FIXED_DEC(1,10));
	intro.logo1.y = lerp(intro.logo1.y, intro.logo1.ty, FIXED_DEC(1,10));
	intro.logo1.w = lerp(intro.logo1.w, 126, FIXED_DEC(1,10));
	intro.logo1.h = lerp(intro.logo1.h, 113, FIXED_DEC(1,10));
	
	RECT logo1_dst = {
		intro.logo1.x / 10 - (intro.logo1.w / 2),
		intro.logo1.y / 10 - (intro.logo1.h / 2),
		intro.logo1.w,
		intro.logo1.h,
	};
	
	Gfx_DrawTex(&tex_intro, &logo1_src, &logo1_dst);
}

void Intro_Unload() {
	
}