#ifndef GIFT_WRAPPING_H_
#define GIFT_WRAPPING_H_

#include <vector>
#include "point.h"

class GiftWrappingHull
{
public:
	GiftWrappingHull(){}
	GiftWrappingHull(std::vector<double> & );

	// step-by-step processing, returns 1 when done
	bool next_step();

	// compute hull in one step
	void wrap();

	std::vector<double> get_convex_hull();
	std::vector<double> processed_lines() { return processed; }
	std::vector<double> current_line();
	std::vector<double> min_line();

private:
	std::vector<point> points;
	std::vector<point> convex_hull;
	std::vector<double> processed;

	unsigned current_position;
	point init_point;
	point min_point;
	point endpoint;
	point hull_point;
	double min_angle;

	// angle between two vectors
	double angle(point, point, point);
	void report(point);
};

#endif