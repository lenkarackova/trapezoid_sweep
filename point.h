#ifndef POINT_H_
#define POINT_H_

#include <iostream>

struct point
{
	double x;
	double y;

	bool operator < (const point other) const;
	bool operator > (const point other) const;
	bool operator == (const point other) const;
	bool operator != (const point other) const;
	double operator * (const point other);

	friend std::ostream & operator << (std::ostream & out, const point & p);

	point(){}
	point(double x, double y) : x(x), y(y) {}
};

#endif