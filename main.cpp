#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/splitter.h>
#include <wx/font.h>
#include <iostream>

#include "canvas.h"
#include "sample.xpm"

const unsigned WIN_WIDTH = 850;
const unsigned WIN_HEIGHT = 650;

const short int ID_WINDOW = 101;
const short int ID_HELP	= 201;
const short int ID_GIFT = 301;
const short int ID_QUICKHULL = 302;
const short int ID_TRAPEZOID = 303;
const short int ID_START = 401;
const short int ID_CLEAR = 402;
const short int ID_SWITCH_COLOR = 403;


class MainWindow: public wxFrame
{
	void SetCanvas(Canvas *c ) { canvas = c; }
	Canvas *GetCanvas() { return canvas; }

private:
	Canvas *canvas;	
	wxTextCtrl *text;
	wxStreamToTextRedirector* redirector;

public:
	MainWindow(wxWindow *parent, wxWindowID id, const wxString& title, 
			   const wxPoint& pos, const wxSize& size)
			   : wxFrame(parent, id, title, pos, size)
	{
		SetIcon(wxIcon(sample_xpm));

		wxMenu *menu_file = new wxMenu;
		menu_file->Append(wxID_EXIT, wxT("E&xit\tAlt+F4"));

		wxMenu *menu_algorithm = new wxMenu;
		menu_algorithm->Append(ID_TRAPEZOID, wxT("&Trapezoid Sweep\tAlt+T"));
		menu_algorithm->Append(ID_QUICKHULL, wxT("&QuickHull\tAlt+Q"));
		menu_algorithm->Append(ID_GIFT, wxT("&Gift Wrapping\tAlt+G"));

		wxMenu *menu_plane = new wxMenu;
		menu_plane->Append(ID_START, wxT("&Start\tEnter"));
		menu_plane->Append(ID_CLEAR, wxT("&Clear\tDel"));
		menu_plane->Append(ID_SWITCH_COLOR, wxT("S&witch color\tTab"));

		wxMenu *menu_help = new wxMenu;		
		menu_help->Append(wxID_ABOUT, wxT("&About...\tAlt+A"));

		wxMenuBar *menu_bar = new wxMenuBar;
		menu_bar->Append(menu_file, wxT("&File"));		
		menu_bar->Append(menu_algorithm,wxT("&Algorithm"));
		menu_bar->Append(menu_plane,wxT("&Plane"));
		menu_bar->Append(menu_help, wxT("&Help"));

		SetMenuBar(menu_bar);

		CreateStatusBar(2);
		SetStatusText(wxT("PA093 Project"));

		wxSplitterWindow *splitter = new wxSplitterWindow(this);

		text = new wxTextCtrl(splitter, wxID_ANY, wxEmptyString,
			wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
		redirector = new wxStreamToTextRedirector(text);
		
		canvas = new Canvas(splitter);
		
		splitter->SplitHorizontally(canvas, text, -120);
		splitter->SetSashGravity(1.0);
		splitter->SetMinimumPaneSize(20);
		
		text->SetEditable(false);

		std::cout << "Created by Bc. Lenka Rackova as a part of PA093 project "
			<< "using C++, OpenGL, GLUT and wxWidgets." << std::endl << std::endl;
		std::cout << "This program implements following computational "
			<< "geometry algorithms:" << std::endl;
		std::cout << "- A Simple Trapezoid Sweep Algorithm for Reporting Red/Blue "
			<< "Segment Intersections by Timothy M. Chan." << std::endl;
		std::cout << "- QuickHull algorithm for computing the convex hull "
			<< "of a given set of points in two dimensions." << std::endl;
		std::cout << "- Gift Wrapping algorithm for computing the convex hull "
			<< "of a given set of points in two dimenstions, also known as Jarvis March." 
			<< std::endl << std::endl;		

		canvas->set_mode(QUICKHULL);
		canvas->SetFocus();
	}

	void on_menu_plane_start(wxCommandEvent& WXUNUSED(event))
	{
		canvas->start();
	}

	void on_menu_plane_clear(wxCommandEvent& WXUNUSED(event))
	{
		canvas->clear();
	}

	void on_menu_plane_switch(wxCommandEvent& WXUNUSED(event))
	{
		canvas->switch_color();
	}

	void on_menu_file_exit(wxCommandEvent& WXUNUSED(event))
	{
		Close(true);
	}

	void on_menu_help_about(wxCommandEvent& WXUNUSED(event))
	{
		wxMessageBox(wxT("PA093 Project by Lenka Rackova"),
			wxT("About..."), wxOK | wxICON_INFORMATION, this);
	}

	void on_menu_algorithm_trapezoid(wxCommandEvent& WXUNUSED(event))
	{		
		canvas->set_mode(TRAPEZOID);
	}

	void on_menu_algorithm_quickhull(wxCommandEvent& WXUNUSED(event))
	{
		canvas->set_mode(QUICKHULL);
	}

	void on_menu_algorithm_gift(wxCommandEvent& WXUNUSED(event))
	{
		canvas->set_mode(GIFT);
	}

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(MainWindow, wxFrame)
	EVT_MENU(ID_CLEAR, MainWindow::on_menu_plane_clear)
	EVT_MENU(ID_START, MainWindow::on_menu_plane_start)
	EVT_MENU(ID_SWITCH_COLOR, MainWindow::on_menu_plane_switch)
	EVT_MENU(wxID_EXIT, MainWindow::on_menu_file_exit)
	EVT_MENU(wxID_ABOUT, MainWindow::on_menu_help_about)
	EVT_MENU(ID_TRAPEZOID, MainWindow::on_menu_algorithm_trapezoid)
	EVT_MENU(ID_QUICKHULL, MainWindow::on_menu_algorithm_quickhull)
	EVT_MENU(ID_GIFT, MainWindow::on_menu_algorithm_gift)
END_EVENT_TABLE()


class PA093_Project: public wxApp
{
	virtual bool OnInit()
	{
		MainWindow *main_window = new MainWindow(NULL, ID_WINDOW, wxT("PA093 Project"), wxPoint(50,50), wxSize(WIN_WIDTH,WIN_HEIGHT));
		main_window->Show(true);
		return true;
	}
};

IMPLEMENT_APP(PA093_Project)
