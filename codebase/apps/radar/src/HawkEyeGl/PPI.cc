// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#include "PPI.hh"
#include <cmath>
#include <iostream>
#include <fstream>
#include <toolsa/toolsa_macros.h>

#include <qtimer.h>
#include <QResizeEvent>
#include <QPalette>
#include <QPaintEngine>
#include <QBrush>
#include <QPen>

#include <GL/glut.h>

using namespace std;

const double PPI::degToRad = M_PI / 180.0;
const double PPI::radToDeg = 180.0 / M_PI;

//
//
// Any drawing action must insure that the GL context is current. paintGL() and
// resize can be called by QGLWidget, and it will make sure that the context
// is current. However, external calls to zoom, pan, draw a new beam,
// and so forth will not have come through QGLWidget's GL code, and so we need
// to explicitly make sure that the context is current in these cases.
//
//
////////////////////////////////////////////////////////////////

PPI::beam::beam(double startAngle,
                double stopAngle,
                int nGates,
                int nVars):
  _startAngle(startAngle), _stopAngle(stopAngle), _nVars(nVars),
  _nGates(nGates)
{
  float cos1 = cos(startAngle * degToRad)/nGates;
  float sin1 = sin(startAngle * degToRad)/nGates;
  float cos2 = cos(stopAngle * degToRad)/nGates;
  float sin2 = sin(stopAngle * degToRad)/nGates;

  // now calculate the vertex values, to be used for all variables
  for (int j = 0; j < _nGates; j++) {
    _triStripVertices.push_back(j*sin1);
    _triStripVertices.push_back(j*cos1);
    _triStripVertices.push_back(j*sin2);
    _triStripVertices.push_back(j*cos2);
  }
  // Allocate space for the colors. Each vertex has an red, green and
  // blue component, and there are 2 vertices per gate.
  _varColors.resize(nVars);
  for (int v = 0; v < nVars; v++) {
    _varColors[v].resize(_nGates*6);
  }
  // there will be one display list id for each variable
  for (int i = 0; i < _nVars; i++) {
    ///@todo add test to insure that the list has been created
    GLuint id = glGenLists(1);
    _glListId.push_back(id);
  }
}
////////////////////////////////////////////////////////////////

PPI::beam::~beam()
{
  for (unsigned int i = 0; i < _glListId.size(); i++)
    glDeleteLists(_glListId[i], 1);

  _glListId.clear();
  _varColors.clear();
  _triStripVertices.clear();
}

////////////////////////////////////////////////////////////////
GLfloat* PPI::beam::vertices()
{
  return &_triStripVertices[0];
}

////////////////////////////////////////////////////////////////
GLfloat* PPI::beam::colors(int varN)
{
  return &(_varColors[varN])[0];
}

////////////////////////////////////////////////////////////////
bool PPI::glutInitialized = false;

PPI::PPI(QWidget* parent, const Params &params):
        QGLWidget(parent), _parent(parent),
        _params(params),
        _decimationFactor(1), _selectedVar(0), _zoom(1.0),
        _centerX(0.0), _centerY(0.0),
        _gridRingsColor(_params.grid_and_range_ring_color),
        _backgroundColor(_params.background_color),
        _ringsEnabled(true),
        _gridsEnabled(false), _resizing(false),
        _scaledLabel(ScaledLabel::DistanceEng), _configured(false),
        _rubberBand(0), _cursorZoom(true)
{

  _azLinesEnabled = false;
  _clickX = -99;
  _clickY = -99;
  initializeGL();

  if (!glutInitialized) {
    int argc = 1;
    char* argv[2];
    argv[0] = (char*)("dummy");
    argv[1] = 0;

    glutInit(&argc, argv);
    glutInitialized = true;
  }

  this->setAutoBufferSwap(false);
  
  // create the rubber band
  _rubberBand = new QRubberBand(QRubberBand::Rectangle, this);

  // connect the resize timer
  _resizeTimer.setSingleShot(true);
  connect(&_resizeTimer, SIGNAL(timeout()), this, SLOT(resizeTimerTimeout()));
}
////////////////////////////////////////////////////////////////

