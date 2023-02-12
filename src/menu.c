/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "menu.h"

#include "mem.h"
#include "main.h"
#include "timer.h"
#include "io.h"
#include "gfx.h"
#include "audio.h"
#include "pad.h"
#include "archive.h"
#include "mutil.h"
#include "network.h"

#include "font.h"
#include "trans.h"
#include "loadscr.h"

#include "stage.h"
#include "save.h"
#include "character/gf.h"

#include "stdlib.h"

static u32 Sounds[8];
static char scoredisp[30];
//Menu messages
static const char *funny_messages[][2] = {
	{"TRUST THE", "FUCKING PLAN"},
	{"IM", "ABOUT TO BLOW"},
	{"JANUARY ONETH", "ALWAYS ON TIME"},
	{"SUPER", "DUPER"},
	{"CHUNUGS", "MASTA"},
	{"BREEZY", "MELEE"},
	{"HI", "GUYS"},
	{"MONOTONE", "ATTACK"},
	{"WE LOVE", "CHUNGUSMASTAH"},
	{"VS", "NEWGROUND"},
	{"FPS PLUS", "COMING IN CLUTCH"},
	{"FUCK YOU", "FUCKIN DICK"},
	{"FABS FINALLY GOT", "SLEEP"},
	{"MERRY IMPOSTOR", "AND A HAPPY NEW YEAR"},
	{"CALL ME MONO", "IM ON THE ATTACK"},
	{"MOD HAS BUGS", "FUCK YOU"},
	{"SASTER", "OH MY GOD SASTER"},
	{"GOD FEARS", "TAN IMPOSTOR"},
	{"THE DOC", "IS OUT"},
	{"CALL ME", "ETHAN"},
	{"HAPPY BIRTHDAY", "RED MUNGUS"},
	{"SHOUTOUT TO", "SILLY SQUAD"},
	{"IM HUMBAY", "IN THE MOD"},
	{"MONOTONE ATTACK", "PEAK SONG"},
	{"RESPECT THE TEAM", "WE WORKED HARD"},
	{"DONT TRACE", "PLEASE GUYS"},
	{"IM GOING TO KILL YOU", "IM BEING GENUINE"},
	{"NICKT", "BEAT SUSSUS MOOGUS"},
	{"PLAY VOTING TIME", "DONT LISTEN TO ETHAN"},
	{"IM CVAL", "IM FABS"},
	{"I LEAKED THE MOD", "transfer.sh/urWK1P/impostor.rar"},
	{"DRIPPYPOP OLD", "MY GOAT"},
};


//Menu state
static struct
{
	//Menu state
	u8 page, next_page;
	boolean page_swap;
	u8 select, next_select;
	boolean idk;
	
	fixed_t scroll;
	fixed_t trans_time;
	
	//Page specific state
	union
	{
		struct
		{
			u8 funny_message;
		} opening;
		struct
		{
			fixed_t logo_bump;
			fixed_t fade, fadespd;
		} title;
		struct
		{
			fixed_t fade, fadespd;
		} story;
		struct
		{
			fixed_t back_r, back_g, back_b;
		} freeplay;
	} page_state;
	
	union
	{
		struct
		{
			u8 id, diff;
			boolean story;
			boolean last;
		} stage;
	} page_param;
	
	//Menu assets
	Gfx_Tex tex_back, tex_ng, tex_story, tex_title, tex_defeat;
	FontData font_bold, font_arial, font_cdr;
	
	Character *gf; //Title Girlfriend
} menu;

//Internal menu functions
char menu_text_buffer[0x100];

static const char *Menu_LowerIf(const char *text, boolean lower)
{
	//Copy text
	char *dstp = menu_text_buffer;
	if (lower)
	{
		for (const char *srcp = text; *srcp != '\0'; srcp++)
		{
			if (*srcp >= 'A' && *srcp <= 'Z')
				*dstp++ = *srcp | 0x20;
			else
				*dstp++ = *srcp;
		}
	}
	else
	{
		for (const char *srcp = text; *srcp != '\0'; srcp++)
		{
			if (*srcp >= 'a' && *srcp <= 'z')
				*dstp++ = *srcp & ~0x20;
			else
				*dstp++ = *srcp;
		}
	}
	
	//Terminate text
	*dstp++ = '\0';
	return menu_text_buffer;
}

static void Menu_DrawBack(boolean flash, s32 scroll, u8 r0, u8 g0, u8 b0, u8 r1, u8 g1, u8 b1)
{
	RECT back_src = {0, 0, 255, 255};
	RECT back_dst = {0, -scroll - screen.SCREEN_WIDEADD2, screen.SCREEN_WIDTH, screen.SCREEN_WIDTH * 4 / 5};
	
	if (flash || (animf_count & 4) == 0)
		Gfx_DrawTexCol(&menu.tex_back, &back_src, &back_dst, r0, g0, b0);
	else
		Gfx_DrawTexCol(&menu.tex_back, &back_src, &back_dst, r1, g1, b1);
}

static int increase_Story(int length, int thesong)
{
	int result = 0;
	int testresult = 0;

	for (int i = 0; i < length; i++)
	{
		testresult = stage.prefs.savescore[thesong + i][menu.page_param.stage.diff];

		if (testresult == 0)
			return 0;

		result += stage.prefs.savescore[thesong + i][menu.page_param.stage.diff];
	}

	return result * 10;
}

static void Menu_DifficultySelector(s32 x, s32 y)
{
	//Change difficulty
	if (menu.next_page == menu.page && Trans_Idle())
	{
		if (pad_state.press & PAD_LEFT)
		{
			if (menu.page_param.stage.diff > StageDiff_Easy)
				menu.page_param.stage.diff--;
			else
				menu.page_param.stage.diff = StageDiff_Hard;
		}
		if (pad_state.press & PAD_RIGHT)
		{
			if (menu.page_param.stage.diff < StageDiff_Hard)
				menu.page_param.stage.diff++;
			else
				menu.page_param.stage.diff = StageDiff_Easy;
		}
	}
	
	//Draw difficulty arrows
	static const RECT arrow_src[2][2] = {
		{{224, 64, 16, 32}, {224, 96, 16, 32}}, //left
		{{240, 64, 16, 32}, {240, 96, 16, 32}}, //right
	};
	
	Gfx_BlitTex(&menu.tex_story, &arrow_src[0][(pad_state.held & PAD_LEFT) != 0], x - 40 - 16, y - 16);
	Gfx_BlitTex(&menu.tex_story, &arrow_src[1][(pad_state.held & PAD_RIGHT) != 0], x + 40, y - 16);
	
	//Draw difficulty
	static const RECT diff_srcs[] = {
		{  0, 96, 64, 18},
		{ 64, 96, 80, 18},
		{144, 96, 64, 18},
	};
	
	const RECT *diff_src = &diff_srcs[menu.page_param.stage.diff];
	Gfx_BlitTex(&menu.tex_story, diff_src, x - (diff_src->w >> 1), y - 9 + ((pad_state.press & (PAD_LEFT | PAD_RIGHT)) != 0));
}

