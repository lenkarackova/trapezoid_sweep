#include "point.h"

bool point::operator < (const point other) const
{ 
	if (x < other.x)
		return true;
	else if (x == other.x) 
		return y < other.y;
	else
		return false;
}

bool point::operator > (const point other) const
{
	if (x > other.x)
		return true;
	else if (x == other.x) 
		return y > other.y;
	else
		return false;
}

bool point::operator == (const point other) const
{
	return (x == other.x && y == other.y);
}

bool point::operator != (const point other) const
{
	return !(*this == other);
}

double point::operator * (const point other)
{
	return x*other.x + y*other.y;
}

std::ostream & operator << (std::ostream & out, const point & p) 
{
	out << "[" << p.x << "," << p.y << "]";
	return out;
}