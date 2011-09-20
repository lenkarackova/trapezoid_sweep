#ifndef QUICKHULL_H_
#define QUICKHULL_H_

#include <vector>
#include <stack>

#include "point.h"

class QuickHull
{
public: 
	QuickHull(){}
	QuickHull(const std::vector<double> &);
	bool next_step();
	std::vector<double> get_convex_hull() const;
	std::vector<double> current_points();
	std::vector<double> current_line();
	std::vector<double> processed_lines() { return processed; }
	std::vector<double> processed_triangle() { return triangle; }

private:
	std::vector<point> convex_hull;	
	std::vector<point> init_points;
	std::vector<double> processed;
	std::vector<double> triangle;
	point l,r;

	struct stack_item
	{
		point a, b;
		std::vector<point> points;

		stack_item(point a, point b, std::vector<point> points) 
			: a(a), b(b), points(points) {}
	};

	std::stack<stack_item> queue;
	bool first_run;

	double point_location(point a, point b, point p) const;
	double distance(point a, point b, point p) const;
	point fartherest_point(point, point, const std::vector<point> & ) const;
	void build_hull(point, point, const std::vector<point> & );
};

#endif