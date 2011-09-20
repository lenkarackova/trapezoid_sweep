#ifndef SEGMENT_H_
#define SEGMENT_H_

#include <iostream>
#include "endpoint.h"

enum segment_color
{
	BLUE, RED
};

struct segment
{
	endpoint left;
	endpoint right;
	segment_color color;
	double y_sweep;
	double x0;

	segment() {	segment(0.0, 0.0, 0.0, 0.0, RED); }
	segment(double, double, double, double, segment_color);
	segment(endpoint, endpoint, segment_color);
	bool operator < (const segment &) const;
	bool operator > (const segment &) const;
	bool operator == (const segment &) const;
	bool operator != (const segment &) const;
	friend std::ostream & operator << (std::ostream &, const segment &);
};

#endif