static void Menu_DrawWeek(const char *week, s32 x, s32 y)
{
	//Draw label
	if (week == NULL)
	{
		//Tutorial
		RECT label_src = {0, 0, 112, 32};
		Gfx_BlitTex(&menu.tex_story, &label_src, x, y);
	}
	else
	{
		//Week
		RECT label_src = {0, 32, 80, 32};
		Gfx_BlitTex(&menu.tex_story, &label_src, x, y);
		
		//Number
		x += 80;
		for (; *week != '\0'; week++)
		{
			//Draw number
			u8 i = *week - '0';
			
			RECT num_src = {128 + ((i & 3) << 5), ((i >> 2) << 5), 32, 32};
			Gfx_BlitTex(&menu.tex_story, &num_src, x, y);
			x += 32;
		}
	}
}

static void Menu_DrawDefeat(void)
{
	//Draw number
	RECT zero_src = {0, 67, 10, 15};
	RECT zero_dst = {5 + 15, 42, 16, 24};
	RECT one_src = {11, 67, 8, 15};
	RECT one_dst = {5 + 15, 42, 13, 24};
	RECT two_src = {20, 67, 10, 15};
	RECT two_dst = {5 + 15, 42, 16, 24};
	RECT three_src = {31, 67, 10, 15};
	RECT three_dst = {5 + 15, 42, 16, 24};
	RECT four_src = {43, 67, 10, 15};
	RECT four_dst = {5 + 15, 42, 16, 24};
	RECT five_src = {54, 67, 10, 15};
	RECT five_dst = {5 + 15, 42, 16, 24};
	if (menu.select == 5)
		Gfx_DrawTex(&menu.tex_defeat, &zero_src, &zero_dst);
	else if (menu.select == 4)
		Gfx_DrawTex(&menu.tex_defeat, &one_src, &one_dst);
	else if (menu.select == 3)
		Gfx_DrawTex(&menu.tex_defeat, &two_src, &two_dst);
	else if (menu.select == 2)
		Gfx_DrawTex(&menu.tex_defeat, &three_src, &three_dst);
	else if (menu.select == 1)
		Gfx_DrawTex(&menu.tex_defeat, &four_src, &four_dst);
	else if (menu.select == 0)
		Gfx_DrawTex(&menu.tex_defeat, &five_src, &five_dst);
	
	//Draw text
	RECT combo_src = {0, 35, 55, 15};
	RECT combo_dst = {76 + 15, 42, 88, 24};
	RECT break_src = {0, 51, 66, 15};
	RECT break_dst = {183 + 15, 42, 107, 24};
	RECT slash_src = {65, 67, 10, 15};
	RECT slash_dst = {23 + 15, 42, 16, 24};
	RECT five2_dst = {41 + 15, 42, 16, 24};
	Gfx_DrawTex(&menu.tex_defeat, &combo_src, &combo_dst);
	Gfx_DrawTex(&menu.tex_defeat, &break_src, &break_dst);
	Gfx_DrawTex(&menu.tex_defeat, &slash_src, &slash_dst);
	Gfx_DrawTex(&menu.tex_defeat, &five_src, &five2_dst);
	
	//Draw dead people
	RECT ghost_src = {0, 0, 28, 30};
	RECT ghost_dst = {0 + 41, 150, 28, 30};
	RECT ghost2_dst = {42 + 41, 150, 28, 30};
	RECT dead_src = {29, 0, 28, 30};
	RECT dead_dst = {84 + 41, 150, 28, 30};
	RECT dead2_dst = {126 + 41, 150, 28, 30};
	RECT bone_src = {58, 0, 28, 30};
	RECT bone_dst = {168 + 41, 150, 28, 30};
	RECT bone2_dst = {210 + 41, 150, 28, 30};
	Gfx_DrawTex(&menu.tex_defeat, &ghost_src, &ghost_dst);
	Gfx_DrawTex(&menu.tex_defeat, &ghost_src, &ghost2_dst);
	Gfx_DrawTex(&menu.tex_defeat, &dead_src, &dead_dst);
	Gfx_DrawTex(&menu.tex_defeat, &dead_src, &dead2_dst);
	Gfx_DrawTex(&menu.tex_defeat, &bone_src, &bone_dst);
	Gfx_DrawTex(&menu.tex_defeat, &bone_src, &bone2_dst);
	
	//Draw arrow
	RECT arrow_src = {0, 31, 28, 3};
	RECT arrow_dst = {(42 * menu.select) + 41,138, 28, 3};
	if (menu.select == 0)
		Gfx_DrawTex(&menu.tex_defeat, &arrow_src, &arrow_dst);
	else if (menu.select == 1)
		Gfx_DrawTex(&menu.tex_defeat, &arrow_src, &arrow_dst);
	else if (menu.select == 2)
		Gfx_DrawTex(&menu.tex_defeat, &arrow_src, &arrow_dst);
	else if (menu.select == 3)
		Gfx_DrawTex(&menu.tex_defeat, &arrow_src, &arrow_dst);
	else if (menu.select == 4)
		Gfx_DrawTex(&menu.tex_defeat, &arrow_src, &arrow_dst);
	else if (menu.select == 5)
		Gfx_DrawTex(&menu.tex_defeat, &arrow_src, &arrow_dst);
}

//Menu functions
void Menu_Load(MenuPage page)
{
	stage.stage_id = StageId_Temp;
	//Load menu assets
	IO_Data menu_arc = IO_Read("\\MENU\\MENU.ARC;1");
	Gfx_LoadTex(&menu.tex_back,  Archive_Find(menu_arc, "back.tim"),  0);
	Gfx_LoadTex(&menu.tex_ng,    Archive_Find(menu_arc, "ng.tim"),    0);
	Gfx_LoadTex(&menu.tex_story, Archive_Find(menu_arc, "story.tim"), 0);
	Gfx_LoadTex(&menu.tex_title, Archive_Find(menu_arc, "title.tim"), 0);
	Gfx_LoadTex(&menu.tex_defeat, Archive_Find(menu_arc, "defeat.tim"), 0);
	Mem_Free(menu_arc);
	
	FontData_Load(&menu.font_bold, Font_Bold, false);
	FontData_Load(&menu.font_arial, Font_Arial, false);
	FontData_Load(&menu.font_cdr, Font_CDR, false);
	
	menu.gf = Char_GF_New(FIXED_DEC(62,1), FIXED_DEC(-12,1));
	stage.camera.x = stage.camera.y = FIXED_DEC(0,1);
	stage.camera.bzoom = FIXED_UNIT;
	stage.gf_speed = 4;
	
	//Initialize menu state
	menu.select = menu.next_select = 0;
	
	switch (menu.page = menu.next_page = page)
	{
		case MenuPage_Opening:
			//Get funny message to use
			//Do this here so timing is less reliant on VSync
			menu.page_state.opening.funny_message = ((*((volatile u32*)0xBF801120)) >> 3) % COUNT_OF(funny_messages); //sysclk seeding
			break;
		default:
			break;
	}
	menu.page_swap = true;
	
	menu.trans_time = 0;
	Trans_Clear();
	
	stage.song_step = 0;

	// to load
	CdlFILE file;
    IO_FindFile(&file, "\\SOUNDS\\SCROLL.VAG;1");
    u32 *data = IO_ReadFile(&file);
    Sounds[0] = Audio_LoadVAGData(data, file.size);
    Mem_Free(data);

	IO_FindFile(&file, "\\SOUNDS\\CONFIRM.VAG;1");
    data = IO_ReadFile(&file);
    Sounds[1] = Audio_LoadVAGData(data, file.size);
    Mem_Free(data);

	IO_FindFile(&file, "\\SOUNDS\\CANCEL.VAG;1");
    data = IO_ReadFile(&file);
    Sounds[2] = Audio_LoadVAGData(data, file.size);
    Mem_Free(data);
	
	IO_FindFile(&file, "\\SOUNDS\\SCROLLFP.VAG;1");
    data = IO_ReadFile(&file);
    Sounds[3] = Audio_LoadVAGData(data, file.size);
    Mem_Free(data);
	
	IO_FindFile(&file, "\\SOUNDS\\CONFIRMF.VAG;1");
    data = IO_ReadFile(&file);
    Sounds[4] = Audio_LoadVAGData(data, file.size);
    Mem_Free(data);
	
	IO_FindFile(&file, "\\SOUNDS\\SWAP0.VAG;1");
    data = IO_ReadFile(&file);
    Sounds[5] = Audio_LoadVAGData(data, file.size);
    Mem_Free(data);
	
	IO_FindFile(&file, "\\SOUNDS\\SWAP1.VAG;1");
    data = IO_ReadFile(&file);
    Sounds[6] = Audio_LoadVAGData(data, file.size);
    Mem_Free(data);
	
	IO_FindFile(&file, "\\SOUNDS\\DEFEAT.VAG;1");
    data = IO_ReadFile(&file);
    Sounds[7] = Audio_LoadVAGData(data, file.size);
    Mem_Free(data);

	//Play menu music
	Audio_PlayXA_Track(XA_GettinFreaky, 0x40, 0, 1);
	Audio_WaitPlayXA();
	
	//Set background colour
	Gfx_SetClear(0, 0, 0);
}

