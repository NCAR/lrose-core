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
#ifndef PpiWidget_HH
#define PpiWidget_HH

#include "Radx/RadxVol.hh"
#include "PolarWidget.hh"

// Widget representing a PPI scan.  Beams are added to the scan as they
// are received.

class DLL_EXPORT PpiWidget : public PolarWidget
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

  PpiWidget(QWidget* parent,
            const PolarManager &manager,
            const Params &params,
            const RadxPlatform &platform,
	    DisplayFieldController *displayFieldController,
	    //            const vector<DisplayField *> &fields,
            bool haveFilteredFields);

  /**
   * @brief Destructor.
   */

  virtual ~PpiWidget();

  /**
   * @brief Configure for range.
   */

  virtual void configureRange(double max_range);

  /**
   * @brief Add a new beam to the display. Data for all fields and all
   *        gates are provided, as well as color maps for all fields.
   *        addBeam() will map the field values to  the correct color, and
   *        render the beam for each field in the appropriate pixamp. The
   *        existing wedge for this beam will be discarded.
   *
   * @param[in] start_angle    The starting angle for the beam.
   * @param[in] stop_angle     The ending angle for the beam.
   * @param[in] gates          The number of gates (must match beam_data vector
   *                             sizes).
   * @param[in] beam_data      Vectors of data, one for each field.
   */

  void addBeam(const RadxRay *ray,
               const float start_angle, const float stop_angle,
               const std::vector< std::vector< double > > &beam_data,
	       size_t nFields); 

  void addCullTrackBeam(const RadxRay *ray,
				   const float start_angle,
				   const float stop_angle,
				   const std::vector< std::vector< double > > &beam_data,
			size_t nFields);

  void updateBeam(const RadxRay *ray,
               const float start_angle, const float stop_angle,
               const std::vector< std::vector< double > > &beam_data,
	       size_t nFields); 

  void updateBeamII(const RadxRay *ray,
               const float start_angle, const float stop_angle,
               const std::vector< std::vector< double > > &beam_data,
		    size_t nFields,
		    const vector<string> &fieldNames);

  void movingDownTheLine(PpiBeam *beam, vector<string> fieldNames,
			 const std::vector< std::vector< double > > &beam_data,
			 size_t nFields); 


  void updateBeamColors(//const RadxRay *ray,
                        //const float start_angle,
                        //const float stop_angle,
                        //const std::vector< double > &beam_data,
			size_t nFields,
                        const string fieldName,
			size_t nGates);

  void updateColorsOnFields(PpiBeam *beam, string fieldName, size_t nFields,
			    const Radx::fl32 *beam_data,
			    size_t nGates,
			    const ColorMap *map,
			    size_t displayFieldIdx);


  void drawColorScaleLegend();


  // are we in archive mode? and if so are we at the start of a sweep?

  void setArchiveMode(bool state) { _isArchiveMode = state; }
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
  void ExamineEdit(double azimuth, double elevation, size_t fieldIndex); // const RadxRay *closestRay);
  void EditRunScript();

  //////////////
  // Qt slots //
  //////////////

 public slots:

  /**
   * @brief Clear the data in the view.
   */

  void clear();

  void contextMenuEditor();
  void contextMenuExamine();
  void contextMenuParameterColors();
  //  void changeToDisplayField(string fieldName); // , ColorMap newColorMap);

 protected:

  // pointers to active beams

  std::vector<PpiBeam*> _ppiBeams;
  //PpiBeamController *_ppiBeamController;

  // are we in archive mode? and if so are we at the start of a sweep?

  bool _isArchiveMode;
  bool _isStartOfSweep;

  // angles and times in archive mode

  RadxTime _plotStartTime;
  RadxTime _plotEndTime;
  double _meanElev;
  double _sumElev;
  double _nRays;

  // override mouse release event
  virtual void mouseReleaseEvent(QMouseEvent* event);

  // used to detect shift key pressed for boundary editor (switches cursor)
  virtual void timerEvent(QTimerEvent * event);


  // get ray closest to click point
  virtual const RadxRay *_getClosestRay(double x_km, double y_km);

  // get azimuth closest to click point
  virtual double _getClosestAz(double x_km, double y_km);

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
    
  /**
   * @brief Refresh the images.  Note that this is an expensive method and
   *        should only be called where needed.
   */

  virtual void _refreshImages();

  /**
   * @brief For dynamically allocated beams, cull the beam list, removing
   *        beams that are hidden by the given new beam.
   *
   * @params[in] beamAB     The new beam being added to the list.  Note that
   *                        this beam must not already be added to the list
   *                        when this method is called or it will be immediately
   *                        removed again.
   */

  void _cullBeams(const PpiBeam *beamAB);
  
  /**
   * @brief Find the index in the _ppiBeams array of the beam that corresponds
   *        to this angle. The beam angles must sweep in a counter clockwise,
   *         i.e. cartessian, direction.
   *
   * @param[in] start_angle    Beginning angle of the beam.
   * @param[in] stop_angle     Ending angle of the beam.
   *
   * @return Returns the index for the given beam.
   */

  inline int _beamIndex(const double start_angle, const double stop_angle);


  void _accumulateStats(const RadxRay *ray);

  RadxVol *_vol;
};


#endif