void PPI::configure(int nVars,
                    int maxGates,
                    double distanceSpanKm,
                    int decimationFactor,
                    double left,
                    double right,
                    double bottom,
                    double top)
{
  // Configure for dynamically allocated beams
  _nVars = nVars;
  _maxGates = maxGates/decimationFactor;
  _preAllocate = false;
  _distanceSpanKm = distanceSpanKm;
  _decimationFactor = decimationFactor;
  _left = left;
  _right = right;
  _bottom = bottom;
  _top = top;
  _centerX = _left + (_right-_left)/2.0;
  _centerY = _bottom + (_top-_bottom)/2.0;
  _zoom = 1.0;

  _configured = true;
    
  makeCurrent();
  paintGL();

}
////////////////////////////////////////////////////////////////

void PPI::configure(int nVars,
                    int maxGates,
                    int nBeams,
                    double distanceSpanKm,
                    int decimationFactor,
                    double left,
                    double right,
                    double bottom,
                    double top)
{
  // Configure for preallocated beamd
  _nVars = nVars;
  _maxGates = maxGates/decimationFactor;
  _preAllocate = true;
  _distanceSpanKm = distanceSpanKm;
  _decimationFactor = decimationFactor;
  _left = left;
  _right = right;
  _bottom = bottom;
  _top = top;
  _centerX = _left + (_right-_left)/2.0;
  _centerY = _bottom + (_top-_bottom)/2.0;
  _zoom = 1.0;

  _configured = true;

  for (unsigned int i = 0; i < _beams.size(); i++)
    delete _beams[i];
  _beams.clear();

  // This constructor is called when we are preallocating beams.
  double angleInc = 360.0/nBeams;
  for (int i = 0; i < nBeams; i++) {
    _beams.push_back(new beam(i*angleInc, (i+1)*angleInc, _maxGates, _nVars));
  }
    
  makeCurrent();
  paintGL();
}
////////////////////////////////////////////////////////////////

PPI::~PPI()
{

  // delete all of the dynamically created beams
  for (unsigned int i = 0; i < _beams.size(); i++) {
    delete _beams[i];
  }
}

////////////////////////////////////////////////////////////////

void PPI::initializeGL()
{
  glClearColor(_backgroundColor.red()/255.0,
	       _backgroundColor.green()/255.0,
	       _backgroundColor.blue()/255.0,
	       0.0f);

  glDrawBuffer(GL_FRONT);
  glPolygonMode(GL_FRONT, GL_FILL);
  glPolygonMode(GL_BACK, GL_FILL);

  glShadeModel(GL_FLAT);

  glLineWidth(1.0);

  glDisable(GL_LINE_SMOOTH);
  glDisable(GL_POINT_SMOOTH);
  glDisable(GL_POLYGON_SMOOTH);
  glDisable(GL_DITHER);
  glDisable(GL_BLEND);
  glDisable(GL_ALPHA_TEST);
  glDisable(GL_INDEX_ARRAY);
  glDisable(GL_EDGE_FLAG_ARRAY);
  glDisable(GL_TEXTURE_COORD_ARRAY);
  glDisable(GL_NORMAL_ARRAY);
  glDisableClientState(GL_INDEX_ARRAY);
  glDisableClientState(GL_EDGE_FLAG_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);

  glEnable(GL_COLOR_ARRAY);
  glEnable(GL_VERTEX_ARRAY);
  glEnable(GL_STENCIL_TEST);

  glEnableClientState(GL_COLOR_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);

  // set the stencil buffer clear value.
  glClearStencil((GLint)0);

  // get a display list id for the rings
  _ringsListId = glGenLists(1);
  // get a display list id for the grid
  _gridListId = glGenLists(1);
  // get a display list id for the az lines
  _azLinesListId = glGenLists(1);
}

////////////////////////////////////////////////////////////////

void PPI::resizeGL(int w,
                   int h)
{
  // setup viewport, projection etc.:
  glViewport( 0, 0, (GLint)w, (GLint)h);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(_left, _right, _bottom, _top);
  glMatrixMode(GL_MODELVIEW);

  _centerX = _left + (_right-_left)/2.0;
  _centerY = _bottom + (_top-_bottom)/2.0;
  _zoom = 1.0;

  dump();
}

////////////////////////////////////////////////////////////////

void PPI::paintGL()
{
  if (_resizing) {
    // clear the image
    glClear(GL_COLOR_BUFFER_BIT);
    return;
  }

  // draw into the back buffer
  glDrawBuffer(GL_BACK);

  // clear the display
  glClear(GL_COLOR_BUFFER_BIT);

  // redraw the beams
  for (unsigned int i = 0; i < _beams.size(); i++) {
    glCallList(_beams[i]->_glListId[_selectedVar]);
  }

  // draw rings/grid
  if (_ringsEnabled || _gridsEnabled || _azLinesEnabled) {
    //createStencil();
    makeRingsAndGrids();
    if (_ringsEnabled)
      glCallList(_ringsListId);
    if (_gridsEnabled)
      glCallList(_gridListId);
    if (_azLinesEnabled)
      glCallList(_azLinesListId);
  }

  // display the back buffer
  swapBuffers();

  // and resume drawing to the front buffer.
  glDrawBuffer(GL_FRONT);
}

