/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "jads.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Jads character structure
enum
{
  Jads_ArcMain_Idle0,
  Jads_ArcMain_Idle1,
  Jads_ArcMain_Idle2,
  Jads_ArcMain_Left0,
  Jads_ArcMain_Left1,
  Jads_ArcMain_Down0,
  Jads_ArcMain_Down1,
  Jads_ArcMain_Up0,
  Jads_ArcMain_Right0,
  Jads_ArcMain_Right1,
	
	Jads_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Jads_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Jads;

//Jads character definitions
static const CharFrame char_jads_frame[] = {
  {Jads_ArcMain_Idle0, {  0,  0,151, 92}, {173, 86}}, //0 idle 1
  {Jads_ArcMain_Idle0, {  0, 92,145, 94}, {171, 88}}, //1 idle 2
  {Jads_ArcMain_Idle1, {  0,  0,141, 95}, {170, 89}}, //2 idle 3
  {Jads_ArcMain_Idle1, {  0, 95,139, 95}, {169, 90}}, //3 idle 4
  {Jads_ArcMain_Idle2, {  0,  0,138, 95}, {169, 90}}, //4 idle 5

  {Jads_ArcMain_Left0, {  0,  0,156, 94}, {222-39, 86+2}}, //5 left 1
  {Jads_ArcMain_Left0, {  0, 94,152, 96}, {217-39, 87+2}}, //6 left 2
  {Jads_ArcMain_Left1, {  0,  0,150, 96}, {215-39, 88+2}}, //7 left 3

  {Jads_ArcMain_Down0, {  0,  0,151, 88}, {186-8, 81-4}}, //8 down 1
  {Jads_ArcMain_Down0, {  0, 88,143, 90}, {182-8, 85-4}}, //9 down 2
  {Jads_ArcMain_Down1, {  0,  0,143, 90}, {182-8, 85-4}}, //10 down 3

  {Jads_ArcMain_Up0, {  0,  0,122,104}, {199-40,120-20}}, //11 up 1
  {Jads_ArcMain_Up0, {122,  0,120,102}, {198-40,118-20}}, //12 up 2
  {Jads_ArcMain_Up0, {  0,104,120,102}, {198-40,118-20}}, //13 up 3

  {Jads_ArcMain_Right0, {  0,  0,163, 96}, {174-3, 86+2}}, //14 right 1
  {Jads_ArcMain_Right0, {  0, 96,154, 96}, {169-3, 87+2}}, //15 right 2
  {Jads_ArcMain_Right1, {  0,  0,154, 96}, {169-3, 87+2}}, //16 right 3
};

static const Animation char_jads_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, 3, 4, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 5, 6, 7, 7, ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 8, 9, 10, 10, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 11, 12, 13, 13, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){ 14, 15, 16, 16, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Jads character functions
void Char_Jads_SetFrame(void *user, u8 frame)
{
	Char_Jads *this = (Char_Jads*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_jads_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Jads_Tick(Character *character)
{
	Char_Jads *this = (Char_Jads*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Jads_SetFrame);
	Character_Draw(character, &this->tex, &char_jads_frame[this->frame]);
}

void Char_Jads_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Jads_Free(Character *character)
{
	Char_Jads *this = (Char_Jads*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Jads_New(fixed_t x, fixed_t y)
{
	//Allocate jads object
	Char_Jads *this = Mem_Alloc(sizeof(Char_Jads));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Jads_New] Failed to allocate jads object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Jads_Tick;
	this->character.set_anim = Char_Jads_SetAnim;
	this->character.free = Char_Jads_Free;
	
	Animatable_Init(&this->character.animatable, char_jads_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;

	//health bar color
	this->character.health_bar = 0xFFAD63D6;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-115,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	this->character.zoom_save = this->character.focus_zoom;
	this->character.size = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\OPPONET2\\JADS.ARC;1");
	
	const char **pathp = (const char *[]){
  "idle0.tim",
  "idle1.tim",
  "idle2.tim",
  "left0.tim",
  "left1.tim",
  "down0.tim",
  "down1.tim",
  "up0.tim",
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