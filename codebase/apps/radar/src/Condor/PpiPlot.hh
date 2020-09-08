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

#ifndef PpiPlot_HH
#define PpiPlot_HH

#include "Radx/RadxVol.hh"
#include "PolarPlot.hh"
class PpiWidget;

// Plot for a PPI scan.  Beams are added to the scan as they
// are received.

class DLL_EXPORT PpiPlot : public PolarPlot
{

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

  PpiPlot(PolarWidget *parent,
          const PolarManager &manager,
          const Params &params,
          int id,
          Params::plot_type_t plotType,
          string label,
          double minAz,
          double maxAz,
          double minEl,
          double maxEl,
          double minXKm,
          double maxXKm,
          double minYKm,
          double maxYKm,
          const RadxPlatform &platform,
          const vector<DisplayField *> &fields,
          bool haveFilteredFields);

  /**
   * @brief Destructor.
   */
  
  virtual ~PpiPlot();

  /**
   * @brief Add a new beam to the display. Data for all fields and all
   *        gates are provided, as well as color maps for all fields.
   *        addRay() will map the field values to  the correct color, and
   *        render the beam for each field in the appropriate pixamp. The
   *        existing wedge for this beam will be discarded.
   *
   * @param[in] start_angle    The starting angle for the beam.
   * @param[in] stop_angle     The ending angle for the beam.
   * @param[in] gates          The number of gates (must match beam_data vector
   *                             sizes).
   * @param[in] beam_data      Vectors of data, one for each field.
   */
  
  virtual void addRay(const RadxRay *ray,
                      const std::vector< std::vector< double > > &beam_data,
                      const std::vector< DisplayField* > &fields);
  
  // are we in archive mode? and if so are we at the start of a sweep?
  
  void setStartOfSweep(bool state) { _isStartOfSweep = state; }

  // get the number of beams stored in widget

  size_t getNumBeams() const;
  
  /**
   * @brief Select the field to display.
   *
   * @param[in] index   Index of the field to display, zero based.
   */

  void selectVar(const size_t index);

  /**
   * @brief Clear the specified field.
   *
   * @param[in] index    Index of the field to be cleared, zero based.
   *
   * @notes This method is not currently called anywhere.
   */
  
  void clearVar(const size_t index);
  
  // get plot times
  
  const RadxTime &getPlotStartTime() { return _plotStartTime; }
  const RadxTime &getPlotEndTime() { return _plotEndTime; }

  void ShowContextMenu(const QPoint &pos, RadxVol *vol);
  void ExamineEdit(const RadxRay *closestRay);

  /**
   * @brief Clear the data in the view.
   */
  
  virtual void clear();

  /**
   * @brief Refresh the images for each field.
   *        Note that this is an expensive method and
   *        should only be called where needed.
   */

  virtual void refreshFieldImages();

  // get ray closest to click point

  virtual const RadxRay *getClosestRay(int imageX, int imageY,
                                       double &xKm, double &yKm);
  
 protected:

  // are we in archive mode? and if so are we at the start of a sweep?

  bool _isArchiveMode;
  bool _isStartOfSweep;

  // angles and times in archive mode

  RadxTime _plotStartTime;
  RadxTime _plotEndTime;
  double _meanElev;
  double _sumElev;
  double _nRays;

  // initialize world coords

  void _initWorld();

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

  // draw text in screen coords

  void _drawScreenText(QPainter &painter, const string &text,
                       int text_x, int text_y,
                       int flags);
    
 private:
  
  // ray locations
  // we use a locator with data every 1/10th degree
  // around the 360 degree circle
  
  static const int RAY_LOC_RES = 10;
  static const int RAY_LOC_N = 3600;
  
  class RayLoc {
    
  public:

    // constructor
    
    RayLoc(int index);

    // set the ray and beam data

    void setData(double az, const RadxRay *ray, PpiBeam *beam);

    // clear the ray and beam data
    
    void clearData();

    // get methods
    
    int getIndex() const { return _index; }
    double getMidAz() const { return _midAz; }
    double getTrueAz() const { return _trueAz; }
    bool getActive() const { return _active; }
    const RadxRay *getRay() const { return _ray; }
    PpiBeam *getBeam() const { return _beam; }

  private:
    
    // members
    
    int _index;
    double _midAz;
    double _trueAz;
    bool _active;
    const RadxRay *_ray;
    PpiBeam *_beam;
    
  };
  
  vector<RayLoc *> _rayLoc;
  int _rayLocWidthHalf;

  int _getRayLocIndex(double az);
  void _storeRayLoc(double az,
                    const RadxRay *ray,
                    PpiBeam *beam);

};


#endif