////////////////////////////////////////////////////////////////
void PPI::rings(bool enabled)
{
  _ringsEnabled = enabled;

  //redraw
  makeCurrent();
  paintGL();
}

////////////////////////////////////////////////////////////////
void PPI::grids(bool enabled)
{
  _gridsEnabled = enabled;

  //redraw
  makeCurrent();
  paintGL();
}

////////////////////////////////////////////////////////////////
void PPI::azLines(bool enabled)
{
  _azLinesEnabled = enabled;

  //redraw
  makeCurrent();
  paintGL();
}

////////////////////////////////////////////////////////////////
void PPI::dump()
{

  // remove the return statement when you need to debug PPI
  return;

  std::cout << "_centerX:" << _centerX << "  _centerY:" << _centerY
            << "   zoom:" << _zoom << "\n";
}

////////////////////////////////////////////////////////////////
void PPI::setZoom(double factor)
{

  makeCurrent();

  // if the zoom request is to go smaller than 1:1, 
  // restore to centered normal display
  if (factor <= 1.0) {
    _centerX = _left + (_right-_left)/2.0;
    _centerY = _bottom + (_top-_bottom)/2.0;
    _zoom = 1.0;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(_left, _right, _bottom, _top);
    glMatrixMode(GL_MODELVIEW);

  } else {
    // determine the new size of the projection.
    // It will retain its current center position.
    _zoom = factor;
    double w = (_right-_left);
    double h = (_top-_bottom);
    double l = _centerX - w/_zoom/2.0;
    double r = _centerX + w/_zoom/2.0;
    double b = _centerY - h/_zoom/2.0;
    double t = _centerY + h/_zoom/2.0;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(l, r, b, t);
    glMatrixMode(GL_MODELVIEW);

  }

  // redraw
  paintGL();

  dump();

  return;
}

////////////////////////////////////////////////////////////////

double PPI::getZoom()
{
  return _zoom;
}

////////////////////////////////////////////////////////////////

void PPI::refresh()
{
  if (_resizing)
    return;
  //redraw
  makeCurrent();
  glClear(GL_COLOR_BUFFER_BIT);
  paintGL();
}
////////////////////////////////////////////////////////////////

void PPI::pan(double deltax,
              double deltay)
{
  makeCurrent();

  // move the center of the projection.
  _centerX = _centerX + deltax*(_right-_left)/_zoom;
  _centerY = _centerY + deltay*(_top-_bottom)/_zoom;

  // determine new locations for the projection boundaries
  double w = (_right-_left);
  double h = (_top-_bottom);
  double l = _centerX - w/_zoom/2.0;
  double r = _centerX + w/_zoom/2.0;
  double b = _centerY - h/_zoom/2.0;
  double t = _centerY + h/_zoom/2.0;
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(l, r, b, t);
  glMatrixMode(GL_MODELVIEW);

  // redraw
  paintGL();

  dump();
}

////////////////////////////////////////////////////////////////
void PPI::resetView()
{
  makeCurrent();

  resizeGL(width(), height());

  // redraw
  paintGL();

  return;
}

////////////////////////////////////////////////////////////////
void PPI::resizeEvent(QResizeEvent * e)
{

  if (_resizing) {
    makeCurrent();
    glClear(GL_COLOR_BUFFER_BIT);
    return;
  }

  _resizing = true;

  _resizeTimer.start(500);
}

////////////////////////////////////////////////////////////////
void PPI::mousePressEvent(QMouseEvent * e)
{
  if (_cursorZoom) {
    _rubberBand->setGeometry(QRect(e->pos(), QSize()));
    _rubberBand->show();
  }

  _oldMouseX = e->x();
  _oldMouseY = e->y();

}

