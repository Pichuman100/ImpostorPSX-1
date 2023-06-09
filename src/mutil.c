/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "mutil.h"

#include "timer.h"

//Sine table
static const s16 sine_table[0x140] = {
	0,6,12,18,25,31,37,43,49,56,62,68,74,80,86,92,
	97,103,109,115,120,126,131,136,142,147,152,157,162,167,171,176,
	181,185,189,193,197,201,205,209,212,216,219,222,225,228,231,234,
	236,238,241,243,244,246,248,249,251,252,253,254,254,255,255,255,
	256,255,255,255,254,254,253,252,251,249,248,246,244,243,241,238,
	236,234,231,228,225,222,219,216,212,209,205,201,197,193,189,185,
	181,176,171,167,162,157,152,147,142,136,131,126,120,115,109,103,
	97,92,86,80,74,68,62,56,49,43,37,31,25,18,12,6,
	0,-6,-12,-18,-25,-31,-37,-43,-49,-56,-62,-68,-74,-80,-86,-92,
	-97,-103,-109,-115,-120,-126,-131,-136,-142,-147,-152,-157,-162,-167,-171,-176,
	-181,-185,-189,-193,-197,-201,-205,-209,-212,-216,-219,-222,-225,-228,-231,-234,
	-236,-238,-241,-243,-244,-246,-248,-249,-251,-252,-253,-254,-254,-255,-255,-255,
	-256,-255,-255,-255,-254,-254,-253,-252,-251,-249,-248,-246,-244,-243,-241,-238,
	-236,-234,-231,-228,-225,-222,-219,-216,-212,-209,-205,-201,-197,-193,-189,-185,
	-181,-176,-171,-167,-162,-157,-152,-147,-142,-136,-131,-126,-120,-115,-109,-103,
	-97,-92,-86,-80,-74,-68,-62,-56,-49,-43,-37,-31,-25,-18,-12,-6,
	0,6,12,18,25,31,37,43,49,56,62,68,74,80,86,92,
	97,103,109,115,120,126,131,136,142,147,152,157,162,167,171,176,
	181,185,189,193,197,201,205,209,212,216,219,222,225,228,231,234,
	236,238,241,243,244,246,248,249,251,252,253,254,254,255,255,255,
};

//Math utility functions
s16 MUtil_Sin(u8 x)
{
	return sine_table[x];
}

s16 MUtil_Cos(u8 x)
{
	return (sine_table + 0x40)[x];
}

void MUtil_RotatePoint(POINT *p, s16 s, s16 c)
{
	short px = p->x;
	short py = p->y;
	p->x = ((px * c) >> 8) - ((py * s) >> 8);
	p->y = ((px * s) >> 8) + ((py * c) >> 8);
}

fixed_t fixed_lerp(fixed_t current, fixed_t target, fixed_t speed)
{
    // Calculate the difference between the target and current positions.
    fixed_t difference = target - current;

    // Calculate the new position by interpolating between the current and target position.
    fixed_t new_position = current + FIXED_MUL(difference, speed);

    // Check if the difference between the new position and the target is within a threshold.
    if ((difference <= FIXED_UNIT) && (difference >= -FIXED_UNIT))
        new_position = target;

    return new_position;
}
