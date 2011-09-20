#include <wx/wx.h>
#include <wx/utils.h>

#if !wxUSE_GLCANVAS
#error "OpenGL required: set wxUSE_GLCANVAS to 1 and rebuild the library"
#endif

#ifdef __DARWIN__
	#include <OpenGL/glu.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif

#include <algorithm>
#include <iterator>
#include <iostream>

#include "canvas.h"

BEGIN_EVENT_TABLE(Canvas, wxGLCanvas)
	EVT_SIZE(Canvas::on_resize)
	EVT_PAINT(Canvas::on_paint)
	EVT_ERASE_BACKGROUND(Canvas::on_erase_background)
	EVT_MOTION(Canvas::mouse_moved)
	EVT_LEFT_DOWN(Canvas::mouse_left_click)
	EVT_RIGHT_DOWN(Canvas::mouse_right_click)
	EVT_LEAVE_WINDOW(Canvas::mouse_left_window)
	EVT_ENTER_WINDOW(Canvas::mouse_endered_window)
	EVT_KEY_DOWN(Canvas::on_key_pressed)
	EVT_IDLE(Canvas::on_idle)
	EVT_TIMER(TIMER_ID, Canvas::on_timer)
END_EVENT_TABLE()

Canvas::Canvas(wxWindow* parent)
	: wxGLCanvas(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
	  wxBORDER_NONE|wxFULL_REPAINT_ON_RESIZE, wxT("canvas")), timer(this, TIMER_ID)
{
	current_mode = TRAPEZOID;
	color_mode = BLUE;
	drawing_segment = false;
	display_guideline = false;
	drawing = true;
	display_solution = false;
}

void Canvas::on_timer(wxTimerEvent& event)
{

	if ((current_mode == TRAPEZOID && trapezoid.next_step()) ||
		(current_mode == GIFT && gift.next_step()) ||
		(current_mode == QUICKHULL && quickhull.next_step()))
	{
		timer.Stop();
		std::cout << "Finished." << std::endl;
		display_guideline = true;
	}

	new_preview();
}

void Canvas::set_mode(mode m)
{
	current_mode = m;
	drawing = true;
	display_solution = false;
	if (m == TRAPEZOID)
		std::cout << "Trapezoid Sweep mode active. Left click to draw new segment, right click to switch color." << std::endl;
	if (m == QUICKHULL)
		std::cout << "QuickHull mode active. Left click to draw new point." << std::endl;
	if (m == GIFT)
		std::cout << "Gift Wrapping mode active. Left click to draw new point." << std::endl;
	new_preview();
}

void Canvas::clear()
{
	std::cout << "Plane cleared." << std::endl;
	drawing = true;
	display_solution = false;
	hull_points.clear();
	red_endpoints.clear();
	blue_endpoints.clear();
	new_preview();
}

void Canvas::start()
{
	drawing = false;
	display_solution = true;
	display_guideline = false;
	std::cout << "Started." << std::endl;
	if (current_mode == TRAPEZOID)
		trapezoid_sweep();
	if (current_mode == QUICKHULL)
		quick_hull();
	if (current_mode == GIFT)
		gift_wrapping_hull();
	drawing = true;
}

void Canvas::switch_color()
{
	if (color_mode == BLUE)
	{
		color_mode = RED;
		new_segment.color = RED;
		std::cout << "Switched from blue to red." << std::endl;
	}
	else
	{
		color_mode = BLUE;
		new_segment.color = BLUE;
		std::cout << "Switched from red to blue." << std::endl;
	}
}

void Canvas::render()
{
	wxPaintDC dc(this);

#ifndef __WXMOTIF__
	if (!GetContext()) return;
#endif

	SetCurrent();

	// Initialize OpenGL
	if (!initialized)
	{
		init_gl();
		initialized = true;
	}

	glEnable(GL_DEPTH_TEST);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPointSize(8);
	glEnable(GL_POINT_SMOOTH);
	//glEnable(GL_LINE_SMOOTH);
	
	set_projection_matrix();
	set_modelview_matrix();

	render_guideline();

	if (display_solution)
	{
		if (current_mode == TRAPEZOID)
			render_trapezoid_sweep();
		else if (current_mode == GIFT)
			render_gift_hull();
		else
			render_quickhull();
	}

	else
	{			
		if (current_mode == TRAPEZOID)
			render_segments();
		else
			render_hull_points();
	}

	glFlush();
	SwapBuffers();
}