////////////////////////////////////////////////////////////////
void PPI::mouseMoveEvent(QMouseEvent * e)
{
  
  if (_cursorZoom) {
    // zooming with the mouse
    int x = e->x();
    int y = e->y();
    int deltaX = x - _oldMouseX;
    int deltaY = y - _oldMouseY;
    // get the current window aspec ratio
    double wAspect = ((double)width())/height();
    // make the rubberband aspect ratio match that
    // of the window
    double dx;
    double dy;
    dx = fabs(deltaY*wAspect);
    dy = fabs(dx/wAspect);
    // preserve the signs
    dx *= fabs(deltaX)/deltaX;
    dy *= fabs(deltaY)/deltaY;
    QRect newRect = QRect(_oldMouseX, _oldMouseY, (int)dx, (int)dy);

    newRect = newRect.normalized();
    _rubberBand->setGeometry(newRect);

  } else {
    // panning with the mouse
    mousePan(e);
  }
}

////////////////////////////////////////////////////////////////
void PPI::mouseReleaseEvent(QMouseEvent * e)
{

  if (_cursorZoom) {

    // we are zooming with the mouse
    makeCurrent();

    QRect g = _rubberBand->geometry();

    // find the center of the rubber band box in QRect coordinates,
    // but with y running from bottom to top
    double avgX = g.x() + g.width()/2.0;
    double avgY = g.y() + g.height()/2.0;

    // normalize to -0.5:0.5, sine we are panning to the 
    // center of the display.
    double newX = avgX/width() - 0.5;
    double newY = 0.5 -avgY/height();

    // calculate new zoom. Base it on the largest edge of 
    // the rubberband box
    double newZoom = 1.0;
    int deltaX = g.width();
    int deltaY = g.height();
    if (deltaX > 0 || deltaY > 0) {
      double zoomX = deltaX / (double)width();
      double zoomY = deltaY / (double)height();
      if (zoomX >= zoomY) {
	newZoom = 1.0/zoomX;
      } else {
	newZoom = 1.0/zoomY;
      }
    }

    // only react for movement of more than 20 pixels

    if (fabs(deltaX) > 20 && fabs(deltaY) > 20) {
      // translate by this amount
      pan(newX, newY);
      // scale by this amount
      setZoom(newZoom*_zoom);
    }

    // hide the rubber band
    _rubberBand->hide();

  } else {

    // we are panning with the mouse
    mousePan(e);
  }

  // emit a signal to indicate that the click location has changed

  double fx = (0.5 - (e->posF().x() / width())) * -1.0;
  double fy = 0.5 - (e->posF().y() / height());
  double xx = (fx / (_zoom / 2.0)) + _centerX;
  double yy = (fy / (_zoom / 2.0)) + _centerY;
  double xkm = xx * _distanceSpanKm / 2.0;
  double ykm = yy * _distanceSpanKm / 2.0;

  emit locationClicked(xkm, ykm);

  _clickX = xx;
  _clickY = yy;
  refresh();

}

////////////////////////////////////////////////////////////////
void PPI::mousePan(QMouseEvent * e)
{
  int x = e->x();
  int y = e->y();

  double deltaX = -(_oldMouseX - x);
  double deltaY = -(y - _oldMouseY);

  _oldMouseX = x;
  _oldMouseY = y;

  // Convert delta distances to our model coordinates
  // 0.0 to 1.0 of the window, across each axis
  deltaX = deltaX/width();
  deltaY = deltaY/height();

  pan(-deltaX, -deltaY);

}

////////////////////////////////////////////////////////////////
void PPI::resizeTimerTimeout()
{
  makeCurrent();
  resizeGL(this->width(), this->height());
  _resizing = false;
  refresh();
}

////////////////////////////////////////////////////////////////

int PPI::numBeams()
{
  return _beams.size();
}

////////////////////////////////////////////////////////////////////////

void PPI::selectVar(int index)
{
  _selectedVar = index;
  updateGL();
  return;
}

////////////////////////////////////////////////////////////////////////

void PPI::clearVar(int index)
{
  if (index >= _nVars)
    return;

  // calling makeDisplayList with data == 0 causes the display list to 
  // be drawn completely with the background color.
  float r = _backgroundColor.red()/255.0;
  float g = _backgroundColor.green()/255.0;
  float b = _backgroundColor.blue()/255.0;

  for (unsigned int i = 0; i < _beams.size(); i++) {
    int cIndex = 0;
    GLfloat* colors = _beams[i]->colors(index);
    for (int gate = 0; gate < _maxGates; gate++) {
      colors[cIndex++] = r;
      colors[cIndex++] = g;
      colors[cIndex++] = b;
      colors[cIndex++] = r;
      colors[cIndex++] = g;
      colors[cIndex++] = b;
    }
  }

  if (index == _selectedVar) {
    selectVar(index);
  }

}

