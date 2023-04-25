/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef PSXF_GUARD_STAGE_H
#define PSXF_GUARD_STAGE_H

#include "events.h"
#include "io.h"
#include "gfx.h"
#include "pad.h"

#include "fixed.h"
#include "character.h"
#include "player.h"
#include "object.h"
#include "font.h"
#include "debug.h"

#include "network.h"
#include "events.h"

//Stage constants
#define INPUT_LEFT  (PAD_LEFT  | PAD_SQUARE | PAD_L2)
#define INPUT_DOWN  (PAD_DOWN  | PAD_CROSS | PAD_L1)
#define INPUT_UP    (PAD_UP    | PAD_TRIANGLE | PAD_R1)
#define INPUT_RIGHT (PAD_RIGHT | PAD_CIRCLE | PAD_R2)

#define STAGE_FLAG_JUST_STEP     (1 << 0) //Song just stepped this frame
#define STAGE_FLAG_VOCAL_ACTIVE  (1 << 1) //Song's vocal track is currently active
#define STAGE_FLAG_SCORE_REFRESH (1 << 2) //Score text should be refreshed

#define STAGE_LOAD_PLAYER     (1 << 0) //Reload player character
#define STAGE_LOAD_PLAYER2    (1 << 1) //Reload player character
#define STAGE_LOAD_OPPONENT   (1 << 2) //Reload opponent character
#define STAGE_LOAD_OPPONENT2  (1 << 3) //Reload 2nd opponent character
#define STAGE_LOAD_GIRLFRIEND (1 << 4) //Reload girlfriend character
#define STAGE_LOAD_STAGE      (1 << 5) //Reload stage
#define STAGE_LOAD_FLAG       (1 << 7)


//Stage enums
typedef enum
{
	StageId_SussusMoogus, //Sussus Moogus
	StageId_Sabotage, //Sabotage
	StageId_Meltdown, //Meltdown
	StageId_SussusToogus, //Sussus Toogus
	StageId_LightsDown, //Lights Down
	StageId_Reactor, //Reactor
	StageId_Ejected, //Ejected
	StageId_Mando, //Mando
	StageId_Dlow, //Dlow
	StageId_Oversight, //Oversight
	StageId_Danger, //Danger
	StageId_DoubleKill, //Double Kill
	StageId_Defeat, //Defeat
	StageId_Finale, //Finale
	StageId_IdentityCrisis, //Identity Crisis
	
	StageId_Ashes, //Ashes
	StageId_Magmatic, //Magmatic
	StageId_BoilingPoint, //Boiling Point
	StageId_Delusion, //Delusion
	StageId_Blackout, //Blackout
	StageId_Neurotic, //Neurotic
	StageId_Heartbeat, //Heartbeat
	StageId_Pinkwave, //Pinkwave
	StageId_Pretender, //Pretender
	StageId_SaucesMoogus, //Sauces Moogus
	
	StageId_O2, //O2
	StageId_VotingTime, //Voting Time
	StageId_Turbulence, //Turbulence
	StageId_Victory, //Victory
	StageId_Roomcode, //Roomcode
	
	StageId_SussyBussy, //Sussy Bussy
	StageId_Rivals, //Rivals
	StageId_Chewmate, //Chewmate
	StageId_TomongusTuesday, //Tomongus Tuesday
	
	StageId_Christmas, //Christmas
	StageId_Spookpostor, //Spookpostor
	
	StageId_Titular, //Titular
	StageId_GreatestPlan, //Greatest Plan
	StageId_Reinforcements, //Reinforcements
	StageId_Armed, //Armed
	
	StageId_AlphaMoogus, //Alpha Moogus
	StageId_ActinSus, //Actin Sus
	
	StageId_Ow, //Ow
	StageId_Who, //Who
	StageId_InsaneStreamer, //Insane Streamer
	StageId_SussusNuzzus, //Sussus Nuzzus
	StageId_Idk, //Idk
	StageId_Esculent, //Esculent
	StageId_Drippypop, //Drippypop
	StageId_Crewicide, //Crewicide
	StageId_MonotoneAttack, //Monotone Attack
	StageId_Top10, //Top 10
	
	StageId_Chippin, //Chippin
	StageId_Chipping, //Chipping
	StageId_Torture, //Torture
	
	StageId_LemonLime, //Lemon Lime
	StageId_Chlorophyll, //Chlorophyll
	StageId_Inflorescence, //Inflorescence
	StageId_Stargazer, //Stargazer
	
	StageId_Temp, //Placeholder
	
	StageId_Max
} StageId;