void Canvas::on_resize(wxSizeEvent& event)
{
	// update the context on some platforms
	wxGLCanvas::OnSize(event);

	// reset OpenGL view aspect
	set_projection_matrix();
	Refresh();
}

void Canvas::mouse_moved(wxMouseEvent& event)
{
	mouse_x = event.GetX();
	mouse_y = win_height - event.GetY();

	if (drawing) 
	{
		Refresh(false);
	}

	if (drawing_segment) 
	{
		new_segment.right.x = mouse_x;
		new_segment.right.y = mouse_y;
		Refresh(false);
	}
}

void Canvas::mouse_right_click(wxMouseEvent& event)
{
	if (current_mode == TRAPEZOID)
	{
		if (drawing_segment)
			drawing_segment = false;
		else
			switch_color();
	}
}

void Canvas::mouse_left_click(wxMouseEvent& event)
{	
	if (!timer.IsRunning())
		display_solution = false;

	if (current_mode == GIFT || current_mode == QUICKHULL)
	{
		std::cout << "New point [" << mouse_x << "," << mouse_y << "]." << std::endl;
		hull_points.push_back(mouse_x);
		hull_points.push_back(mouse_y);
		Refresh(false);
	}

	if (current_mode == TRAPEZOID)
	{
		if (!drawing_segment)
		{
			new_segment.color = color_mode;
			new_segment.left.x = mouse_x;
			new_segment.left.y = mouse_y;
			new_segment.right.x = mouse_x;
			new_segment.right.y = mouse_y;
			drawing_segment = true;
		}
		else
		{
			new_segment.right.x = mouse_x;
			new_segment.right.y = mouse_y;
			drawing_segment = false;
			save_new_segment();
		}
		Refresh(false);
	}
}

void Canvas::mouse_left_window(wxMouseEvent& event)
{
	display_guideline = false;
	Refresh(false);
}

void Canvas::mouse_endered_window(wxMouseEvent& event)
{
	if (!timer.IsRunning())
	{
		display_guideline = true;
		Refresh(false);
	}
}

void Canvas::init_gl()
{
	glDepthFunc(GL_LESS);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Canvas::set_projection_matrix()
{
	GetClientSize(&win_width, &win_height);

	SetCurrent();
	glViewport(0,0,(GLint)win_width,(GLint)win_height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0,win_width,0,win_height);
}

void Canvas::set_modelview_matrix()
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();	
}

void Canvas::render_trapezoid_sweep()
{		
	// point being processed
	if (timer.IsRunning())
	{
		glPointSize(5);
		if (trapezoid.current_segment_color() == RED)
			glColor3ub(255, 0, 0);
		else 
			glColor3ub(0, 0, 255);
		glBegin(GL_POINTS);
		glVertex2d(trapezoid.current_endpoint_x(),trapezoid.current_endpoint_y());
		glEnd();
	}

	// intersections (black)
	glEnableClientState(GL_VERTEX_ARRAY);
	const std::vector<double> & intersections = trapezoid.intersections();
	if (!intersections.empty())
	{
		timer.IsRunning() ? glPointSize(4) : glPointSize(5);
		glColor3f(0.0f, 0.0f, 0.0f);
		glVertexPointer(2, GL_DOUBLE, 0, &intersections[0]);
		glDrawArrays(GL_POINTS,0,(GLsizei)intersections.size()/2);
	}
	glDisableClientState(GL_VERTEX_ARRAY);

	render_segments();

	glEnableClientState(GL_VERTEX_ARRAY);
	// sweep line (black)
	if (timer.IsRunning())
	{		
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0xf0f0);
		glColor3ub(120, 120, 120);
		glBegin(GL_LINES);
		glVertex2d(trapezoid.sweepline_x(),0.0);
		glVertex2d(trapezoid.sweepline_x(),win_height);
		glEnd();
		glDisable(GL_LINE_STIPPLE);
	}

	//trapezoid walls (grey)
	if (timer.IsRunning())
		glColor3ub(220, 220, 220);
	else
		glColor3ub(235, 235, 235);
	const std::vector<double> & walls = trapezoid.trapezoid_walls();
	if (!walls.empty())
	{
		//glEnable(GL_LINE_STIPPLE);
		//glLineStipple(1,0xf0f0);
		glVertexPointer(2, GL_DOUBLE, 0, &walls[0]);
		glDrawArrays(GL_LINES,0,(GLsizei)walls.size()/2);
		//glDisable(GL_LINE_STIPPLE);
	}

	//finished trapezoids (grey)
	if (timer.IsRunning())
		glColor3ub(243, 243, 243);
	else
		glColor3ub(255, 255, 255);
	const std::vector<double> & finished = trapezoid.finished();
	if (!finished.empty())
	{
		glVertexPointer(2, GL_DOUBLE, 0, &finished[0]);
		glDrawArrays(GL_QUADS,0,(GLsizei)finished.size()/2);
	}

	// current trapezoids (yellow)
	if (timer.IsRunning())
	{
		const std::vector<double> & current = trapezoid.current();
		if (!current.empty())
		{
			glColor3ub(255, 255, 200);
			glVertexPointer(2, GL_DOUBLE, 0, &current[0]);
			glDrawArrays(GL_QUADS,0,(GLsizei)current.size()/2);
		}
	}

	glDisableClientState(GL_VERTEX_ARRAY);
}

