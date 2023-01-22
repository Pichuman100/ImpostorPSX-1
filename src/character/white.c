/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "white.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//White character structure
enum
{
  White_ArcMain_Idle0,
  White_ArcMain_Idle1,
  White_ArcMain_Idle2,
  White_ArcMain_Left0,
  White_ArcMain_Left1,
  White_ArcMain_Down0,
  White_ArcMain_Up0,
  White_ArcMain_Up1,
  White_ArcMain_Right0,
  White_ArcMain_Right1,
	
	White_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[White_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_White;

//White character definitions
static const CharFrame char_white_frame[] = {
  {White_ArcMain_Idle0, {  0,  0,166,124}, {135,102}}, //0 idle 1 G
  {White_ArcMain_Idle0, {  0,124,164,123}, {133,101}}, //1 idle 2 G
  {White_ArcMain_Idle1, {  0,  0,164,121}, {134,101}}, //2 idle 3 
  {White_ArcMain_Idle1, {  0,121,167,117}, {138,102}}, //3 idle 4
  {White_ArcMain_Idle2, {  0,  0,167,117}, {138,103}}, //4 idle 5
  {White_ArcMain_Idle2, {  0,117,168,116}, {139,102}}, //5 idle 6

  {White_ArcMain_Left0, {  0,  0,173,112}, {164,105}}, //6 left 1
  {White_ArcMain_Left0, {  0,114,172,114}, {162,107}}, //7 left 2
  {White_ArcMain_Left1, {  0,  0,172,113}, {163,106}}, //8 left 3

  {White_ArcMain_Down0, {  0,  0,109,103}, { 84, 83}}, //9 down 1
  {White_ArcMain_Down0, {109,  0,106,102}, { 85, 84}}, //10 down 2
  {White_ArcMain_Down0, {  0,103,106,102}, { 85, 84}}, //11 down 3

  {White_ArcMain_Up0, {  0,  0,171,121}, {140,116}}, //12 up 1
  {White_ArcMain_Up0, {  0,122,166,122}, {139,117}}, //13 up 2
  {White_ArcMain_Up1, {  0,  0,167,121}, {139,116}}, //14 up 3

  {White_ArcMain_Right0, {  0,  0,151,106}, { 79,100}}, //15 right 1
  {White_ArcMain_Right0, {  0,107,154,107}, { 82, 99}}, //16 right 2
  {White_ArcMain_Right1, {  0,  0,153,107}, { 82, 99}}, //17 right 3
};

static const Animation char_white_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, 3, 4, 5, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 6, 7, 8, 8, 8, ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 9, 10, 11, 11, 11, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 12, 13, 14, 14, 14, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){ 15, 16, 17, 17, 17, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//White character functions
void Char_White_SetFrame(void *user, u8 frame)
{
	Char_White *this = (Char_White*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_white_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_White_Tick(Character *character)
{
	Char_White *this = (Char_White*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_White_SetFrame);
	Character_Draw(character, &this->tex, &char_white_frame[this->frame]);
}

void Char_White_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_White_Free(Character *character)
{
	Char_White *this = (Char_White*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_White_New(fixed_t x, fixed_t y)
{
	//Allocate white object
	Char_White *this = Mem_Alloc(sizeof(Char_White));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_White_New] Failed to allocate white object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_White_Tick;
	this->character.set_anim = Char_White_SetAnim;
	this->character.free = Char_White_Free;
	
	Animatable_Init(&this->character.animatable, char_white_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	this->character.idle2 = 0;
	
	this->character.health_i = 1;

	//health bar color
	this->character.health_bar = 0xFFD1D2F8;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-115,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	this->character.size = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\WHITE.ARC;1");
	
	const char **pathp = (const char *[]){
  "idle0.tim",
  "idle1.tim",
  "idle2.tim",
  "left0.tim",
  "left1.tim",
  "down0.tim",
  "up0.tim",
  "up1.tim",
  "right0.tim",
  "right1.tim",
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}