#define StageId_LastVanilla StageId_Stargazer

typedef enum
{
	StageDiff_Easy,
	StageDiff_Normal,
	StageDiff_Hard,
} StageDiff;

typedef enum
{
	StageMode_Normal,
	StageMode_Swap,
	StageMode_2P,
	StageMode_Net1,
	StageMode_Net2,
} StageMode;

typedef enum
{
	StageTrans_Menu,
	StageTrans_NextSong,
	StageTrans_Reload,
} StageTrans;

//Stage background
typedef struct StageBack
{
	//Stage background functions
	void (*draw_fg)(struct StageBack*);
	void (*draw_md)(struct StageBack*);
	void (*draw_bg)(struct StageBack*);
	void (*free)(struct StageBack*);
} StageBack;

//Stage definitions
typedef struct
{
	//Characters
	struct
	{
		Character* (*new)();
		fixed_t x, y;
	} pchar, pchar2, ochar, ochar2, gchar;
	
	//Stage background
	StageBack* (*back)();
	
	//Song info
	u8 week, week_song;
	u8 music_track, music_channel;
	const char songname[40];
	const char composer[40];
	const char composer2[40];
	
	StageId next_stage;
	u8 next_load;
} StageDef;

//Stage state
#define SECTION_FLAG_OPPFOCUS (1 << 15) //Focus on opponent
#define SECTION_FLAG_BPM_MASK 0x7FFF //1/24

typedef struct
{
	u16 end; //1/12 steps
	u16 flag;
} Section;

#define NOTE_FLAG_OPPONENT    (1 << 2) //Note is opponent's
#define NOTE_FLAG_SUSTAIN     (1 << 3) //Note is a sustain note
#define NOTE_FLAG_SUSTAIN_END (1 << 4) //Is either end of sustain
#define NOTE_FLAG_ALT_ANIM    (1 << 5) //Note plays alt animation
#define NOTE_FLAG_MINE        (1 << 6) //Note is a mine
#define NOTE_FLAG_HIT         (1 << 7) //Note has been hit
#define NOTE_FLAG_CHAR2SING   (1 << 8) //Note that only the 2 character plays
#define NOTE_FLAG_BOTHSING    (1 << 9) //Note that only that both the 1 and 2 character sings
#define NOTE_FLAG_NO_ANIM    (1 << 10) //Note plays no animation

#define NOTE_FLAG_PLAYED    (1 << 15) //Note that only used for check if note has been already played

typedef struct
{
	u16 pos; //1/12 steps
	u16 type;
} Note;

typedef struct
{
	Character *character;
	Character *character2;
	Character *charactersecond;
	
	fixed_t arrow_hitan[4]; //Arrow hit animation for presses

	s16 health;
	u16 combo;
	
	boolean refresh_score;
	s32 score, max_score;
	char score_text[13];

	boolean refresh_miss;
	s32 miss;
	char miss_text[13];
	
	boolean refresh_accuracy;
	s32 min_accuracy;
	s32 accuracy;
	s32 max_accuracy;
	char accuracy_text[21];

	char rank[13];
	
	u16 pad_held, pad_press;
} PlayerState;

