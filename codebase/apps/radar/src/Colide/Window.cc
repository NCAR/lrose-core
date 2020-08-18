/**
 * @file Window.cc
 */

#define WINDOW_MAIN
#include "Window.hh"
#undef WINDOW_MAIN

#include <cstdio>
#include <math.h>

/*----------------------------------------------------------------*/
// default constructor
Window::Window()
{
  _length = _width = _step = 0.0;
  _npt = _nang = 0;
}

/*----------------------------------------------------------------*/
Window::Window(double len, double wid)
{
  _length = len;
  _width = wid;

  _step = _length/(double)(window::MAX_NP - 1);
  if (_step < 1.2)
    _step = 1.2;
  _nang = (int)(3.141592653589 / atan(_width/_length));
  if (_nang > window::MAX_NANG)
    _nang = window::MAX_NANG;
  _npt = (int)((_length/_step) + .5) + 1;
  if (_npt > window::MAX_NP)
    _npt = window::MAX_NP;
}

/*----------------------------------------------------------------*/
Window::Window(double len, double wid, int nangles)
{
  _length = len;
  _width = wid;

  _step = _length/(double)(window::MAX_NP - 1);
  if (_step < 1.2)
    _step = 1.2;
  _nang = nangles;
  if (_nang > window::MAX_NANG)
    _nang = window::MAX_NANG;
  _npt = (int)((_length/_step) + .5) + 1;
  if (_npt > window::MAX_NP)
    _npt = window::MAX_NP;
}

/*----------------------------------------------------------------*/
Window::~Window()
{
}

