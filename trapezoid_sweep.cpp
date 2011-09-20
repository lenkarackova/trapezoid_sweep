#include <stdexcept>
#include "trapezoid_sweep.h"

TrapezoidSweep::TrapezoidSweep(const std::vector<double>& blue_endpoints, const std::vector<double>& red_endpoints)
{
	x_sweep = 0.0;
	y_min =  infinity;
	y_max = -infinity;
	NULL_POINT = endpoint(infinity, infinity);
	NULL_SEGMENT = segment(NULL_POINT, NULL_POINT, RED);

	// initialize queue..
	init_queue(blue_endpoints, BLUE);
	init_queue(red_endpoints, RED);
	current_endpoint = NULL_POINT;
}

bool TrapezoidSweep::next_step()
{
	if (current_endpoint == NULL_POINT)
	{
		done = false;
		queue_it = queue.begin();
	}

	if (queue_it == queue.end())
	{
		done = true;
		return done;
	}

	endpoint p = queue_it->first;
	segment* s = queue_it->second;

	current_segment = *s;
	current_endpoint = p;

	x_sweep = p.x;

	// update y-coordinate of intersection with sweep line
	s->y_sweep = p.y;
	update_y_sweep(L_red);
	update_y_sweep(L_blue);
	
	finished_t.insert(finished_t.end(),current_t.begin(),current_t.end());
	current_t.clear();

	if (s->color == RED || p.type == LEFT)
	{
		add_trapezoid(*search(L_blue,s, 1),*search(L_blue,s,-1));
	}
	else
	{
		add_trapezoid(*search(L_blue,s, 1),*s);
		add_trapezoid(*s,*search(L_blue,s,-1));
	}
			
	// find the nearest s_blue in both directions
	if (s->color == RED || p.type == LEFT)
	{
		advance(search(L_blue,s, 1));
		advance(search(L_blue,s,-1));
	}

	// right blue segment
	else
	{
		advance(next(L_blue,s, 1));
		advance(s);
		advance(next(L_blue,s,-1));
	}

	// update both L_red and L_blue list
	if (p.type == LEFT)
	{
		if (s->color == RED)
			insert_segment(L_red , s);
		if (s->color == BLUE)
			insert_segment(L_blue, s);
	} else {
		if (s->color == RED)
			delete_segment(L_red , s);
		if (s->color == BLUE)
			delete_segment(L_blue, s);
	}
	
	++queue_it;
	return done;
}

double TrapezoidSweep::current_endpoint_x() const
{
	return current_endpoint == NULL_POINT ? infinity : current_endpoint.x;
}

double TrapezoidSweep::current_endpoint_y() const
{
	return current_endpoint == NULL_POINT ? infinity : current_endpoint.y;
}


void TrapezoidSweep::insert_segment(std::set<segment*,set_comp>& segment_list, segment* s)
{
	segment_list.insert(s);
}

void TrapezoidSweep::delete_segment(std::set<segment*,set_comp>& segment_list, segment* s)
{
	std::set<segment*,set_comp>::iterator it = segment_list.find(s);
	if (it != segment_list.end())
		segment_list.erase(it);
}

// returns the element in list L that is just grater (or less) that s if dir = +1/-1
segment* TrapezoidSweep::search(std::set<segment*,set_comp>& segment_set, segment* s, int dir)
{
	std::set<segment*,set_comp>::const_iterator it;

	if (dir == 0 || segment_set.empty())
		return &NULL_SEGMENT;
	else if (dir < 0)
		it = --segment_set.lower_bound(s);
	else
		it = segment_set.upper_bound(s);

	if (it == segment_set.end())
		return &NULL_SEGMENT;
	else
		return *it;
}

// returns the successor / predecessor of s in list L if dir = +1/-1
segment* TrapezoidSweep::next(std::set<segment*,set_comp>& segment_set, segment* s, int dir)
{
	std::set<segment*,set_comp>::const_iterator it = segment_set.find(s);//!

	if (segment_set.empty() || it == segment_set.end())
		return &NULL_SEGMENT;

	if (dir < 0)
		--it;
	else if (dir > 0)
		++it;
	else
		return &NULL_SEGMENT;

	if (it != segment_set.end())
		return *it;
	else
		return &NULL_SEGMENT;
}

