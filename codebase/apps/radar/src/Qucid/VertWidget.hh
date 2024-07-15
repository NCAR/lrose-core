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
#ifndef VertWidget_HH
#define VertWidget_HH

#include "CartWidget.hh"
// #include "RayLoc.hh"
#include <radar/BeamHeight.hh>
#include <Radx/RadxTime.hh>
// #include <deque>
class VertWindow;

// Widget representing an RHI scan.  Beams are added to the scan as they
// are received.  The widget can be set up to display the RHI in a 90 degree
// view or in a 180 degree view.

class DLL_EXPORT VertWidget : public CartWidget
{

  // must include this if you use Qt signals/slots
  Q_OBJECT

 public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   *
   * @param[in] parent   Parent widget.
   * @param[in] params   TDRP parameters.
   */

  VertWidget(QWidget* parent, 
             const CartManager &manager,
             const VertWindow &vertWindow);

  /**
   * @brief Destructor.
   */

  virtual ~VertWidget();

  /**
   * @brief Configure the CartWidget for world coords
   */

  virtual void configureWorldCoords(int zoomLevel = 0);

  /**
   * @brief Add a new beam to the display. Data for all fields and all
   *        gates are provided, as well as color maps for all fields.
   *        addBeam() will map the field values to  the correct color, and
   *        render the beam for each field in the appropriate pixamp. The
   *        existing wedge for this beam will be discarded.
   *
   * @param[in] gates          The number of gates (must match beam_data vector
   *                             sizes).
   * @param[in] beam_data      Vectors of data, one for each field.
   * @param[in] maps           Colormaps, one for each field.
   */

  // void addBeam(const RadxRay *ray,
  //              const std::vector< std::vector< double > > &beam_data,
  //              const std::vector< DisplayField* > &fields);

  // are we in archive mode? and if so are we at the start of a sweep?

  void setArchiveMode(bool state) { _isArchiveMode = state; }
  // void setStartOfSweep(bool state) { _isStartOfSweep = state; }

  /**
   * @brief Select the field to display.
   *
   * @param[in] index   Index of the field to display, zero based.
   */

  void selectVar(const size_t index);

  // get plot times

  const RadxTime &getPlotStartTime() { return _plotStartTime; }
  const RadxTime &getPlotEndTime() { return _plotEndTime; }

signals:

  ////////////////
  // Qt signals //
  ////////////////

  /**
   * @brief Signal emitted when we have processed several beams.  This signal
   *        tells the RHI window that it can do a resize to fix the widget
   *        sizing problem that we have on startup.
   */

  void severalBeamsProcessed();
  
  //////////////
  // Qt slots //
  //////////////

 public slots:

  /**
   * @brief Reset the view to unzoomed.
   */

  void unzoomView();

  /**
   * @brief Clear the data in the view.
   */

  void clear();

  /*************************************************************************
   * refresh()
   */
  
  void refresh();

  /**
   * @brief Resize the window.
   *
   */

  void resize(int width, int height);

  // paint event

  void paintEvent(QPaintEvent *event);

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  const VertWindow &_vertWindow;

  // are we in archive mode? and if so are we at the start of a sweep?

  bool _isArchiveMode;
  bool _isStartOfSweep;

  // angles and times in archive mode

  RadxTime _plotStartTime;
  RadxTime _plotEndTime;
  double _meanAz;
  double _sumAz;
  double _nRays;

  /**
   * @brief The maximum range of the beams, in km.  It affects the
   *        labelling of the range rings
   */

  double _maxHeightKm;
  double _xGridSpacing;
  double _yGridSpacing;

  /**
   * @brief Pointers to all of the active beams are saved here.
   */

  // std::deque<RhiBeam*> _rhiBeams;

  // ray locations

  // vector<RayLoc> _rayLoc;
  BeamHeight _beamHt;

  // computing angle limits of rays

  RadxTime _prevTime;
  double _prevAz;
  double _prevElev;
  double _startElev;
  double _endElev;

  // override mouse release event

  virtual void mouseReleaseEvent(QMouseEvent* event);

  /**
   * @brief The number of RHI beams processed so far.  I have to keep track of
   *        this so that I can automatically resize the window after processing
   *        a few beams to get rid of a problem with widget size on startup.
   */

  int _beamsProcessed;
  
  // get ray closest to click point

  virtual const RadxRay *_getClosestRay(double x_km, double y_km);

  /**
   * @brief Render the rings and grid. The current value of _ringsGridColor
   *        will be used for the color.
   *
   * @param[in] painter    Painter to use for rendering.
   */

  virtual void _drawOverlays(QPainter &painter);

  /**
   * @brief Determine a ring spacing which will give even distances, and
   *        fit a reasonable number of rings in the display.
   *
   * @return Returns the ring spacing in kilometers.
   */

  virtual void _setGridSpacing();
  double _getSpacing(double range);

  // Compute the limits of the ray angles
  
  void _computeAngleLimits(const RadxRay *ray);
  
  // store ray location
  
  void _storeRayLoc(const RadxRay *ray);

  // clear overlap with existing rays

  void _clearRayOverlap(const int startIndex,
                        const int endIndex);

  // overide refresh images

  virtual void _refreshImages();

};

#endif
