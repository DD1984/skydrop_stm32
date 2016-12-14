/*
 * layouts.h
 *
 *  Created on: 2.3.2015
 *      Author: horinek
 */

#ifndef LAYOUTS_H_
#define LAYOUTS_H_

struct layout_t;

#include "../widgets.h"


struct widget_pos {
	uint8_t x;
	uint8_t y;
	uint8_t w;
	uint8_t h;
};

struct layout_desc {
	uint8_t number_of_widgets;

	widget_pos widgets[MAX_WIDGES_PER_PAGE];
};


struct layout_t
{
	uint8_t type;

	uint8_t widgets[MAX_WIDGES_PER_PAGE];
};


#define LAYOUT_OFF		0xFF
#define LAYOUT_1		0
#define LAYOUT_113		1
#define LAYOUT_12		2
#define LAYOUT_121		3
#define LAYOUT_122		4
#define LAYOUT_123		5
#define LAYOUT_21		6
#define LAYOUT_22		7
#define LAYOUT_222		8
#define LAYOUT_333		9

#define NUMBER_OF_LAYOUTS	10


extern const layout_desc * layout_list[NUMBER_OF_LAYOUTS];

#endif /* LAYOUTS_H_ */
