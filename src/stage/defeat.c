/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "defeat.h"

#include "../archive.h"
#include "../mem.h"
#include "../stage.h"
#include "../random.h"
#include "../timer.h"
#include "../animation.h"

//Defeat background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	Gfx_Tex tex_back0; //back0
	Gfx_Tex tex_back1; //back1
	Gfx_Tex tex_back2; //back2

} Back_Defeat;

void Back_Defeat_DrawBG(StageBack *back)
{
	Back_Defeat *this = (Back_Defeat*)back;

	fixed_t fx, fy;

	//Draw defeatbg
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	RECT back0_src = {  0,  0,255, 81};
	RECT_FIXED back0_dst = {
		FIXED_DEC(-60 - screen.SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(-5,1) - fy,
		FIXED_DEC(636 + screen.SCREEN_WIDEOADD,1),
		FIXED_DEC(201,1)
	};
	
	RECT back1_src = {  0,  0,226,160};
	RECT_FIXED back1_dst = {
		FIXED_DEC((0 - 62) - screen.SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(-38,1) - fy,
		FIXED_DEC(311 + screen.SCREEN_WIDEOADD,1),
		FIXED_DEC(220,1)
	};
	
	RECT back2_src = {  0,  0,224,160};
	RECT_FIXED back2_dst = {
		FIXED_DEC((309 - 62) - screen.SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(-38,1) - fy,
		FIXED_DEC(308 + screen.SCREEN_WIDEOADD,1),
		FIXED_DEC(220,1)
	};
	
	RECT oops_src = { 56,146, 14,  8};
	RECT_FIXED oops_dst = {
		FIXED_DEC(50 - screen.SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(164,1) - fy,
		FIXED_DEC(370 + screen.SCREEN_WIDEOADD,1),
		FIXED_DEC(34,1)
	};
	
	Debug_StageMoveDebug(&back0_dst, 5, fx, fy);
	Debug_StageMoveDebug(&back1_dst, 6, fx, fy);
	Debug_StageMoveDebug(&back2_dst, 7, fx, fy);
	Debug_StageMoveDebug(&oops_dst, 10, fx, fy);
	if (stage.song_step >= 272)
	{
		Stage_DrawTex(&this->tex_back1, &back1_src, &back1_dst, stage.camera.bzoom);
		Stage_DrawTex(&this->tex_back2, &back2_src, &back2_dst, stage.camera.bzoom);
		Stage_DrawTex(&this->tex_back2, &oops_src, &oops_dst, stage.camera.bzoom);
	}
	
	Stage_DrawTex(&this->tex_back0, &back0_src, &back0_dst, stage.camera.bzoom);
	
	//Draw black
	RECT screen_src = {0, 0, screen.SCREEN_WIDTH, screen.SCREEN_HEIGHT};
	Gfx_DrawRect(&screen_src, 0, 0, 0);
}

void Back_Defeat_Free(StageBack *back)
{
	Back_Defeat *this = (Back_Defeat*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Defeat_New(void)
{
	//Allocate background structure
	Back_Defeat *this = (Back_Defeat*)Mem_Alloc(sizeof(Back_Defeat));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Defeat_DrawBG;
	this->back.free = Back_Defeat_Free;

	//Load background textures
	IO_Data arc_back = IO_Read("\\BG\\DEFEAT.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "back0.tim"), 0);
	Gfx_LoadTex(&this->tex_back1, Archive_Find(arc_back, "back1.tim"), 0);
	Gfx_LoadTex(&this->tex_back2, Archive_Find(arc_back, "back2.tim"), 0);
	Mem_Free(arc_back);

	Gfx_SetClear(0, 0, 0);
	
	return (StageBack*)this;
}