void Canvas::render_gift_hull()
{
	// points being processed
	const std::vector<double> & curr_line = gift.current_line();
	const std::vector<double> & min_line = gift.min_line();
	if (curr_line.size() > 3 && min_line.size() > 3  && timer.IsRunning())
	{
		glPointSize(7);
		glBegin(GL_POINTS);
			// endpoint (red)
			glColor3ub(255, 60, 50);
			glVertex2d(curr_line[0],curr_line[1]);
			// min point (red)
			glVertex2d(min_line[2],min_line[3]);
			// current point (blue)
			glColor3ub(140, 170, 255);
			glVertex2d(curr_line[2],curr_line[3]);
		glEnd();
	}

	render_hull_points();

	// line being processed (blue)
	if (curr_line.size() > 3 && timer.IsRunning())
	{
		//glColor3ub(81, 244, 0);
		glColor3ub(140, 170, 255);
		glBegin(GL_LINES);
			glVertex2d(curr_line[0],curr_line[1]);
			glVertex2d(curr_line[2],curr_line[3]);
		glEnd();
	}

	// min line found so far (red)
	if (min_line.size() > 3  && timer.IsRunning())
	{
		glColor3ub(255, 60, 50);
		glBegin(GL_LINES);
			glVertex2d(min_line[0],min_line[1]);
			glVertex2d(min_line[2],min_line[3]);
		glEnd();
	}

	glEnableClientState(GL_VERTEX_ARRAY);

	// convex hull border (red)
	const std::vector<double> & hull = gift.get_convex_hull();
	if (!hull.empty())
	{
		glColor3ub(255, 160, 150);
		glVertexPointer(2, GL_DOUBLE, 0, &hull[0]);

		// close line when finished
		if (timer.IsRunning())
			glDrawArrays(GL_LINE_STRIP,0,(GLsizei)hull.size()/2);
		else
			glDrawArrays(GL_LINE_LOOP,0,(GLsizei)hull.size()/2);
	}

	// all connections processed so far (grey)
	const std::vector<double> & processed = gift.processed_lines();
	if (timer.IsRunning())
		glColor3ub(220, 220, 220);
	else
		glColor3ub(235, 235, 235);
	if (!processed.empty())
	{
		glVertexPointer(2, GL_DOUBLE, 0, &processed[0]);
		glDrawArrays(GL_LINES,0,(GLsizei)processed.size()/2);
	}
	glDisableClientState(GL_VERTEX_ARRAY);
}