double TrapezoidSweep::intersection(segment s, double x) const
{
	double y = (s.left.y-s.right.y)/(s.left.x-s.right.x)*(x-s.left.x)+s.left.y;
	if ((y >= s.left.y && y <= s.right.y) || (y >= s.right.y && y <= s.left.y))
		return y;
	else 
		return infinity;
}

endpoint TrapezoidSweep::intersection(segment s1, segment s2) const
{
	double x, y;
	double m1, m2;
	double b1, b2;

	if (s1 == NULL_SEGMENT || s2 == NULL_SEGMENT)
	{
		return NULL_POINT;
	}

	// convert to slope-intercept form (y = mx + b), detect vertical lines
	if (s1.left.x != s1.right.x)
	{
		m1 = (s1.right.y - s1.left.y) / (s1.right.x - s1.left.x);
		b1 = s1.right.y - m1 * s1.right.x;
	} else {
		x = s1.left.x;
		m1 = 0.0;
		b1 = 0.0;
	}

	if (s2.left.x != s2.right.x)
	{
		m2 = (s2.right.y - s2.left.y) / (s2.right.x - s2.left.x);
		b2 = s2.right.y - m2 * s2.right.x;
	} else {
		x = s2.left.x;
		m2 = 0.0;
		b2 = 0.0;
	}

	// parallel lines
	if (m2 == m1)
		return NULL_POINT;

	// none of lines vertical => get x coordinate of intersction
	if ((s1.left.x != s1.right.x) && (s2.left.x != s2.right.x))
		x = (b1 - b2)/(m2 - m1);

	// get y coordinate of intersecion
	if (s2.left.x != s2.right.x)
		y = m2 * x + b2;
	else
		y = m1 * x + b1;
    
	//check the segment boundaries
	if (x < s1.left.x || x > s1.right.x || x < s2.left.x || x > s2.right.x)
		return NULL_POINT;
	else
		return endpoint(x,y);
}

//return -infinity if s_red and s_blue don't intersect
//otherwise return the x-coordinate of the intersection
double TrapezoidSweep::meet(segment s_red, segment s_blue) const
{
	endpoint p = intersection(s_red, s_blue);
	return p == NULL_POINT ? -infinity : p.x;
}
	

void TrapezoidSweep::report(const segment* s_red, const segment* s_blue)
{
	endpoint p = intersection(*s_red, *s_blue);

	if (p != NULL_POINT)
	{
		m_intersections.push_back(p.x);
		m_intersections.push_back(p.y);
	}
}

void TrapezoidSweep::update_y_sweep(std::set<segment*,set_comp> & segment_set)
{
	segment *s;
	std::set<segment*,set_comp>::iterator it;
	for (it = segment_set.begin(); it != segment_set.end(); ++it)
	{
		s = *it;
		s->y_sweep = (s->left.x != s->right.x) ? intersection(*s,x_sweep) : s->right.y;
	}
}

/* for each s*_red that intersects s*, reports the intersection of s*_red with s*
   and the intersections of s*_red with all other s*_blue to left of that intersection */
void TrapezoidSweep::advance(segment* s)
{
	segment* s_red;
	segment* s_blue;

	if (s == &NULL_SEGMENT)
		return;

	// for dir from {+1,-1}...
	int repeat = 1;
	for (int dir = 1; repeat >= 0; dir = -1)
	{
		s_red = search(L_red, s, dir);
		for (;(s_red->x0 < meet(*s_red, *s)) && (meet(*s_red, *s) < x_sweep);)
		{
			s_blue = s;
			for (;(s_red->x0 < meet(*s_red, *s_blue)) && 
				(meet(*s_red, *s_blue) < x_sweep) && 
				(s_blue != &NULL_SEGMENT);)
			{
				report(s_red, s_blue);
				s_blue = next(L_blue, s_blue, -dir);
			}
			s_red->x0 = x0_red = meet(*s_red, *s);
			s_red = next(L_red, s_red, dir);
		}
		--repeat;
	}
}