void Menu_Unload(void)
{
	//Free title Girlfriend
	Character_Free(menu.gf);
}

void Menu_ToStage(StageId id, StageDiff diff, boolean story)
{
	menu.next_page = MenuPage_Stage;
	menu.page_param.stage.id = id;
	menu.page_param.stage.story = story;
	menu.page_param.stage.diff = diff;
	Trans_Start();
}

void Menu_Tick(void)
{
	//Clear per-frame flags
	stage.flag &= ~STAGE_FLAG_JUST_STEP;
	
	//Get song position
	u16 next_step = Audio_TellXA_Milli() / 147; //100 BPM
	if (next_step != stage.song_step)
	{
		if (next_step >= stage.song_step)
			stage.flag |= STAGE_FLAG_JUST_STEP;
		stage.song_step = next_step;
	}
	
	//Handle transition out
	if (Trans_Tick())
	{
		//Change to set next page
		menu.page_swap = true;
		menu.page = menu.next_page;
		menu.select = menu.next_select;
	}
	
	//Tick menu page
	MenuPage exec_page;
	switch (exec_page = menu.page)
	{
		case MenuPage_Opening:
		{
			u16 beat = stage.song_step >> 2;
			
			//Start title screen if opening ended
			if (beat >= 16)
			{
				menu.page = menu.next_page = MenuPage_Title;
				menu.page_swap = true;
				//Fallthrough
			}
			else
			{
				//Start title screen if start pressed
				if (pad_state.held & PAD_START)
					menu.page = menu.next_page = MenuPage_Title;
				
				//Draw different text depending on beat
				RECT src_ng = {0, 0, 128, 128};
				const char **funny_message = funny_messages[menu.page_state.opening.funny_message];
				
				switch (beat)
				{
					case 3:
						menu.font_bold.draw(&menu.font_bold, "PRESENTS", screen.SCREEN_WIDTH2, screen.SCREEN_HEIGHT2, FontAlign_Center);
				//Fallthrough
					case 2:
					case 1:
						menu.font_bold.draw(&menu.font_bold, "IMPOSTORM", screen.SCREEN_WIDTH2, screen.SCREEN_HEIGHT2 - 16, FontAlign_Center);
						break;
					
					case 7:
						menu.font_bold.draw(&menu.font_bold, "HEY LOOK ITS NEWGROUNDS",    screen.SCREEN_WIDTH2, screen.SCREEN_HEIGHT2 - 42, FontAlign_Center);
						Gfx_BlitTex(&menu.tex_ng, &src_ng, (screen.SCREEN_WIDTH - 128) >> 1, screen.SCREEN_HEIGHT2 - 26);
						menu.font_bold.draw(&menu.font_bold, "IM TOO LAZY TO REMOVE IT",    screen.SCREEN_WIDTH2, screen.SCREEN_HEIGHT2 + 102, FontAlign_Center);
				//Fallthrough
					case 6:
					case 5:
						menu.font_bold.draw(&menu.font_bold, "THIS IS A MOD TO", screen.SCREEN_WIDTH2, screen.SCREEN_HEIGHT2 - 74, FontAlign_Center);
						menu.font_bold.draw(&menu.font_bold, "PSXFUNKIN LMAO",           screen.SCREEN_WIDTH2, screen.SCREEN_HEIGHT2 - 58, FontAlign_Center);
						break;
					
					case 11:
						menu.font_bold.draw(&menu.font_bold, funny_message[1], screen.SCREEN_WIDTH2, screen.SCREEN_HEIGHT2, FontAlign_Center);
				//Fallthrough
					case 10:
					case 9:
						menu.font_bold.draw(&menu.font_bold, funny_message[0], screen.SCREEN_WIDTH2, screen.SCREEN_HEIGHT2 - 16, FontAlign_Center);
						break;
					
					case 15:
						menu.font_bold.draw(&menu.font_bold, "FUNKIN", screen.SCREEN_WIDTH2, screen.SCREEN_HEIGHT2 + 8, FontAlign_Center);
				//Fallthrough
					case 14:
						menu.font_bold.draw(&menu.font_bold, "NIGHT", screen.SCREEN_WIDTH2, screen.SCREEN_HEIGHT2 - 8, FontAlign_Center);
				//Fallthrough
					case 13:
						menu.font_bold.draw(&menu.font_bold, "FRIDAY", screen.SCREEN_WIDTH2, screen.SCREEN_HEIGHT2 - 24, FontAlign_Center);
						break;
				}
				break;
			}
		}
	//Fallthrough
		case MenuPage_Title:
		{
			//Initialize page
			if (menu.page_swap)
			{
				menu.page_state.title.logo_bump = (FIXED_DEC(7,1) / 24) - 1;
				menu.page_state.title.fade = FIXED_DEC(255,1);
				menu.page_state.title.fadespd = FIXED_DEC(90,1);
			}
			
			//Draw white fade
			if (menu.page_state.title.fade > 0)
			{
				RECT flash = {0, 0, screen.SCREEN_WIDTH, screen.SCREEN_HEIGHT};
				u8 flash_col = menu.page_state.title.fade >> FIXED_SHIFT;
				Gfx_BlendRect(&flash, flash_col, flash_col, flash_col, 1);
				menu.page_state.title.fade -= FIXED_MUL(menu.page_state.title.fadespd, timer_dt);
			}
			
			//Go to main menu when start is pressed
			if (menu.trans_time > 0 && (menu.trans_time -= timer_dt) <= 0)
				Trans_Start();
			
			if ((pad_state.press & PAD_START) && menu.next_page == menu.page && Trans_Idle())
			{
				//play confirm sound
				Audio_PlaySound(Sounds[1], 0x3fff);
				menu.trans_time = FIXED_UNIT;
				menu.page_state.title.fade = FIXED_DEC(255,1);
				menu.page_state.title.fadespd = FIXED_DEC(300,1);
				menu.next_page = MenuPage_Main;
				menu.next_select = 0;
			}
			
			//Draw Friday Night Funkin' logo
			if ((stage.flag & STAGE_FLAG_JUST_STEP) && (stage.song_step & 0x3) == 0 && menu.page_state.title.logo_bump == 0)
				menu.page_state.title.logo_bump = (FIXED_DEC(7,1) / 24) - 1;
			
			static const fixed_t logo_scales[] = {
				FIXED_DEC(1,1),
				FIXED_DEC(101,100),
				FIXED_DEC(102,100),
				FIXED_DEC(103,100),
				FIXED_DEC(105,100),
				FIXED_DEC(110,100),
				FIXED_DEC(97,100),
			};
			fixed_t logo_scale = logo_scales[(menu.page_state.title.logo_bump * 24) >> FIXED_SHIFT];
			u32 x_rad = (logo_scale * (176 >> 1)) >> FIXED_SHIFT;
			u32 y_rad = (logo_scale * (112 >> 1)) >> FIXED_SHIFT;
			
			RECT logo_src = {0, 0, 176, 112};
			RECT logo_dst = {
				100 - x_rad + (screen.SCREEN_WIDEADD2 >> 1),
				68 - y_rad,
				x_rad << 1,
				y_rad << 1
			};
			Gfx_DrawTex(&menu.tex_title, &logo_src, &logo_dst);
			
			if (menu.page_state.title.logo_bump > 0)
				if ((menu.page_state.title.logo_bump -= timer_dt) < 0)
					menu.page_state.title.logo_bump = 0;
			
			//Draw "Press Start to Begin"
			if (menu.next_page == menu.page)
			{
				//Blinking blue
				s16 press_lerp = (MUtil_Cos(animf_count << 3) + 0x100) >> 1;
				u8 press_r = 51 >> 1;
				u8 press_g = (58  + ((press_lerp * (255 - 58))  >> 8)) >> 1;
				u8 press_b = (206 + ((press_lerp * (255 - 206)) >> 8)) >> 1;
				
				RECT press_src = {0, 112, 256, 32};
				Gfx_BlitTexCol(&menu.tex_title, &press_src, (screen.SCREEN_WIDTH - 256) / 2, screen.SCREEN_HEIGHT - 48, press_r, press_g, press_b);
			}
			else
			{
				//Flash white
				RECT press_src = {0, (animf_count & 1) ? 144 : 112, 256, 32};
				Gfx_BlitTex(&menu.tex_title, &press_src, (screen.SCREEN_WIDTH - 256) / 2, screen.SCREEN_HEIGHT - 48);
			}
			
			//Draw Girlfriend
			menu.gf->tick(menu.gf);
			break;
		}
		case MenuPage_Main:
		{
			static const char *menu_options[] = {
				"STORY MODE",
				"FREEPLAY",
				"CREDITS",
				"OPTIONS",
			};
			
			//Initialize page
			if (menu.page_swap)
				menu.scroll = menu.select * FIXED_DEC(12,1);
				
			
			//Draw version identification
			menu.font_bold.draw(&menu.font_bold,
				"PSXFUNKIN BY CUCKYDEV",
				16,
				screen.SCREEN_HEIGHT - 32,
				FontAlign_Left
			);
			
			//Handle option and selection
			if (menu.trans_time > 0 && (menu.trans_time -= timer_dt) <= 0)
				Trans_Start();
			
			if (menu.next_page == menu.page && Trans_Idle())
			{
				//Change option
				if (pad_state.press & PAD_UP)
				{
					//play scroll sound
                    Audio_PlaySound(Sounds[0], 0x3fff);
					if (menu.select > 0)
						menu.select--;
					else
						menu.select = COUNT_OF(menu_options) - 1;
				}
				if (pad_state.press & PAD_DOWN)
				{
					//play scroll sound
                    Audio_PlaySound(Sounds[0], 0x3fff);
					if (menu.select < COUNT_OF(menu_options) - 1)
						menu.select++;
					else
						menu.select = 0;
				}
				
				//Select option if cross is pressed
				if (pad_state.press & (PAD_START | PAD_CROSS))
				{
					//play confirm sound
					Audio_PlaySound(Sounds[1], 0x3fff);
					switch (menu.select)
					{
						case 0: //Story Mode
							menu.next_page = MenuPage_Story;
							break;
						case 1: //Freeplay
							menu.next_page = MenuPage_Freeplay;
							break;
						case 2: //Credits
							menu.next_page = MenuPage_Credits;
							break;
						case 3: //Options
							menu.next_page = MenuPage_Options;
							break;
					}
					menu.next_select = 0;
					menu.trans_time = FIXED_UNIT;
				}
				
				//Return to title screen if circle is pressed
				if (pad_state.press & PAD_CIRCLE)
				{
					//play cancel sound
					Audio_PlaySound(Sounds[2], 0x3fff);
					menu.next_page = MenuPage_Title;
					Trans_Start();
				}
			}
			
			//Draw options
			s32 next_scroll = menu.select * FIXED_DEC(12,1);

			menu.scroll += (next_scroll - menu.scroll) >> 2;
			
			if (menu.next_page == menu.page || menu.next_page == MenuPage_Title)
			{
				//Draw all options
				for (u8 i = 0; i < COUNT_OF(menu_options); i++)
				{
					menu.font_bold.draw(&menu.font_bold,
						Menu_LowerIf(menu_options[i], menu.select != i),
						screen.SCREEN_WIDTH2,
						screen.SCREEN_HEIGHT2 + (i << 5) - 48 - (menu.scroll >> FIXED_SHIFT),
						FontAlign_Center
					);
				}
			}
			else if (animf_count & 2)
			{
				//Draw selected option
				menu.font_bold.draw(&menu.font_bold,
					menu_options[menu.select],
					screen.SCREEN_WIDTH2,
					screen.SCREEN_HEIGHT2 + (menu.select << 5) - 48 - (menu.scroll >> FIXED_SHIFT),
					FontAlign_Center
				);
			}
			
			//Draw background
			Menu_DrawBack(
				menu.next_page == menu.page || menu.next_page == MenuPage_Title,
				menu.scroll >> (FIXED_SHIFT + 3),
				253 >> 1, 231 >> 1, 113 >> 1,
				253 >> 1, 113 >> 1, 155 >> 1
			);
			break;
		}
		case MenuPage_Story:
		{
			static const struct
			{
				const char *week;
				StageId stage;
				const char *name;
				const char *tracks[5];
				int length;
			} menu_options[] = {
				{"1", StageId_SussusMoogus, "POLUS PROBLEMS", {"SUSSUS MOOGUS", "SABOTAGE", "MELTDOWN", NULL, NULL}, 3},
				{"2", StageId_SussusToogus, "MIRA MANIA", {"SUSSUS TOOGUS", "LIGHTS DOWN", "REACTOR", "EJECTED", NULL}, 4},
				{"3", StageId_Mando, "AIRSHIP ATROCITIES", {"MANDO", "DLOW", "OVERSIGHT", "DANGER", "DOUBLE KILL"}, 5},
				{"4", StageId_Defeat, "DEFEAT", {NULL, NULL, NULL, NULL, NULL}, 1},
				{"5", StageId_Ashes, "MAGMATIC MONSTROSITY", {"ASHES", "MAGMATIC", "BOILING POINT", NULL, NULL}, 3},
				{"6", StageId_Delusion, "DEADLY DELUSION", {"DELUSION", "BLACKOUT", "NEUROTIC", NULL, NULL}, 3},
				{"7", StageId_Heartbeat, "HUMANE HEARTBEAT", {"HEARTBEAT", "PINKWAVE", "PRETENDER", NULL, NULL}, 3},
				{NULL, StageId_O2, "JORSAWSEES JAMS", {"OTWO", "VOTING TIME", "TURBULENCE", "VICTORY", NULL}, 4},
				{NULL, StageId_SussyBussy, "ROUSEY RIVAL", {"SUSSY BUSSY", "RIVALS", "CHEWMATE", NULL, NULL}, 3},
				{NULL, StageId_Christmas, "LOGGOS HALLOWEEN", {"CHRISTMAS", "SPOOKPOSTOR", NULL, NULL, NULL}, 2},
				{NULL, StageId_Titular, "BATTLING THE BOYFRIEND", {"TITULAR", "GREATEST PLAN", "REINFORCEMENTS", "ARMED", NULL}, 4},
			};
	
			//Draw week name and tracks
			menu.font_arial.draw(&menu.font_arial,
				scoredisp,
				0,
				7,
				FontAlign_Left
			);

			sprintf(scoredisp, "PERSONAL BEST: %d", increase_Story(menu_options[menu.select].length, menu_options[menu.select].stage));
			
			//Initialize page
			if (menu.page_swap)
			{
				menu.scroll = 0;
				menu.page_param.stage.diff = StageDiff_Normal;
				menu.page_state.title.fade = FIXED_DEC(0,1);
				menu.page_state.title.fadespd = FIXED_DEC(0,1);
				menu.page_param.stage.last = true;
			}
			
			//Draw white fade
			if (menu.page_state.title.fade > 0)
			{
				RECT flash2 = {0, 0, screen.SCREEN_WIDTH, screen.SCREEN_HEIGHT};
				u8 flash_col = menu.page_state.title.fade >> FIXED_SHIFT;
				Gfx_BlendRect(&flash2, flash_col, flash_col, flash_col, 1);
				menu.page_state.title.fade -= FIXED_MUL(menu.page_state.title.fadespd, timer_dt);
			}
			
			//Handle option and selection
			if (menu.trans_time > 0 && (menu.trans_time -= timer_dt) <= 0)
				Trans_Start();
			
			if (menu.next_page == menu.page && Trans_Idle())
			{
				//Change option
				if (pad_state.press & PAD_UP)
				{
					//play scroll sound
					Audio_PlaySound(Sounds[0], 0x3fff);
					if (menu.select > 0)
						menu.select--;
					else
						menu.select = COUNT_OF(menu_options) - 1;
				}
				if (pad_state.press & PAD_DOWN)
				{
					//play scroll sound
                    Audio_PlaySound(Sounds[0], 0x3fff);
					if (menu.select < COUNT_OF(menu_options) - 1)
						menu.select++;
					else
						menu.select = 0;
				}
				
				//Select option if cross is pressed
				if (pad_state.press & (PAD_START | PAD_CROSS))
				{
					if ((menu.select != 3) || ((menu.select == 3) && (stage.prefs.defeat != 1)))
					{
						//play confirm sound
						Audio_PlaySound(Sounds[1], 0x3fff);
						menu.next_page = MenuPage_Stage;
						menu.page_param.stage.id = menu_options[menu.select].stage;
						menu.page_param.stage.story = true;
						menu.trans_time = FIXED_UNIT;
						menu.page_state.title.fade = FIXED_DEC(255,1);
						menu.page_state.title.fadespd = FIXED_DEC(510,1);
					}
					else if ((menu.select == 3) && (stage.prefs.defeat == 1))
					{
						//play confirm sound
						Audio_PlaySound(Sounds[1], 0x3fff);
						menu.next_page = MenuPage_Defeat;
						menu.page_param.stage.id = menu_options[menu.select].stage;
						menu.page_param.stage.story = true;
						menu.trans_time = FIXED_UNIT;
						menu.page_state.title.fade = FIXED_DEC(255,1);
						menu.page_state.title.fadespd = FIXED_DEC(510,1);
					}
				}
				
				//Return to main menu if circle is pressed
				if (pad_state.press & PAD_CIRCLE)
				{
					//play cancel sound
					Audio_PlaySound(Sounds[2], 0x3fff);
					menu.next_page = MenuPage_Main;
					menu.next_select = 0; //Story Mode
					Trans_Start();
				}
			}
			
			//Draw week name and tracks
			menu.font_bold.draw(&menu.font_bold,
				menu_options[menu.select].name,
				screen.SCREEN_WIDTH - 16,
				24,
				FontAlign_Right
			);
			
			const char * const *trackp = menu_options[menu.select].tracks;
			for (size_t i = 0; i < COUNT_OF(menu_options[menu.select].tracks); i++, trackp++)
			{
				if (*trackp != NULL)
					menu.font_bold.draw(&menu.font_bold,
						*trackp,
						screen.SCREEN_WIDTH - 16,
						screen.SCREEN_HEIGHT - (4 * 24) + (i * 24),
						FontAlign_Right
					);
			}
			
			//Draw upper strip
			RECT name_bar = {0, 16, screen.SCREEN_WIDTH, 32};
			Gfx_DrawRect(&name_bar, 249, 207, 81);
			
			//Draw options
			s32 next_scroll = menu.select * FIXED_DEC(48,1);
			menu.scroll += (next_scroll - menu.scroll) >> 3;
			
			if (menu.next_page == menu.page || menu.next_page == MenuPage_Main)
			{
				//Draw all options
				for (u8 i = 0; i < COUNT_OF(menu_options); i++)
				{
					s32 y = 64 + (i * 48) - (menu.scroll >> FIXED_SHIFT);
					if (y <= 16)
						continue;
					if (y >= screen.SCREEN_HEIGHT)
						break;
					Menu_DrawWeek(menu_options[i].week, 48, y);
				}
			}
			else if (animf_count & 2)
			{
				//Draw selected option
				Menu_DrawWeek(menu_options[menu.select].week, 48, 64 + (menu.select * 48) - (menu.scroll >> FIXED_SHIFT));
			}
			
			break;
		}
		case MenuPage_Freeplay:
		{
			static const struct
			{
				StageId stage;
				u32 col;
				const char *text;
			} menu_options[] = {
				{StageId_SussusMoogus, 0xFF9271FD, "SUSSUS MOOGUS"},
				{StageId_Sabotage, 0xFF9271FD, "SABOTAGE"},
				{StageId_Meltdown, 0xFF9271FD, "MELTDOWN"},
				{StageId_SussusToogus, 0xFF9271FD, "SUSSUS TOOGUS"},
				{StageId_LightsDown, 0xFF9271FD, "LIGHTS DOWN"},
				{StageId_Reactor, 0xFF9271FD, "REACTOR"},
				{StageId_Ejected, 0xFF9271FD, "EJECTED"},
				{StageId_Mando, 0xFF9271FD, "MANDO"},
				{StageId_Dlow, 0xFF9271FD, "DLOW"},
				{StageId_Oversight, 0xFF9271FD, "OVERSIGHT"},
				{StageId_Danger, 0xFF9271FD, "DANGER"},
				{StageId_DoubleKill, 0xFF9271FD, "DOUBLE KILL"},
				{StageId_Defeat, 0xFF9271FD, "DEFEAT"},
				{StageId_Finale, 0xFF9271FD, "FINALE"},
				{StageId_IdentityCrisis, 0xFF9271FD, "IDENTITY CRISIS"},
				
				{StageId_Ashes, 0xFF9271FD, "ASHES"},
				{StageId_Magmatic, 0xFF9271FD, "MAGMATIC"},
				{StageId_BoilingPoint, 0xFF9271FD, "BOILING POINT"},
				{StageId_Delusion, 0xFF9271FD, "DELUSION"},
				{StageId_Blackout, 0xFF9271FD, "BLACKOUT"},
				{StageId_Neurotic, 0xFF9271FD, "NEUROTIC"},
				{StageId_Heartbeat, 0xFF9271FD, "HEARTBEAT"},
				{StageId_Pinkwave, 0xFF9271FD, "PINKWAVE"},
				{StageId_Pretender, 0xFF9271FD, "PRETENDER"},
				{StageId_SaucesMoogus, 0xFF9271FD, "SAUCES MOOGUS"},
				
				{StageId_O2, 0xFF9271FD, "OTWO"},
				{StageId_VotingTime, 0xFF9271FD, "VOTING TIME"},
				{StageId_Turbulence, 0xFF9271FD, "TURBULENCE"},
				{StageId_Victory, 0xFF9271FD, "VICTORY"},
				{StageId_Roomcode, 0xFF9271FD, "ROOMCODE"},
				
				{StageId_SussyBussy, 0xFF9271FD, "SUSSY BUSSY"},
				{StageId_Rivals, 0xFF9271FD, "RIVALS"},
				{StageId_Chewmate, 0xFF9271FD, "CHEWMATE"},
				{StageId_TomongusTuesday, 0xFF9271FD, "TOMONGUS TUESDAY"},
				
				{StageId_Christmas, 0xFF9271FD, "CHRISTMAS"},
				{StageId_Spookpostor, 0xFF9271FD, "SPOOKPOSTOR"},
				
				{StageId_Titular, 0xFF9271FD, "TITULAR"},
				{StageId_GreatestPlan, 0xFF9271FD, "GREATEST PLAN"},
				{StageId_Reinforcements, 0xFF9271FD, "REINFORCEMENTS"},
				{StageId_Armed, 0xFF9271FD, "ARMED"},
				
				{StageId_AlphaMoogus, 0xFF9271FD, "ALPHA MOOGUS"},
				{StageId_ActinSus, 0xFF9271FD, "ACTIN SUS"},
				
				{StageId_Ow, 0xFF9271FD, "OW"},
				{StageId_Who, 0xFF9271FD, "WHO"},
				{StageId_InsaneStreamer, 0xFF9271FD, "INSANE STREAMER"},
				{StageId_SussusNuzzus, 0xFF9271FD, "SUSSUS NUZZUS"},
				{StageId_Idk, 0xFF9271FD, "IDK"},
				{StageId_Esculent, 0xFF9271FD, "ESCULENT"},
				{StageId_Drippypop, 0xFF9271FD, "DRIPPYPOP"},
				{StageId_Crewicide, 0xFF9271FD, "CREWICIDE"},
				{StageId_MonotoneAttack, 0xFF9271FD, "MONOTONE ATTACK"},
				{StageId_Top10, 0xFF9271FD, "TOP TEN"},
				
				{StageId_Chippin, 0xFF9271FD, "CHIPPIN"},
				{StageId_Chipping, 0xFF9271FD, "CHIPPING"},
				{StageId_Torture, 0xFF9271FD, "TORTURE"},
			};

			menu.font_arial.draw(&menu.font_arial,
				scoredisp,
				150,
				screen.SCREEN_HEIGHT / 2 - 75,
				FontAlign_Left
			);

			sprintf(scoredisp, "PERSONAL BEST: %d", (stage.prefs.savescore[menu_options[menu.select].stage][menu.page_param.stage.diff] > 0) ? stage.prefs.savescore[menu_options[menu.select].stage][menu.page_param.stage.diff] * 10 : 0);

			//Initialize page
			if (menu.page_swap)
			{
				menu.page_param.stage.last = false;
				menu.scroll = COUNT_OF(menu_options) * FIXED_DEC(24 + screen.SCREEN_HEIGHT2,1);
				menu.page_param.stage.diff = StageDiff_Normal;
				menu.page_state.freeplay.back_r = FIXED_DEC(255,1);
				menu.page_state.freeplay.back_g = FIXED_DEC(255,1);
				menu.page_state.freeplay.back_b = FIXED_DEC(255,1);
			}

			//Draw page label
			menu.font_bold.draw(&menu.font_bold,
				"FREEPLAY",
				16,
				screen.SCREEN_HEIGHT - 32,
				FontAlign_Left
			);
			
			//Handle option and selection
			if (menu.next_page == menu.page && Trans_Idle())
			{
				//Change option
				if (pad_state.press & PAD_UP)
				{
					//play scroll sound
                    Audio_PlaySound(Sounds[3], 0x3fff);
					if (menu.select > 0)
						menu.select--;
					else
						menu.select = COUNT_OF(menu_options) - 1;
				}
				if (pad_state.press & PAD_DOWN)
				{
					//play scroll sound
                    Audio_PlaySound(Sounds[3], 0x3fff);
					if (menu.select < COUNT_OF(menu_options) - 1)
						menu.select++;
					else
						menu.select = 0;
				}
				
				//Select option if cross is pressed
				if (pad_state.press & (PAD_START | PAD_CROSS))
				{
					if ((menu.select != 12) || ((menu.select == 12) && (stage.prefs.defeat != 1)))
					{
						//play confirm sound
						Audio_PlaySound(Sounds[4], 0x3fff);
						menu.next_page = MenuPage_Stage;
						menu.page_param.stage.id = menu_options[menu.select].stage;
						menu.page_param.stage.story = false;
						Trans_Start();
					}
					else if ((menu.select == 12) && (stage.prefs.defeat == 1))
					{
						//play confirm sound
						Audio_PlaySound(Sounds[4], 0x3fff);
						menu.next_page = MenuPage_Defeat;
						menu.page_param.stage.id = menu_options[menu.select].stage;
						menu.page_param.stage.story = false;
						Trans_Start();
					}
				}
				
				//Return to main menu if circle is pressed
				if (pad_state.press & PAD_CIRCLE)
				{
					//play cancel sound
					Audio_PlaySound(Sounds[2], 0x3fff);
					menu.next_page = MenuPage_Main;
					menu.next_select = 1; //Freeplay
					Trans_Start();
				}
			}
	
			//Draw options
			s32 next_scroll = menu.select * FIXED_DEC(24,1);
			menu.scroll += (next_scroll - menu.scroll) >> 4;
			
			for (u8 i = 0; i < COUNT_OF(menu_options); i++)
			{
				//Get position on screen
				s32 y = (i * 24) - 8 - (menu.scroll >> FIXED_SHIFT);
				if (y <= -screen.SCREEN_HEIGHT2 - 8)
					continue;
				if (y >= screen.SCREEN_HEIGHT2 + 8)
					break;
				
				//Draw text
				menu.font_bold.draw(&menu.font_bold,
					Menu_LowerIf(menu_options[i].text, menu.select != i),
					48 + (y >> 2),
					screen.SCREEN_HEIGHT2 + y - 8,
					FontAlign_Left
				);
			}
			
			//Draw background
			fixed_t tgt_r = (fixed_t)((menu_options[menu.select].col >> 16) & 0xFF) << FIXED_SHIFT;
			fixed_t tgt_g = (fixed_t)((menu_options[menu.select].col >>  8) & 0xFF) << FIXED_SHIFT;
			fixed_t tgt_b = (fixed_t)((menu_options[menu.select].col >>  0) & 0xFF) << FIXED_SHIFT;
			
			menu.page_state.freeplay.back_r += (tgt_r - menu.page_state.freeplay.back_r) >> 4;
			menu.page_state.freeplay.back_g += (tgt_g - menu.page_state.freeplay.back_g) >> 4;
			menu.page_state.freeplay.back_b += (tgt_b - menu.page_state.freeplay.back_b) >> 4;
			
			Menu_DrawBack(
				true,
				8,
				menu.page_state.freeplay.back_r >> (FIXED_SHIFT + 1),
				menu.page_state.freeplay.back_g >> (FIXED_SHIFT + 1),
				menu.page_state.freeplay.back_b >> (FIXED_SHIFT + 1),
				0, 0, 0
			);
			break;
		}
		case MenuPage_Credits:
		{
			static const struct
			{
				StageId stage;
				const char *text;
				boolean difficulty;
			} menu_options[] = {
				{StageId_Temp, "PORT DEV", false},
				{StageId_Temp, "    PICHUMANTEN", false},
				{StageId_Temp, "HELP", false},
				{StageId_Temp, "    IGORSOU", false},
				{StageId_Temp, "PSXFUNKIN CREATOR", false},
				{StageId_Temp, "    CUCKYDEV", false},
				{StageId_Temp, "FORK DEVS", false},
				{StageId_Temp, "    UNSTOPABLE", false},
				{StageId_Temp, "    IGORSOU", false},
				{StageId_Temp, "    SPICYJPEG", false},
				{StageId_Temp, "    SPARK", false},
				{StageId_Temp, "OTHER COOL PEOPLE", false},
				{StageId_Temp, "    IGORSOU", false},
				{StageId_Temp, "    UNSTOPABLE", false},
				{StageId_Temp, "    LUCKY", false},
				{StageId_Temp, "    GEYT", false},
				{StageId_Temp, "    DREAMCASTNICK", false},
				{StageId_Temp, "    NINTENDOBRO", false},
				{StageId_Temp, "    MAXDEV", false},
				{StageId_Temp, "    MRRUMBLEROSES", false},
				{StageId_Temp, "OG MOD TEAM", false},
				{StageId_Temp, "    PERSON", false},
			};
			    
			//Initialize page
			if (menu.page_swap)
			{
				menu.scroll = COUNT_OF(menu_options) * FIXED_DEC(24 + screen.SCREEN_HEIGHT2,1);
				menu.page_param.stage.diff = StageDiff_Normal;
			}
			
			//Draw page label
			menu.font_bold.draw(&menu.font_bold,
				"CREDITS",
				16,
				screen.SCREEN_HEIGHT - 32,
				FontAlign_Left
			);
			
			//Draw difficulty selector
			if (menu_options[menu.select].difficulty)
				Menu_DifficultySelector(screen.SCREEN_WIDTH - 100, screen.SCREEN_HEIGHT2 - 48);
			
			//Handle option and selection
			if (menu.next_page == menu.page && Trans_Idle())
			{
				//Change option
				if (pad_state.press & PAD_UP)
				{
					if (menu.select > 0)
						menu.select--;
					else
						menu.select = COUNT_OF(menu_options) - 1;
				}
				if (pad_state.press & PAD_DOWN)
				{
					if (menu.select < COUNT_OF(menu_options) - 1)
						menu.select++;
					else
						menu.select = 0;
				}
				
				//Return to main menu if circle is pressed
				if (pad_state.press & PAD_CIRCLE)
				{
					//play cancel sound
					Audio_PlaySound(Sounds[2], 0x3fff);
					menu.next_page = MenuPage_Main;
					menu.next_select = 2; //Credits
					Trans_Start();
				}
			}
			
			//Draw options
			s32 next_scroll = menu.select * FIXED_DEC(24,1);
			menu.scroll += (next_scroll - menu.scroll) >> 4;
			
			for (u8 i = 0; i < COUNT_OF(menu_options); i++)
			{
				//Get position on screen
				s32 y = (i * 24) - 8 - (menu.scroll >> FIXED_SHIFT);
				if (y <= -screen.SCREEN_HEIGHT2 - 8)
					continue;
				if (y >= screen.SCREEN_HEIGHT2 + 8)
					break;
				
				//Draw text
				menu.font_bold.draw(&menu.font_bold,
					Menu_LowerIf(menu_options[i].text, menu.select != i),
					48 + (y >> 2),
					screen.SCREEN_HEIGHT2 + y - 8,
					FontAlign_Left
				);
			}
			
			//Draw background
			Menu_DrawBack(
				true,
				8,
				197 >> 1, 240 >> 1, 95 >> 1,
				0, 0, 0
			);
			break;
		}
		case MenuPage_Options:
		{
			static const char *gamemode_strs[] = {"NORMAL", "SWAP", "TWO PLAYER"};
			static const struct
			{
				enum
				{
					OptType_Boolean,
					OptType_Enum,
				} type;
				const char *text;
				void *value;
				union
				{
					struct
					{
						int a;
					} spec_boolean;
					struct
					{
						s32 max;
						const char **strs;
					} spec_enum;
				} spec;
			} menu_options[] = {
				{OptType_Enum,    "GAMEMODE", &stage.mode, {.spec_enum = {COUNT_OF(gamemode_strs), gamemode_strs}}},
				{OptType_Boolean, "GHOST TAP", &stage.prefs.ghost, {.spec_boolean = {0}}},
				{OptType_Boolean, "DOWNSCROLL", &stage.prefs.downscroll, {.spec_boolean = {0}}},
				{OptType_Boolean, "MIDDLESCROLL", &stage.prefs.middlescroll, {.spec_boolean = {0}}},
				{OptType_Boolean, "FLASHING LIGHTS", &stage.prefs.flash, {.spec_boolean = {0}}},
				{OptType_Boolean, "SHOW SONG TIME", &stage.prefs.songtimer, {.spec_boolean = {0}}},
				{OptType_Boolean, "MISS SOUNDS", &stage.prefs.sfxmiss, {.spec_boolean = {0}}},
				{OptType_Boolean, "DEFEAT MISSES", &stage.prefs.defeat, {.spec_boolean = {0}}},
				{OptType_Boolean, "PAL REFRESH RATE", &stage.prefs.palmode, {.spec_boolean = {0}}},
				{OptType_Boolean, "BOTPLAY", &stage.prefs.botplay, {.spec_boolean = {0}}},
				{OptType_Boolean, "PRACTICE MODE", &stage.prefs.practice, {.spec_boolean = {0}}},
				{OptType_Boolean, "DEBUG MODE", &stage.prefs.debug, {.spec_boolean = {0}}},
			};
			if (menu.select == 8 && pad_state.press & (PAD_CROSS | PAD_LEFT | PAD_RIGHT))
				stage.pal_i = 1;

			if (stage.mode == StageMode_2P)
				stage.prefs.middlescroll = false;
			
			//Initialize page
			if (menu.page_swap)
				menu.scroll = COUNT_OF(menu_options) * FIXED_DEC(24 + screen.SCREEN_HEIGHT2,1);
			
			RECT save_src = {0, 120, 55, 7};
			RECT save_dst = {screen.SCREEN_WIDTH / 2 + 30 - (121 / 2), screen.SCREEN_HEIGHT - 30, 53 * 2, 7 * 2};
			Gfx_DrawTex(&menu.tex_story, &save_src, &save_dst);

			//Draw page label
			menu.font_bold.draw(&menu.font_bold,
				"OPTIONS",
				16,
				screen.SCREEN_HEIGHT - 32,
				FontAlign_Left
			);
			
			//Handle option and selection
			if (menu.next_page == menu.page && Trans_Idle())
			{
				//Change option
				if (pad_state.press & PAD_UP)
				{
					//play scroll sound
                    Audio_PlaySound(Sounds[0], 0x3fff);
					if (menu.select > 0)
						menu.select--;
					else
						menu.select = COUNT_OF(menu_options) - 1;
				}
				if (pad_state.press & PAD_DOWN)
				{
					//play scroll sound
                    Audio_PlaySound(Sounds[0], 0x3fff);
					if (menu.select < COUNT_OF(menu_options) - 1)
						menu.select++;
					else
						menu.select = 0;
				}
				
				//Handle option changing
				switch (menu_options[menu.select].type)
				{
					case OptType_Boolean:
						if (pad_state.press & (PAD_CROSS | PAD_LEFT | PAD_RIGHT))
							*((boolean*)menu_options[menu.select].value) ^= 1;
						break;
					case OptType_Enum:
						if (pad_state.press & PAD_LEFT)
							if (--*((s32*)menu_options[menu.select].value) < 0)
								*((s32*)menu_options[menu.select].value) = menu_options[menu.select].spec.spec_enum.max - 1;
						if (pad_state.press & PAD_RIGHT)
							if (++*((s32*)menu_options[menu.select].value) >= menu_options[menu.select].spec.spec_enum.max)
								*((s32*)menu_options[menu.select].value) = 0;
						break;
				}
				
				if (pad_state.press & PAD_SELECT)
					writeSaveFile();

				//Return to main menu if circle is pressed
				if (pad_state.press & PAD_CIRCLE)
				{
					//play cancel sound
					Audio_PlaySound(Sounds[2], 0x3fff);
					menu.next_page = MenuPage_Main;
					menu.next_select = 3; //Options
					Trans_Start();
				}
			}
			
			//Draw options
			s32 next_scroll = menu.select * FIXED_DEC(24,1);
			menu.scroll += (next_scroll - menu.scroll) >> 4;
			
			for (u8 i = 0; i < COUNT_OF(menu_options); i++)
			{
				//Get position on screen
				s32 y = (i * 24) - 8 - (menu.scroll >> FIXED_SHIFT);
				if (y <= -screen.SCREEN_HEIGHT2 - 8)
					continue;
				if (y >= screen.SCREEN_HEIGHT2 + 8)
					break;
				
				//Draw text
				char text[0x80];
				switch (menu_options[i].type)
				{
					case OptType_Boolean:
						sprintf(text, "%s %s", menu_options[i].text, *((boolean*)menu_options[i].value) ? "ON" : "OFF");
						break;
					case OptType_Enum:
						sprintf(text, "%s %s", menu_options[i].text, menu_options[i].spec.spec_enum.strs[*((s32*)menu_options[i].value)]);
						break;
				}
				menu.font_bold.draw(&menu.font_bold,
					Menu_LowerIf(text, menu.select != i),
					48 + (y >> 2),
					screen.SCREEN_HEIGHT2 + y - 8,
					FontAlign_Left
				);
			}
			
			//Draw background
			Menu_DrawBack(
				true,
				8,
				253 >> 1, 113 >> 1, 155 >> 1,
				0, 0, 0
			);
			break;
		}
		case MenuPage_Stage:
		{
			//Unload menu state
			Menu_Unload();
			
			//Load new stage
			LoadScr_Start();
			Stage_Load(menu.page_param.stage.id, menu.page_param.stage.diff, menu.page_param.stage.story);
			gameloop = GameLoop_Stage;
			LoadScr_End();
			break;
		}
		case MenuPage_Defeat:
		{
			//Initialize page
			if (menu.page_swap)
			{
				Audio_StopXA();
				menu.select = 0;
				menu.idk = false;
				menu.page_state.title.fade = FIXED_DEC(0,1);
				menu.page_state.title.fadespd = FIXED_DEC(0,1);
			}
			
			//Handle option and selection
			if (menu.trans_time > 0 && (menu.trans_time -= timer_dt) <= 0)
				Trans_Start();
			
			if (menu.next_page == menu.page && Trans_Idle())
			{
				//Change option
				if (pad_state.press & PAD_LEFT)
				{
					//play scroll sound
					Audio_PlaySound(Sounds[6], 0x3fff);
					if (menu.select > 0)
						menu.select--;
					else
						menu.select = 6 - 1;
				}
				if (pad_state.press & PAD_RIGHT)
				{
					//play scroll sound
                    Audio_PlaySound(Sounds[5], 0x3fff);
					if (menu.select < 6 - 1)
						menu.select++;
					else
						menu.select = 0;
				}
				
				//Select option if cross is pressed
				if (pad_state.press & (PAD_START | PAD_CROSS))
				{
					//play confirm sound
					Audio_PlaySound(Sounds[7], 0x3fff);
					menu.idk = true;
					menu.page_param.stage.id = StageId_Defeat;
					menu.next_page = MenuPage_Stage;
					menu.trans_time = FIXED_UNIT;
				}
				
				//Return to main menu if circle is pressed
				if (pad_state.press & PAD_CIRCLE)
				{
					//play cancel sound
					Audio_PlaySound(Sounds[2], 0x3fff);
					if (menu.page_param.stage.last == true)
					{
						menu.next_page = MenuPage_Story;
						menu.next_select = 3; //Story Mode
					}
					else
					{
						menu.next_page = MenuPage_Freeplay;
						menu.next_select = 12; //Story Mode
					}
					Trans_Start();
					Audio_PlayXA_Track(XA_GettinFreaky, 0x40, 0, 1);
					Audio_WaitPlayXA();
				}
			}
			
			//Draw options
			if (menu.idk != true)
				Menu_DrawDefeat();
			if (menu.select == 0)
				stage.defeatmiss = 5;
			else if (menu.select == 1)
				stage.defeatmiss = 4;
			else if (menu.select == 2)
				stage.defeatmiss = 3;
			else if (menu.select == 3)
				stage.defeatmiss = 2;
			else if (menu.select == 4)
				stage.defeatmiss = 1;
			else if (menu.select == 5)
				stage.defeatmiss = 0;
			
			//Draw black
			RECT screen_src = {0, 0, screen.SCREEN_WIDTH, screen.SCREEN_HEIGHT};
			Gfx_DrawRect(&screen_src, 0, 0, 0);
			break;
		}
		default:
			break;
	}
	
	//Clear page swap flag
	menu.page_swap = menu.page != exec_page;
}
