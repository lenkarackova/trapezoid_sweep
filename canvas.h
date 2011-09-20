#ifndef CANVAS_H_
#define CANVAS_H_

#include <wx/glcanvas.h>
#include <vector>

#include "trapezoid_sweep.h"
#include "quickhull.h"
#include "gift_wrapping_hull.h"

const short int TIMER_ID = 301;

enum mode
{
	TRAPEZOID,
	QUICKHULL,
	GIFT
};

class Canvas: public wxGLCanvas
{
public:
	Canvas (wxWindow*);
	~Canvas(){}
	void on_timer(wxTimerEvent&);
	void set_mode(mode);
	void clear();
	void start();
	void switch_color();

protected:
	void render();
	void on_resize(wxSizeEvent&);
	void mouse_moved(wxMouseEvent&);
	void mouse_right_click(wxMouseEvent&);
	void mouse_left_click(wxMouseEvent&);
	void mouse_left_window(wxMouseEvent&);
	void mouse_endered_window(wxMouseEvent&);
	void on_key_pressed(wxKeyEvent& event) {}
	void on_paint(wxPaintEvent& WXUNUSED(event)) { render(); }
	void on_erase_background(wxEraseEvent& WXUNUSED(event)){}	
	void on_idle(wxIdleEvent & event) {}

private:		
	std::vector<double> red_endpoints;
	std::vector<double> blue_endpoints;
	std::vector<double> hull_points;

	TrapezoidSweep trapezoid;
	QuickHull quickhull;
	GiftWrappingHull gift;

	wxTimer timer;	
	segment new_segment;
	segment_color color_mode;
	mode current_mode;

	bool initialized;
	bool drawing;
	bool drawing_segment;
	bool display_guideline;
	bool display_solution;
	double mouse_x;
	double mouse_y;
	int win_width;
	int win_height;

	void init_gl();
	void new_preview() { Refresh(); }
	void set_projection_matrix();
	void set_modelview_matrix();
	void render_trapezoid_sweep();
	void render_gift_hull();
	void render_quickhull();
	void render_hull_points();
	void render_segments();
	void render_guideline();
	void trapezoid_sweep();
	void quick_hull();
	void gift_wrapping_hull();
	void save_new_segment();

	DECLARE_EVENT_TABLE()
};

#endif