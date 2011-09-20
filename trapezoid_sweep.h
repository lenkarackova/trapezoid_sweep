#ifndef TRAPEZOID_SWEEP_H_
#define TRAPEZOID_SWEEP_H_

#include <vector>
#include <set>
#include <map>
#include <iterator>

#include "segment.h"

class TrapezoidSweep
{
public:
	TrapezoidSweep(){}
	TrapezoidSweep(const std::vector<double>&, const std::vector<double>&);
	~TrapezoidSweep(){}

	bool next_step();
	double sweepline_x() const { return x_sweep; }
	double x_red() const { return x0_red; }
	std::vector<double> intersections() const { return m_intersections; }
	std::vector<double> current() const { return current_t; }
	std::vector<double> finished() const { return finished_t; }
	std::vector<double> trapezoid_walls() const { return walls; }
	segment_color current_segment_color() const { return current_segment.color; }

	// sweeps the endpoints from left to right
	bool sweep() { for (;!next_step();); return true; }
	
	double current_endpoint_x() const;
	double current_endpoint_y() const;

private:
	std::map<endpoint,segment*> queue;	// queue lexicographically sorted by a point coordinate
	double x_sweep;				// x-coordinate of the sweep line
	double x0_red;				// largest x-coordinate of the reported intersection of s_red

	struct set_comp
	{
		bool operator () (const segment *s1, const segment *s2) const
		{
			return s1->y_sweep < s2->y_sweep;
		}
	};

	// lists of segments intersecting the sweep line
	// ordered by y-intersection with x_sweep
	std::set<segment*, set_comp> L_red;
	std::set<segment*, set_comp> L_blue;

	std::vector<double> m_intersections;	// intersections found so far
	std::vector<double> finished_t;		// closed trapezoids
	std::vector<double> current_t;		// trapezoids being processed
	std::vector<double> walls;

	double y_min, y_max;
	bool done;					//sweeping finished
	std::map<endpoint,segment*>::iterator queue_it; //iterator to current endpoint
	endpoint current_endpoint;			//endpoint being processed
	segment current_segment;			//segment being processed
	endpoint NULL_POINT;
	segment NULL_SEGMENT;

	void insert_segment(std::set<segment*,set_comp>&, segment*);
	void delete_segment(std::set<segment*,set_comp>&, segment*);

	// returns the element in list L that is just grater (or less) that s if dir = +1/-1
	segment* search(std::set<segment*,set_comp>&, segment*, int);

	// returns the successor / predecssor of s in list L if dir = +1/-1
	segment* next(std::set<segment*,set_comp>&, segment*, int);

	double intersection(segment, double) const;
	endpoint intersection(segment, segment) const;

	//return -infinity if s_red and s_blue don't intersect
	//otherwise return the x-coordinate of the intersection
	double meet(segment, segment) const;

	void report(const segment*, const segment*);
	void update_y_sweep(std::set<segment*,set_comp>&);

	/* for each s*_red that intersects s*, reports the intersection of s*_red with s*
	   and the intersections of s*_red with all other s*_blue to left of that intersection */
	void advance(segment*);

	void init_queue(const std::vector<double>&, segment_color);
	void add_trapezoid(segment, segment);
};

#endif