void Canvas::render_quickhull()
{
	// points to be processed in the next step (blue)
	const std::vector<double> & current_points = quickhull.current_points();
	glEnableClientState(GL_VERTEX_ARRAY);
	if (!current_points.empty())
	{
		glPointSize(5);
		glColor3ub(140, 170, 255);
		glVertexPointer(2, GL_DOUBLE, 0, &current_points[0]);
		glDrawArrays(GL_POINTS,0,(GLsizei)current_points.size()/2);
	}
	glDisableClientState(GL_VERTEX_ARRAY);

	const std::vector<double> & line = quickhull.current_line();
	if (line.size() > 3)
	{
		glColor3ub(140, 170, 255);
		glBegin(GL_POINTS);
			glVertex2d(line[0],line[1]);
			glVertex2d(line[2],line[3]);
		glEnd();
	}

	render_hull_points();
	
	// current line (blue)
	if (line.size() > 3)
	{
		glColor3ub(140, 170, 255);
		glBegin(GL_LINES);
			glVertex2d(line[0],line[1]);
			glVertex2d(line[2],line[3]);
		glEnd();
	}

	// convex hull border found so far
	glEnableClientState(GL_VERTEX_ARRAY);
	const std::vector<double> & hull = quickhull.get_convex_hull();
	if (!hull.empty())
	{
		glColor3ub(255, 160, 150);
		glVertexPointer(2, GL_DOUBLE, 0, &hull[0]);
		glDrawArrays(GL_LINE_LOOP,0,(GLsizei)hull.size()/2);
	}

	// all connections processed so far (grey)
	const std::vector<double> & processed = quickhull.processed_lines();
	if (timer.IsRunning())
		glColor3ub(220, 220, 220);
	else
		glColor3ub(235, 235, 235);
	if (!processed.empty())
	{
		glVertexPointer(2, GL_DOUBLE, 0, &processed[0]);
		glDrawArrays(GL_LINES,0,(GLsizei)processed.size()/2);
	}		
	glDisableClientState(GL_VERTEX_ARRAY);

	const std::vector<double> & triangle = quickhull.processed_triangle();
	if (triangle.size() > 5 && timer.IsRunning())
	{
		glColor3ub(243, 243, 255);
		glBegin(GL_TRIANGLES);
			glVertex2d(triangle[0],triangle[1]);
			glVertex2d(triangle[2],triangle[3]);
			glVertex2d(triangle[4],triangle[5]);
		glEnd();
	}
}

void Canvas::render_hull_points()
{
	if (hull_points.empty())
		return;
	glEnableClientState(GL_VERTEX_ARRAY);
	glPointSize(5);
	glColor3f(0.0f, 0.0f, 0.0f);
	glVertexPointer(2, GL_DOUBLE, 0, &hull_points[0]);
	glDrawArrays(GL_POINTS,0,(GLsizei)hull_points.size()/2);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void Canvas::render_segments()
{
	glEnableClientState(GL_VERTEX_ARRAY);
	// blue segments
	if (!blue_endpoints.empty())
	{
		glColor3ub(140, 170, 255);
		glVertexPointer(2, GL_DOUBLE, 0, &blue_endpoints[0]);
		glDrawArrays(GL_LINES,0,(GLsizei)blue_endpoints.size()/2);
	}

	// red segments	
	if (!red_endpoints.empty())
	{
		glColor3ub(255, 170, 140);
		glVertexPointer(2, GL_DOUBLE, 0, &red_endpoints[0]);
		glDrawArrays(GL_LINES,0,(GLsizei)red_endpoints.size()/2);
	}
	glDisableClientState(GL_VERTEX_ARRAY);

	// new segment
	if (drawing_segment)
	{
		if (new_segment.color == RED)
			glColor3ub(255, 170, 140);
		else
			glColor3ub(140, 170, 255);
		glBegin(GL_LINES);
		glVertex2d(new_segment.left.x, new_segment.left.y);
		glVertex2d(mouse_x,mouse_y);
		glEnd();
	}
}

void Canvas::render_guideline()
{
	if (!display_guideline)
		return;

	int w, h;
	GetClientSize(&w, &h);
	glEnable(GL_LINE_STIPPLE);
	glColor3ub(200, 200, 200);
	glLineStipple(2,0xcccc);
	glBegin(GL_LINES);
		glVertex2d(0.0,mouse_y);
		glVertex2d(w  ,mouse_y);
		glVertex2d(mouse_x,  h);
		glVertex2d(mouse_x,0.0);
	glEnd();
	glDisable(GL_LINE_STIPPLE);
}

void Canvas::trapezoid_sweep()
{
	trapezoid = TrapezoidSweep(blue_endpoints, red_endpoints);
	//trapezoid.sweep();
	timer.Start(500);
}

void Canvas::quick_hull()
{
	quickhull = QuickHull(hull_points);
	timer.Start(700);
}

void Canvas::gift_wrapping_hull()
{
	gift = GiftWrappingHull(hull_points);
	timer.Start(300);
}

void Canvas::save_new_segment()
{
	if (new_segment.color == RED)
	{
		red_endpoints.push_back(new_segment.left.x);
		red_endpoints.push_back(new_segment.left.y);
		red_endpoints.push_back(new_segment.right.x);
		red_endpoints.push_back(new_segment.right.y);
	}
	else
	{
		blue_endpoints.push_back(new_segment.left.x);
		blue_endpoints.push_back(new_segment.left.y);
		blue_endpoints.push_back(new_segment.right.x);
		blue_endpoints.push_back(new_segment.right.y);
	}
	std::cout << "New " << ((new_segment.color == RED) ? "red" : "blue")
			  << " segment " << new_segment << "." << std::endl;
}
