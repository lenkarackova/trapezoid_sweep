#include "endpoint.h"

bool endpoint::operator < (const endpoint &other) const
{
	if (x < other.x)
		return true;
	else if (x == other.x)
		return y < other.y;
	else
		return false;
}

bool endpoint::operator > (const endpoint &other) const
{
	if (x > other.x)
		return true;
	else if (x == other.x)
		return y > other.y;
	else
		return false;
}

bool endpoint::operator == (const endpoint &other) const
{
	return (x == other.x && y == other.y);
}

bool endpoint::operator != (const endpoint &other) const
{
	return !(*this == other);
}

std::ostream & operator << (std::ostream & out, const endpoint & p)
{
	out << "[" << p.x << "," << p.y << "]";
	return out;
}
