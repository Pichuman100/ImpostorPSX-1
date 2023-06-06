/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "skeld.h"

#include "../archive.h"
#include "../mem.h"
#include "../stage.h"
#include "../random.h"
#include "../timer.h"
#include "../animation.h"

int bgswitch = 0;

//Skeld background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	Gfx_Tex tex_back0; //back0
	Gfx_Tex tex_back1; //back1
	Gfx_Tex tex_back2; //back2
	Gfx_Tex tex_back3; //back3
	Gfx_Tex tex_back4; //back4
	Gfx_Tex tex_back5; //back5
	Gfx_Tex tex_back6; //back6

} Back_Skeld;

void Back_Skeld_DrawBG(StageBack *back)
{
	Back_Skeld *this = (Back_Skeld*)back;

	fixed_t fx, fy;

	//Draw skeld
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	RECT back0_src = {  0,  0,255,255};
	RECT_FIXED back0_dst = {
		FIXED_DEC(0 - screen.SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(0,1) - fy,
		FIXED_DEC(484 + screen.SCREEN_WIDEOADD,1),
		FIXED_DEC(484,1)
	};
	
	RECT back1_src = {  0,  0,173,255};
	RECT_FIXED back1_dst = {
		FIXED_DEC(482 - screen.SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(0,1) - fy,
		FIXED_DEC(328 + screen.SCREEN_WIDEOADD,1),
		FIXED_DEC(484,1)
	};
	
	RECT back4_src = {  0,  0,255,136};
	RECT_FIXED back4_dst = {
		FIXED_DEC(0 - screen.SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(0,1) - fy,
		FIXED_DEC(705 + screen.SCREEN_WIDEOADD,1),
		FIXED_DEC(377,1)
	};
	
	RECT back5_src = {  0,  0,255, 67};
	RECT_FIXED back5_dst = {
		FIXED_DEC(0 - screen.SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(0,1) - fy,
		FIXED_DEC(1645 + screen.SCREEN_WIDEOADD,1),
		FIXED_DEC(433,1)
	};
	RECT back51_src = {  0,  0, 64, 67};
	RECT_FIXED back51_dst = {
		FIXED_DEC(0 - screen.SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(0,1) - fy,
		FIXED_DEC(413 + screen.SCREEN_WIDEOADD,1),
		FIXED_DEC(433,1)
	};
	RECT back52_src = { 63,  0, 64, 67};
	RECT_FIXED back52_dst = {
		FIXED_DEC((413-7)*1 - screen.SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(0,1) - fy,
		FIXED_DEC(413 + screen.SCREEN_WIDEOADD,1),
		FIXED_DEC(433,1)
	};
	RECT back53_src = {127,  0, 64, 67};
	RECT_FIXED back53_dst = {
		FIXED_DEC((413-7)*2 - screen.SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(0,1) - fy,
		FIXED_DEC(413 + screen.SCREEN_WIDEOADD,1),
		FIXED_DEC(433,1)
	};
	RECT back54_src = {191,  0, 64, 67};
	RECT_FIXED back54_dst = {
		FIXED_DEC((413-7)*3 - screen.SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(0,1) - fy,
		FIXED_DEC(413 + screen.SCREEN_WIDEOADD,1),
		FIXED_DEC(433,1)
	};
	
	RECT back6_src = {  0,  0,255, 68};
	RECT_FIXED back6_dst = {
		FIXED_DEC(175 - screen.SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(27,1) - fy,
		FIXED_DEC(486 + screen.SCREEN_WIDEOADD,1),
		FIXED_DEC(130,1)
	};
	
	Debug_StageMoveDebug(&back0_dst, 5, fx, fy);
	Debug_StageMoveDebug(&back1_dst, 6, fx, fy);
	Debug_StageMoveDebug(&back4_dst, 7, fx, fy);
	Debug_StageMoveDebug(&back5_dst, 8, fx, fy);
	Debug_StageMoveDebug(&back6_dst, 9, fx, fy);
	if (bgswitch == 0)
	{
		Stage_DrawTex(&this->tex_back0, &back0_src, &back0_dst, stage.camera.bzoom);
		Stage_DrawTex(&this->tex_back1, &back1_src, &back1_dst, stage.camera.bzoom);
	}
	else if (bgswitch == 1)
	{
		Stage_DrawTex(&this->tex_back2, &back0_src, &back0_dst, stage.camera.bzoom);
		Stage_DrawTex(&this->tex_back3, &back1_src, &back1_dst, stage.camera.bzoom);
	}
	else if (bgswitch == 2)
	{
		Stage_DrawTex(&this->tex_back4, &back4_src, &back4_dst, stage.camera.bzoom);
	}
	else if (bgswitch == 3)
	{
		Stage_DrawTex(&this->tex_back6, &back6_src, &back6_dst, stage.camera.bzoom);
		Stage_DrawTex(&this->tex_back5, &back51_src, &back51_dst, stage.camera.bzoom);
		Stage_DrawTex(&this->tex_back5, &back52_src, &back52_dst, stage.camera.bzoom);
		Stage_DrawTex(&this->tex_back5, &back53_src, &back53_dst, stage.camera.bzoom);
		Stage_DrawTex(&this->tex_back5, &back54_src, &back54_dst, stage.camera.bzoom);
	}
	
	RECT screen_src = {0, 0, screen.SCREEN_WIDTH, screen.SCREEN_HEIGHT};
	Gfx_DrawRect(&screen_src, 0, 0, 0);
	
	//bg switch
	if (stage.song_step == -29) //normal
		bgswitch = 0;
	if (stage.song_step == 638) //red
		bgswitch = 1;
	if (stage.song_step == 896) //normal
		bgswitch = 0;
	if (stage.song_step == 1174) //green
		bgswitch = 2;
	if (stage.song_step == 1500) //normal
		bgswitch = 0;
	if (stage.song_step == 1972) //black
		bgswitch = 3;
	if (stage.song_step == 2270) //normal
		bgswitch = 0;
	if (stage.song_step == 2816) //red
		bgswitch = 1;
	if (stage.song_step == 2878) //green
		bgswitch = 2;
	if (stage.song_step == 3072) //black
		bgswitch = 3;
	if (stage.song_step == 3198) //red
		bgswitch = 1;
	if (stage.song_step == 3280) //green
		bgswitch = 2;
	if (stage.song_step == 3296) //black
		bgswitch = 3;
	if (stage.song_step == 3328) //normal
		bgswitch = 0;
}

void Back_Skeld_Free(StageBack *back)
{
	Back_Skeld *this = (Back_Skeld*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Skeld_New(void)
{
	//Allocate background structure
	Back_Skeld *this = (Back_Skeld*)Mem_Alloc(sizeof(Back_Skeld));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Skeld_DrawBG;
	this->back.free = Back_Skeld_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\BG\\SKELD.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "back0.tim"), 0);
	Gfx_LoadTex(&this->tex_back1, Archive_Find(arc_back, "back1.tim"), 0);
	Gfx_LoadTex(&this->tex_back2, Archive_Find(arc_back, "back2.tim"), 0);
	Gfx_LoadTex(&this->tex_back3, Archive_Find(arc_back, "back3.tim"), 0);
	Gfx_LoadTex(&this->tex_back4, Archive_Find(arc_back, "back4.tim"), 0);
	Gfx_LoadTex(&this->tex_back5, Archive_Find(arc_back, "back5.tim"), 0);
	Gfx_LoadTex(&this->tex_back6, Archive_Find(arc_back, "back6.tim"), 0);
	Mem_Free(arc_back);
	
	bgswitch = 0;
	
	return (StageBack*)this;
}