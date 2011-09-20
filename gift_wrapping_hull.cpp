#include <cmath>
#include <stdexcept>
#include "gift_wrapping_hull.h"

GiftWrappingHull::GiftWrappingHull(std::vector<double> & coordinates)
{
	if (coordinates.size() < 2) 
		return;
	
	point p;
	init_point.x = coordinates.at(0);
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

		if (p.x <= init_point.x) 
			init_point = p;

		points.push_back(p);
	}

	report(init_point);
	endpoint = min_point = init_point;
	hull_point = point(init_point.x, 0);
	current_position = 0;
	//wrap();
}

// step-by-step processing, returns 1 when done
bool GiftWrappingHull::next_step()
{
	if (points.size() < 2)
		return true;

	if (current_position < points.size())
	{
		point p = points.at(current_position);
		double current_angle = angle(hull_point, endpoint, p);

		if ((current_position == 0) || (current_angle <= min_angle && endpoint != p))
		{
			min_angle = current_angle;
			min_point = p;
		}
		processed.push_back(endpoint.x);
		processed.push_back(endpoint.y);
		processed.push_back(p.x);
		processed.push_back(p.y);
		current_position++;
	}

	if (current_position >= points.size())
	{
		current_position = 0;
		hull_point = endpoint;
		endpoint = min_point;
		convex_hull.push_back(endpoint);

		if (endpoint == init_point)
			return true;
		else
			std::cout << "Convex hull point found at " << endpoint << "." << std::endl;
	}			
	return false;
}

// compute hull in one step
void GiftWrappingHull::wrap()
{
	if (points.size() < 2)
		return;

	point p;
	double current_angle;

	for(;;)
	{
		for (unsigned j = 0; j < points.size(); j++)
		{
			p = points.at(j);
		
			current_angle = angle(hull_point, endpoint, p);

			if (j == 0)
			{
				min_angle = current_angle;
				min_point = p;
			}

			if (endpoint == p)
				continue;

			if (current_angle <= min_angle)
			{
				min_angle = current_angle;
				min_point = p;
			}
		}

		hull_point = endpoint;
		endpoint = min_point;
		convex_hull.push_back(endpoint);

		if (endpoint == init_point)
			break;
		else
			std::cout << "Convex hull point found at " << endpoint << "." << std::endl;
	}
}

std::vector<double> GiftWrappingHull::get_convex_hull()
{
	std::vector<double> hull;
	for(unsigned i = 0; i < convex_hull.size(); i++)
	{
		hull.push_back(convex_hull.at(i).x);
		hull.push_back(convex_hull.at(i).y);
	}
	return hull;
}

std::vector<double> GiftWrappingHull::current_line()
{
	std::vector<double> line;
	if (points.size() > 2)
	{
		line.push_back(endpoint.x);
		line.push_back(endpoint.y);
		line.push_back(points.at(current_position).x);
		line.push_back(points.at(current_position).y);
	}
	return line;
}

std::vector<double> GiftWrappingHull::min_line()
{
	std::vector<double> line;
	if (points.size() > 2)
	{
		line.push_back(endpoint.x);
		line.push_back(endpoint.y);
		line.push_back(min_point.x);
		line.push_back(min_point.y);
	}
	return line;
}

// angle between two vectors
double GiftWrappingHull::angle(point a, point b, point c) 
{
	point v1 = point(b.x-a.x, b.y-a.y);
	point v2 = point(c.x-b.x, c.y-b.y);

	if (v1*v1 == 0 || v2*v2 == 0)
		return 3.141592;

	double x = v1*v2 / (sqrt(v1*v1) * sqrt(v2*v2));
	if (x >= 1)
		return 0;
	if (x <=-1)
		return 3.141592;

	return acos(x);
}

void GiftWrappingHull::report(point p)
{
	convex_hull.push_back(p);
	std::cout << "Convex hull point found at " << p << "." << std::endl;
}