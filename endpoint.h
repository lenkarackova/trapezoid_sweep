#ifndef ENDPOINT_H_
#define ENDPOINT_H_

#include <iostream>
#include <limits>

const double infinity = std::numeric_limits<double>::infinity();

enum endpoint_type
{
	LEFT, RIGHT
};

struct endpoint
{
	double x;
	double y;
	endpoint_type type;

	bool operator < (const endpoint &) const;
	bool operator > (const endpoint &) const;
	bool operator == (const endpoint &) const;
	bool operator != (const endpoint &) const;
	friend std::ostream & operator << (std::ostream &, const endpoint &);
	endpoint(){}
	endpoint(double x, double y) : x(x), y(y) {}
};

#endif