typedef struct
{
	DISPENV disp[2];
	DRAWENV draw[2];

	//Stage settings
	int pal_i, wide_i;
	int pause_state;
	struct
	{
		boolean ghost, downscroll, middlescroll, expsync, practice, debug, palmode, widescreen, sfxmiss, songtimer, botplay, flash, defeat;
		int savescore[StageId_Max][3];
	} prefs;
	s32 mode;
	
	u32 offset;
	
	fixed_t pause_scroll;
	u8 pause_select;
	boolean paused;
	boolean black;
	
	//HUD textures
	Gfx_Tex tex_hud0, tex_hud1, tex_count;
	Gfx_Tex tex_ded;

	//font
	FontData font_cdr, font_bold;
	
	//Stage data
	const StageDef *stage_def;
	StageId stage_id;
	StageDiff stage_diff;
	char songname[40];
	char composer[40];
	char composer2[40];
	
	IO_Data chart_data;
	Section *sections;
	Note *notes;
	size_t num_notes;
	Event* events;

	IO_Data event_chart_data;
	Section *event_sections;
	Note *event_notes;
	Event* event_events;
	
	fixed_t speed, ogspeed;
	fixed_t step_crochet, step_time;
	fixed_t early_safe, late_safe, early_sus_safe, late_sus_safe;
	fixed_t flash, flashspd;
	fixed_t reactor, reactorspd, pink, pinkspd;
	fixed_t bop1, bop2, bopintense1, bopintense2;
	int defeatmiss, lights, hudfade, camswitch;
	boolean pinkstuff;
	u16 opacity;
	
	//if stage have intro or no
	boolean intro;
	
	//Stage state
	boolean story;
	u8 flag;
	StageTrans trans;
	
	struct
	{
		fixed_t x, y, zoom;
		fixed_t tx, ty, tz, td;
		fixed_t bzoom;
	} camera;
	fixed_t bump, sbump, charbump;
	boolean cam_should_scroll;
	
	StageBack *back;
	
	Character *player;
	Character *player2;
	Character *opponent;
	Character *opponent2;
	Character *gf;
	
	Section *cur_section; //Current section
	Note *cur_note; //First visible and hittable note, used for drawing and hit detection
	Event* cur_event; //Current event

	// For event.json
	Section *event_cur_section; //Current section
	Note *event_cur_note; //First visible and hittable note, used for drawing and hit detection
	Event* event_cur_event; //Current event

	fixed_t event_step_crochet;
	fixed_t event_note_scroll;
	fixed_t note_scroll, song_time, interp_time, interp_ms, interp_speed;
	
	u16 last_bpm;
	
	fixed_t time_base;
	u16 step_base;
	Section *section_base;
	
	s16 noteshakex;
	s16 noteshakey;

	s32 song_step;
	s16 song_beat;

	boolean freecam;
	
	u8 gf_speed; //Typically 4 steps, changes in Fresh
	
	PlayerState player_state[2];
	s32 max_score;
	
	enum
	{
		StageState_Play, //Game is playing as normal
		StageState_Dead,       //Start BREAK animation and reading extra data from CD
		StageState_DeadLoad,   //Wait for said data to be read
		StageState_DeadDrop,   //Mic drop
		StageState_DeadRetry,  //Retry prompt
		StageState_DeadDecide, //Decided
	} state;
	
	u8 note_swap;
	
	//Object lists
	ObjectList objlist_splash, objlist_fg, objlist_bg;
} Stage;

extern Stage stage;

//Stage drawing functions
void Stage_DrawTexCol(Gfx_Tex *tex, const RECT *src, const RECT_FIXED *dst, fixed_t zoom, u8 r, u8 g, u8 b);
void Stage_DrawTex(Gfx_Tex *tex, const RECT *src, const RECT_FIXED *dst, fixed_t zoom);
void Stage_DrawTexRotate(Gfx_Tex *tex, const RECT *src, const RECT_FIXED *dst, fixed_t zoom, u8 angle);
void Stage_BlendTexRotate(Gfx_Tex *tex, const RECT *src, const RECT_FIXED *dst, fixed_t zoom, u8 angle, u8 mode);
void Stage_BlendTex(Gfx_Tex *tex, const RECT *src, const RECT_FIXED *dst, fixed_t zoom, u8 mode);
void Stage_BlendTexV2(Gfx_Tex *tex, const RECT *src, const RECT_FIXED *dst, fixed_t zoom, u8 mode, u8 opacity);
void Stage_DrawTexArb(Gfx_Tex *tex, const RECT *src, const POINT_FIXED *p0, const POINT_FIXED *p1, const POINT_FIXED *p2, const POINT_FIXED *p3, fixed_t zoom);
void Stage_BlendTexArb(Gfx_Tex *tex, const RECT *src, const POINT_FIXED *p0, const POINT_FIXED *p1, const POINT_FIXED *p2, const POINT_FIXED *p3, fixed_t zoom, u8 mode);

//Stage functions
void Stage_Load(StageId id, StageDiff difficulty, boolean story);
void Stage_Unload();
void Stage_Tick();

#endif
