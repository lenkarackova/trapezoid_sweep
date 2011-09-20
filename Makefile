all:
	g++ -Wall main.cpp point.cpp endpoint.cpp segment.cpp canvas.cpp quickhull.cpp trapezoid_sweep.cpp gift_wrapping_hull.cpp -o trapezoid_sweep `wx-config --cppflags --libs --gl-libs` -lGL
