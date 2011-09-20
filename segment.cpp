#include "segment.h"

segment::segment(double x1, double y1, double x2, double y2, segment_color color) : color(color)
{
	left.type = LEFT;
	right.type = RIGHT;

	left.x  = x1 < x2 ? x1 : x2;
	left.y  = x1 < x2 ? y1 : y2;
	right.x = x1 < x2 ? x2 : x1;
	right.y = x1 < x2 ? y2 : y1;

	y_sweep = 0.0;
	x0 = x1;
}

segment::segment(endpoint left, endpoint right, segment_color color) : left(left), right(right), color(color)
{
	y_sweep = 0.0;
	x0 = left.x;
}

bool segment::operator < (const segment &other) const 
{
	return y_sweep < other.y_sweep;
}

bool segment::operator > (const segment &other) const 
{
	return y_sweep > other.y_sweep;
}

bool segment::operator == (const segment &other) const
{
	return (left == other.left && right == other.right && color == other.color);
}

bool segment::operator != (const segment &other) const
{
	return !(*this == other);
}

std::ostream & operator << (std::ostream & out, const segment & s) 
{
	out << s.left << "," << s.right;
	return out;
}
