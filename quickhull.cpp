#include <set>
#include <iterator>
#include <stdexcept>

#include "quickhull.h"

QuickHull::QuickHull(const std::vector<double> & coordinates)
{
	if (coordinates.size() < 2) 
		return;

	first_run = true;

	// find left- and right-most points A and B
	double min_x, max_x;
	point p = point(coordinates.at(0), coordinates.at(1));
	l = r = p; min_x = max_x = p.x;
	for (unsigned i = 0; i < coordinates.size();) 
	{
		try
		{
			p.x = coordinates.at(i++);
			p.y = coordinates.at(i++);
		}
		catch (std::out_of_range)
		{
			break;
		}

		if (p.x < min_x) 
		{
			min_x = p.x;
			l = p;
		} 
		if (p.x > max_x) 
		{
			max_x = p.x;
			r = p;
		}
		init_points.push_back(p);
	}

	// add A and B to convex hull 
	convex_hull.push_back(l);
	std::cout << "Convex hull point found at " << l << "." << std::endl;
	if (init_points.size() <= 1)
		return;

	convex_hull.push_back(r);	
	std::cout << "Convex hull point found at " << r << "." << std::endl;
	if (init_points.size() <= 2)
		return;	

	// divide points into lower and upper set
	std::vector<point> upper_points;
	std::vector<point> lower_points;
	for(unsigned i = 0; i < init_points.size(); i++)
	{
		p = init_points.at(i);
		if (point_location(r,l,p) > 0)
			upper_points.push_back(p);
		if (point_location(r,l,p) < 0)
			lower_points.push_back(p);
	}

	//build_hull(r,l,upper_points);
	//build_hull(l,r,lower_points);
	queue.push(stack_item(l,r,lower_points));
	queue.push(stack_item(r,l,upper_points));
}

// step-by-step processing, returns 1 when done
bool QuickHull::next_step()
{
	if (queue.empty())
		return true;

	if (first_run)
	{
		first_run = false;
		convex_hull.clear();
		return false;
	}

	stack_item item = queue.top();
	processed.push_back(item.a.x);
	processed.push_back(item.a.y);
	processed.push_back(item.b.x);
	processed.push_back(item.b.y);
	triangle.clear();
	triangle.push_back(item.a.x);
	triangle.push_back(item.a.y);
	triangle.push_back(item.b.x);
	triangle.push_back(item.b.y);
	queue.pop();

	if (item.points.size() == 0)
		return false;

	std::vector<point> ac_points;
	std::vector<point> cb_points;

	point c = fartherest_point(item.a, item.b, item.points);
	convex_hull.push_back(c);
	triangle.push_back(c.x);
	triangle.push_back(c.y);
	std::cout << "Convex hull point found at " << c << "." << std::endl;

	point p;
	for (unsigned i = 0; i < item.points.size(); i++)
	{
		p = item.points.at(i);
		if (point_location(item.a,c,p) > 0)
			ac_points.push_back(p);
		if (point_location(c,item.b,p) > 0)
			cb_points.push_back(p);
	}		
	queue.push(stack_item(c, item.b, cb_points));
	queue.push(stack_item(item.a, c, ac_points));
	return false;
}

/* too complicated process of creating vector containing
   convex hull points in the right order for further easy rendering */
std::vector<double> QuickHull::get_convex_hull() const
{
	std::vector<double> hull;
	std::set<point> upper;
	std::set<point> lower;

	point p;
	for(unsigned i = 0; i < convex_hull.size(); i++)
	{
		p = convex_hull.at(i);
		if (point_location(r,l,p) > 0)
			upper.insert(p);
		if (point_location(r,l,p) < 0)
			lower.insert(p);
	}

	hull.push_back(l.x);
	hull.push_back(l.y);

	std::set<point>::const_iterator it;
	for(it = upper.begin(); it != upper.end(); ++it)
	{
		hull.push_back(it->x);
		hull.push_back(it->y);
	}

	hull.push_back(r.x);
	hull.push_back(r.y);

	std::set<point>::const_reverse_iterator rit;
	for(rit = lower.rbegin(); rit != lower.rend(); ++rit)
	{
		hull.push_back(rit->x);
		hull.push_back(rit->y);
	}
	return hull;
}

std::vector<double> QuickHull::current_points()
{
	std::vector<double> curr_points;
	if (!queue.empty())
	{
		std::vector<point> points = queue.top().points;
		for (unsigned i = 0; i < points.size(); i++)
		{
			curr_points.push_back(points.at(i).x);
			curr_points.push_back(points.at(i).y);
		}
	}
	return curr_points;
}

std::vector<double> QuickHull::current_line()
{
	std::vector<double> curr_line;
	if (!queue.empty())
	{
		point a = queue.top().a;
		point b = queue.top().b;
		curr_line.push_back(a.x);
		curr_line.push_back(a.y);
		curr_line.push_back(b.x);
		curr_line.push_back(b.y);
	}
	return curr_line;
}

// +/- if P is on the right/left from AB
inline
double QuickHull::point_location(point a, point b, point p) const
{
	return (a.x-b.x)*(p.y-b.y) - (p.x-b.x)*(a.y-b.y);
}

// distance of point P from AB
double QuickHull::distance(point a, point b, point p) const
{
	double x, y, u;
	u = ((p.x-a.x)*(b.x-a.x) + (p.y-a.y)*(b.y-a.y)) / ((b.x-a.x)*(b.x-a.x) + (b.y-a.y)*(b.y-a.y));
	x = a.x + u*(b.x-a.x);
	y = a.y + u*(b.y-a.y);
	return ((x-p.x)*(x-p.x) + (y-p.y)*(y-p.y));
}

// find most distant point from AB
point QuickHull::fartherest_point(point a, point b, const std::vector<point> & points) const
{
	point max_point;
	double max_distance;
	double point_distance;

	point_distance = max_distance = 0.0;
	max_point.x = max_point.y = 0.0;
	for (unsigned i = 0; i < points.size(); i++)
	{
		point_distance = distance(a, b, points.at(i));
		if (point_distance > max_distance)
		{
			max_distance = point_distance;
			max_point = points.at(i);
		}
	}
	return max_point;
}

void QuickHull::build_hull(point a, point b, const std::vector<point> & points)
{
	if (points.size() == 0) 
		return;

	std::vector<point> ac_points;
	std::vector<point> cb_points;

	point c = fartherest_point(a, b, points);
	convex_hull.push_back(c);
	//std::cout << "Convex hull point found at " << c << "." << std::endl;

	point p;
	for (unsigned i = 0; i < points.size(); i++)
	{
		p = points.at(i);
		if (point_location(a,c,p) > 0)
			ac_points.push_back(p);
		if (point_location(c,b,p) > 0)
			cb_points.push_back(p);
	}
	build_hull(a, c, ac_points);
	build_hull(c, b, cb_points);
}