////////////////////////////////////////////////////////////////

void PPI::addBeam(float startAngle,
                  float stopAngle,
                  int gates,
                  std::vector<std::vector<double> >& _beamData,
                  std::vector<ColorMap*>& maps)
{

  makeCurrent();

  // add a new beam to the display. 
  // The steps are:
  // 1. preallocate mode: find the beam to be drawn, or dynamic mode:
  //    create the beam(s) to be drawn.
  // 2. fill the colors for all variables in the beams to be drawn
  // 3. make the display list for the selected variables in the beams
  //    to be drawn.
  // 4. call the new display list(s)

  beam* b;
  std::vector<beam*> newBeams;

  // the start and stop angle MUST specify a counterclockwise fill for the sector. Thus 
  // if startAngle > stopAngle, we know that we have crossed the 0 boundary, and must
  // break it up into 2 beams.

  // create the new beam(s), to keep track of the display information
  // Beam start and stop angles are adjusted here so that they always 
  // increase counterclockwise. Like wise, if a beam crosses the 0 degree
  // boundary, it is split into two beams, each of them again obeying the
  // counterclockwise rule. Prescribing these rules makes the beam culling
  // logic a lot simpler.

  startAngle = startAngle - ((int)(startAngle/360.0))*360.0;
  stopAngle = stopAngle - ((int)(stopAngle/360.0))*360.0;

  if (startAngle <= stopAngle) {

    if (_preAllocate) {
      b = _beams[beamIndex(startAngle, stopAngle)];
      newBeams.push_back(b);
    } else {
      beam* b = new beam(startAngle, stopAngle, _maxGates, _nVars);
      _beams.push_back(b);
      newBeams.push_back(b);
    }
  } else {
    if (_preAllocate) {
      b = _beams[beamIndex(startAngle, 360.0)];
      newBeams.push_back(b);
    } else {
      b = new beam(startAngle, 360.0, _maxGates, _nVars);
      _beams.push_back(b);
      newBeams.push_back(b);
    }
    if (_preAllocate) {
      b = _beams[beamIndex(0.0, stopAngle)];
      newBeams.push_back(b);
    } else {
      b = new beam(0.0, stopAngle, _maxGates, _nVars);
      _beams.push_back(b);
      newBeams.push_back(b);
    }
    newBeams.push_back(b);
  }

  // newBeams has collected the beams to be rendered; now fill in 
  // their colors and draw them
  for (unsigned int i = 0; i < newBeams.size(); i++) {
    b = newBeams[i];
    fillColors(b, _beamData, gates, maps);

    for (int v = 0; v < _nVars; v++) {
      makeDisplayList(b, v);
    }
  }

  // draw it
  for (unsigned int i = 0; i < newBeams.size(); i++) {
    b = newBeams[i];
    if (!_resizing)
      glCallList(b->_glListId[_selectedVar]);
  }

  // draw the rings and grid if they are enabled. Don't worry,
  // it is only two display list calls. They are relative short
  // lists compared to the beam drawing, and done on the graphics card anyway.
  if (_ringsEnabled)
    glCallList(_ringsListId);

  if (_gridsEnabled)
    glCallList(_gridListId);

  if (_azLinesEnabled)
    glCallList(_azLinesListId);

  if (!_resizing)
    glFlush();

  if (!_preAllocate) {
    // in dynamic mode, cull hidden beams
    cullBeamList();
  }

}
////////////////////////////////////////////////////////////////

void PPI::fillColors(beam* beam,
                     std::vector<std::vector<double> >& _beamData,
                     int gates,
                     std::vector<ColorMap*>& maps)
{

  float red, green, blue;
  float bgRed = _backgroundColor.red()/255.0;
  float bgGreen = _backgroundColor.green()/255.0;
  float bgBlue = _backgroundColor.blue()/255.0;

  for (int v = 0; v < _nVars; v++) {

    ColorMap* map = maps[v];
    GLfloat* colors = beam->colors(v);
    int cIndex = 0;
    
    double* varData = &(_beamData[v][0]);
    for (int gate = 0; gate < gates; gate += _decimationFactor) {
      double data = varData[gate];
      if (data < -9990) {
        colors[cIndex++] = bgRed;
        colors[cIndex++] = bgGreen;
        colors[cIndex++] = bgBlue;
        colors[cIndex++] = bgRed;
        colors[cIndex++] = bgGreen;
        colors[cIndex++] = bgBlue;
      } else {
        map->dataColor(data, red, green, blue);
        colors[cIndex++] = red;
        colors[cIndex++] = green;
        colors[cIndex++] = blue;
        colors[cIndex++] = red;
        colors[cIndex++] = green;
        colors[cIndex++] = blue;
      }
    }

    for (int gate = gates; gate < _maxGates; gate++) {
      colors[cIndex++] = bgRed;
      colors[cIndex++] = bgGreen;
      colors[cIndex++] = bgBlue;
      colors[cIndex++] = bgRed;
      colors[cIndex++] = bgGreen;
      colors[cIndex++] = bgBlue;
    }

  }

}