void TrapezoidSweep::init_queue(const std::vector<double> & endpoints, segment_color color)
{
	endpoint left_point;
	endpoint right_point;

	for (unsigned i = 0; i < endpoints.size();)
	{
		try
		{
			left_point.x  = endpoints.at(i++);
			left_point.y  = endpoints.at(i++);
			right_point.x = endpoints.at(i++);
			right_point.y = endpoints.at(i++);
		}
		catch (std::out_of_range)
		{
			break;
		}

		if (right_point.x == left_point.x)
		{
			left_point.x += 0.0000001;
		}

		right_point.type = RIGHT;
		left_point.type = LEFT;

		if (left_point > right_point)
		{
			right_point.type = LEFT;
			left_point.type = RIGHT;
			queue[right_point] = queue[left_point] = new segment(right_point, left_point, color);
		}
		else
		{
			queue[left_point] = queue[right_point] = new segment(left_point, right_point, color);
		}

		//update max and min
		if (right_point.y > y_max)
			y_max = right_point.y;
		if (left_point.y > y_max)
			y_max = left_point.y;
		if (right_point.y < y_min)
			y_min = right_point.y;
		if (left_point.y < y_min)
			y_min = left_point.y;
	}
}

void TrapezoidSweep::add_trapezoid(segment s_upper, segment s_lower)
{
	endpoint top_left;
	endpoint top_right;
	endpoint bottom_left;
	endpoint bottom_right;

	top_right.x = x_sweep;
	bottom_right.x = x_sweep;

	if (s_upper != NULL_SEGMENT && s_lower != NULL_SEGMENT)
	{
		top_right.y = s_upper.y_sweep;
		bottom_right.y = s_lower.y_sweep;

		if (s_upper.left > s_lower.left)
		{
			top_left.x = s_upper.left.x;
			top_left.y = s_upper.left.y;
			bottom_left.x = s_upper.left.x;
			bottom_left.y = intersection(s_lower,top_left.x);
		}
		else
		{
			top_left.x = s_lower.left.x;
			top_left.y = intersection(s_upper,top_left.x);
			bottom_left.x = s_lower.left.x;
			bottom_left.y = s_lower.left.y;
		}
	}

	else if (s_upper == NULL_SEGMENT && s_lower != NULL_SEGMENT)
	{
		top_left.x = s_lower.left.x;
		top_left.y = y_max + 30;
		top_right.y = y_max + 30;
		bottom_right.y = s_lower.y_sweep;
		bottom_left.x = s_lower.left.x;
		bottom_left.y = s_lower.left.y;
	}

	else if (s_lower == NULL_SEGMENT && s_upper != NULL_SEGMENT)
	{
		top_left.x = s_upper.left.x;
		top_left.y = s_upper.left.y;
		top_right.y = s_upper.y_sweep;
		bottom_right.y = y_min - 30;
		bottom_left.x = s_upper.left.x;
		bottom_left.y = y_min - 30;
	}

	else //both NULL_SEGMENT
	{			
		top_left.x = x_sweep;
		top_left.y = y_max + 30;
		top_right.y = y_max + 30;
		bottom_right.y = y_min - 30;
		bottom_left.x = x_sweep;
		bottom_left.y = y_min - 30;
		if (current_endpoint.type == RIGHT)
		{
			top_left.x = current_segment.left.x;
			bottom_left.x = current_segment.left.x;
		}
		else if (!queue.empty())
		{
			top_left.x = queue.begin()->first.x;
			bottom_left.x = queue.begin()->first.x;
		}
	}

	walls.push_back(top_right.x);
	walls.push_back(top_right.y);
	walls.push_back(bottom_right.x);
	walls.push_back(bottom_right.y);

	current_t.push_back(top_left.x);
	current_t.push_back(top_left.y);
	current_t.push_back(bottom_left.x);
	current_t.push_back(bottom_left.y);
	current_t.push_back(bottom_right.x);
	current_t.push_back(bottom_right.y);
	current_t.push_back(top_right.x);
	current_t.push_back(top_right.y);
}