////////////////////////////////////////////////////////////////

void PPI::makeDisplayList(beam* b,
                          int v)
{
  //	glGenBuffers();

  // create a display list to hold the gl commands
  glNewList(b->_glListId[v], GL_COMPILE);

  // set the vertex pointer
  glVertexPointer(2, GL_FLOAT, 0, b->vertices());

  // set the colors pointer
  glColorPointer(3, GL_FLOAT, 0, b->colors(v));

  // draw a triangle strip. 
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 2*_maxGates);

  // end the display list
  glEndList();

}

////////////////////////////////////////////////////////////////////////

void PPI::cullBeamList()
{
  // This routine examines the collection of beams, and removes those that are 
  // completely occluded by other beams. The algorithm gives precedence to the 
  // most recent beams; i.e. beams at the end of the _beams vector.
  //
  // Remember that there won't be any beams that cross angles through zero; 
  // otherwise the beam culling logic would be a real pain, and PPI has
  // already split incoming beams into two, if it received a beam of this type.
  //
  // The logic is as follows. First of all, just consider the start and stop angle 
  // of a beam to be a linear region. We can diagram the angle interval of beam(AB) as:
  //         a---------b
  // 
  // The culling logic will compare all other beams (XY) to AB, looking for an overlap.
  // An example overlap might be:
  //         a---------b
  //    x---------y
  // 
  // If an overlap on beam XY is detected, the occluded region is recorded as the interval (CD):        
  //         a---------b
  //    x---------y
  //         c----d
  // 
  // The culling algorithm starts with the last beam in the list, and compares it with all
  // preceeding beams, setting their overlap regions appropriately. Then the next to the last
  // beam is compared with all preceeding beams. Previously found occluded regions will be 
  // expanded as they are detected.
  // 
  // Once the occluded region spans the entire beam, then the beam is known 
  // to be hidden, and it doesn't need to be tested any more, nor is it it used as a 
  // test on other beams.
  //
  // After the list has been completly processed in this manner, the completely occluded 
  // beams are removed.
  // .
  // Note now that if the list is rendered from beginning to end, the more recent beams will
  // overwrite the portions of previous beams that they share.
  //

  // do nothing if we don't have at least two beams 
  if (_beams.size() < 2)
    return;

  int i;
  int j;
  beam* beamAB;
  beam* beamXY;

  // initialize the house keeping
  for (unsigned int k = 0; k < _beams.size(); k++) {
    beamAB = _beams[k];
    beamAB->_hidden = false;
    beamAB->_c = -1.0;
    beamAB->_d = -1.0;
  }

  // Here is an outerloop and a nested innerloop. Work backwords
  // from the end of the display list, comparing one beam to all
  // of those which preceed it.
  for (i = _beams.size()-1; i >= 1; i--) {
    // select the next preceeding beam on the beam list
    beamAB = _beams[i];

    // if this beam is hidden, we don't need to compare it to preceeding ones, 
    // since they will be hidden by the ones that hid this beam.
    if (!beamAB->_hidden) {
      double a = beamAB->_startAngle;
      double b = beamAB->_stopAngle;

      for (j = i-1; j >= 0; j--) {
	// and compare it to all of its predecessors
	beamXY = _beams[j];

	// if this beam has alread been marked hidden, we don't need to 
	// look at it.
	if (!beamXY->_hidden) {
	  double x = beamXY->_startAngle;
	  double y = beamXY->_stopAngle;

	  if (b <= x || a >= y) {
	    //  handles these cases:
	    //  a-----b           a-----b
	    //        x-----------y
	    //  
	    // they don't overlap at all
	  } else {

	    if (a <= x && b >= y) {
	      //        a-----------b
	      //        x-----------y
	      // completely covered
	      beamXY->_hidden = true;
	    } else {

	      if (a <= x && b <= y) {
		//   a-----------b
		//        x-----------y
		beamXY->_c = x;
		if (beamXY->_d < b)
		  beamXY->_d = b;
		if ((beamXY->_c == x) && (beamXY->_d == y))
		  beamXY->_hidden = true;
	      } else {

		if (a >= x && b >= y) {
		  //       a-----------b
		  //   x-----------y
		  beamXY->_d = y;
		  if (a < beamXY->_c)
		    beamXY->_c = a;
		  if ((beamXY->_c == x) && (beamXY->_d == y))
		    beamXY->_hidden = true;
		}

		//   // all that is left is this pathological case:
		//      //     a-------b
		//      //   x-----------y
		//      // we need to extend c and d, if the are inside of a and b.
		if (beamXY->_c > a)
		  beamXY->_c = a;
		if (beamXY->_d < b)
		  beamXY->_d = b;

	      }//               if (a <= x && b <= y) {
	    } //            if (a <= x && b >= y) {
	  } //           if (b <= x || a >= y) { 
	} //          if (!beamXY->_hidden) {
      }//        for (j = b; j >= 0; j--) {
    }//  if (beamAB->_hidden) {
  } //  for (i = _beams.size()-1; i >= 1; i--) {

    // now actually cull the list
  int nCulled = 0;
  for (i = _beams.size()-1; i >= 0; i--) {
    if (_beams[i]->_hidden) {
      delete _beams[i];
      _beams.erase(_beams.begin()+i);
      nCulled++;
    }
  }
}

////////////////////////////////////////////////////////////////////////

int PPI::beamIndex(double startAngle,
                   double stopAngle)
{
  int i = (int)(_beams.size()*(startAngle + (stopAngle-startAngle)/2)/360.0);
  if (i<0)
    i = 0;
  if (i>(int)_beams.size()-1)
    i = _beams.size()-1;

  return i;
}

////////////////////////////////////////////////////////////////////////
void PPI::backgroundColor(QColor color)
{
  _backgroundColor = color;
  glClearColor(_backgroundColor.red()/255.0,
	       _backgroundColor.green()/255.0,
	       _backgroundColor.blue()/255.0,
	       0.0f);

  makeCurrent();
  updateGL();
}
////////////////////////////////////////////////////////////////////////
void PPI::gridRingsColor(QColor color)
{
  _gridRingsColor = color;

  makeCurrent();
  updateGL();
}

////////////////////////////////////////////////////////////////////////
void PPI::makeRingsAndGrids()
{

  // don't try to draw rings if we haven't been configured yet
  if (!_configured)
    return;

  // or if the rings or grids aren't enabled
  if (!_ringsEnabled && !_gridsEnabled)
    return;

  double ringDelta = ringSpacing();
  double ringLabelIncrement = ringDelta;
  double ringLabelOffset = 0.02/_zoom; // used to move some of the labelling so that it does not overlap the rings.
  double lineWidth = 0.002/ _zoom;

  // Do range rings?
  if (ringDelta > 0 && _ringsEnabled) {

    // create a display list to hold the gl commands
    glNewList(_ringsListId, GL_COMPILE);

    // set the color
    glColor3f(_gridRingsColor.red()/255.0,
	      _gridRingsColor.green()/255.0,
	      _gridRingsColor.blue()/255.0);

    // Get a new quadric object.
    GLUquadricObj *quadObject = gluNewQuadric();

    GLdouble radius = ringDelta;

    // Draw our range rings.
    while (radius <= 1.01) {
      gluDisk(quadObject, radius-lineWidth/2, radius+lineWidth/2, 100, 1);
      radius += ringDelta;
    }

    // label the rings
    if (ringLabelIncrement > 0.0) {
      std::vector<std::string> ringLabels;
      // creat the labels. Note that we are not creating a lable at zero
      for (int i = 0; i < 1/ringLabelIncrement - 1; i++) {
	double value = (i+1)*ringLabelIncrement*_distanceSpanKm / 2.0;
	ringLabels.push_back(_scaledLabel.scale(value));
      }

      for (unsigned int j = 0; j < ringLabels.size(); j++) {
	double d = 0.707*(j+1)*ringDelta;
	const char* cStart = ringLabels[j].c_str();
	const char* c;

	// upper right qudrant lables
	glRasterPos2d(d, d);
	c = cStart;
	while (*c)
	  glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,*c++);
        
	// lower left quadrant labels
	glRasterPos2d(-d, -d);
	c = cStart;
	while (*c)
	  glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,*c++);

	// lower right quadrant labels
	glRasterPos2d(d+ringLabelOffset, -d-ringLabelOffset);
	c = cStart;
	while (*c)
	  glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,*c++);

	// upper left qudrant labels
	glRasterPos2d(-d+ringLabelOffset, d-ringLabelOffset);
	c = cStart;
	while (*c)
	  glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,*c++);

      }
    }
    // get rid of quad object
    gluDeleteQuadric(quadObject);

    glEndList();

  }

  // do the grid
  if (ringDelta > 0 && _gridsEnabled) {

    // create a display list to hold the gl commands
    glNewList(_gridListId, GL_COMPILE);

    // set the color
    glColor3f(_gridRingsColor.red()/255.0,
	      _gridRingsColor.green()/255.0,
	      _gridRingsColor.blue()/255.0);

    glLineWidth(1);

    glBegin(GL_LINES);
    // First the vertical lines.
    // set the first x value
    GLdouble x = (-(int)(1.0/ringDelta)) * ringDelta;
    while (x <= 1.0) {
      glVertex2d(x, -1.0);
      glVertex2d(x, 1.0);
      x += ringDelta;
    }
    // Now horizontial lines
    // set the first y value to an even increment of the grid spacing.
    GLdouble y = (-(int)(1.0/ringDelta)) * ringDelta;
    ;
    while (y <= 1.0) {
      glVertex2d(-1.0, y);
      glVertex2d( 1.0, y);
      y += ringDelta;
    }
    glEnd();

    glEndList();
  }

  // do the azimuth lines

  if (_azLinesEnabled) {

    // create a display list to hold the gl commands
    glNewList(_azLinesListId, GL_COMPILE);

    // set the color
    glColor3f(_gridRingsColor.red()/255.0,
	      _gridRingsColor.green()/255.0,
	      _gridRingsColor.blue()/255.0);
    
    glLineWidth(1);
    glBegin(GL_LINES);

    for (double az = 0; az < 180; az += 30.0) {

      double xx = cos(az * degToRad);
      double yy = sin(az * degToRad);
      glVertex2d(xx, yy);
      xx *= -1.0;
      yy *= -1.0;
      glVertex2d(xx, yy);

    }
    
    glEnd();

    glEndList();

  } // azLines

  // click point cross hairs

  if (_clickX > -90 && _clickY > -90) {

    // create a display list to hold the gl commands
    glNewList(_clickListId, GL_COMPILE);

    // set the color
    glColor3f(_gridRingsColor.red()/255.0,
	      _gridRingsColor.green()/255.0,
	      _gridRingsColor.blue()/255.0);
    
    glLineWidth(1);
    glBegin(GL_LINES);
    
    double crossSize = 0.025 / _zoom;
    glVertex2d(_clickX - crossSize, _clickY);
    glVertex2d(_clickX + crossSize, _clickY);
    glVertex2d(_clickX, _clickY - crossSize);
    glVertex2d(_clickX, _clickY + crossSize);

    glEnd();
    
    glEndList();

  } // click point

}

////////////////////////////////////////////////////////////////////////
double PPI::ringSpacing()
{

  // R is the visible distance from center to edge
  double R = (_distanceSpanKm / _zoom);
  double e = (int)floor(log10(R));
  double Rn = R / pow(10.0, e);

  double delta = 2.0;
  if (Rn <= 5.0) {
    delta = 1.0;
  }
  if (Rn <= 3.0) {
    delta = 0.5;
  }
  if (Rn <= 1.0) {
    delta = 0.2;
  }
  if (Rn <= 0.5) {
    delta = 0.1;
  }

  delta = delta * pow(10.0, e);

  delta = delta/_distanceSpanKm;

  return delta;

}
////////////////////////////////////////////////////////////////////////
QImage* PPI::getImage()
{
  makeCurrent();
  updateGL();
  glReadBuffer(GL_FRONT);
  QImage* pImage = new QImage(grabFrameBuffer(true));
  return pImage;
}

////////////////////////////////////////////////////////////////////////
QPixmap* PPI::getPixmap()
{
  QPixmap* pImage = new QPixmap(renderPixmap());
  return pImage;
}

////////////////////////////////////////////////////////////////////////
void PPI::cursorZoom()
{
  _cursorZoom = true;
}

////////////////////////////////////////////////////////////////////////
void PPI::cursorPan()
{
  _cursorZoom = false;
}
