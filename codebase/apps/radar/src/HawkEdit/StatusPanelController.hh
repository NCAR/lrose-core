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
/////////////////////////////////////////////////////////////
// StatusPanelController.hh
//
// StatusPanel object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2014
//
// Brenda Javornik, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
// June 2023
//
///////////////////////////////////////////////////////////////
//
// StatusPanel manages the display of metadata for a selected
// (file, sweep, ray, range) = (time, elevation, azimuth, range)
//
// Let the view request information for update
// The controller/model will supply the information
// 
// We ask the view to read the data in the top left cell again 
// by emitting the dataChanged() signal. Note that we did not 
// explicitly connect the dataChanged() signal to the view. 
// This happened automatically when we called setModel().
//
///////////////////////////////////////////////////////////////

#ifndef StatusPanelController_HH
#define StatusPanelController_HH

#include <string>
#include <vector>

#include "Args.hh"
#include "Params.hh"

#include "StatusPanelView.hh"

#include <euclid/SunPosn.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxGeoref.hh>

class QDateTime;

class RadxPlatform;


class StatusPanelController : public QObject {

  Q_OBJECT

public:

  // constructor
  
  StatusPanelController(StatusPanelView *view);
  
  // destructor
  
  ~StatusPanelController();

  // override event handling

  //void timerEvent (QTimerEvent * event);
  void resizeEvent (QResizeEvent * event);

  /* location

  double getRadarLat() const { return _radarLat; }
  double getRadarLon() const { return _radarLon; }
  double getRadarAltKm() const { return _radarAltKm; }
  const RadxPlatform &getPlatform() const { return _platform; }

  bool evaluateCursor(bool isShiftKeyDown);

  bool evaluateRange(double xRange);
*/

  void closeEvent(QEvent *event);

  void setDisplay(
    bool show_fixed_angle,
    bool show_volume_number,
    bool show_sweep_number,
    bool show_n_samples,
    bool show_n_gates,
    bool show_gate_length,
    bool show_pulse_width,
    bool show_prf_mode,
    bool show_prf,
    bool show_nyquist,
    bool show_max_range,
    bool show_unambiguous_range,
    bool show_measured_power_h,
    bool show_measured_power_v,
    bool show_scan_name,
    bool show_scan_mode,
    bool show_polarization_mode,
    bool show_latitude,
    bool show_longitude,
    bool show_altitude,
    bool show_altitude_rate,
    bool show_speed,
    bool show_heading,
    bool show_track,
    bool show_sun_elevation,
    bool show_sun_azimuth);  

  void createStatusPanel();
  void updateStatusPanel(const RadxRay *ray);
  void setFontSize(int fontSize);

  void setRadarName(string radarName, string siteName);  
  void createDateTime();

  string interpretSweepMode(Radx::SweepMode_t sweepMode);

public slots:

  void newDataFile();
 
  //void setDataMissing(string fieldName, float missingValue);

  //void selectedFieldChanged(QString newFieldName);
  //void selectedFieldChanged(string fieldName);
  //void _updateField(size_t fieldId);

  //void selectedSweepChanged(int sweepNumber);


signals:

  ////////////////
  // Qt signals //
  ////////////////

  
  //void frameResized(const int width, const int height);
  //void setParamsFile();

  //void addField(QString fieldName);

  //void newSweepData(int sweepNumber);

private:

  // from DisplayManager ...
  //ParamFile *_params;

  StatusPanelView *_view;
  
  // instrument platform details 

  RadxPlatform _platform;

  bool _altitudeInFeet;

  vector<QLabel *> _valsRight;
  
  // click location report dialog
  QDialog *_clickReportDialog;
  QGridLayout *_clickReportDialogLayout;
  QLabel *_dateClicked;
  QLabel *_timeClicked;
  QLabel *_elevClicked;
  QLabel *_azClicked;
  QLabel *_gateNumClicked;
  QLabel *_rangeClicked;
  QLabel *_altitudeClicked;

  QCheckBox *_applyCfacToggle;
  
  // sun position calculator
  double _radarLat, _radarLon, _radarAltKm;
  SunPosn _sunPosn;

  void _chooseGeoReferenceOrPlatform(const RadxRay *ray, int key);  
  void _getInforFromPlatform(const RadxRay *ray, int key);
  void _getInfoFromGeoreference(const RadxGeoref *georef,
   RadxTime rayTime, int key);

  double _calculateSpeed(double ewVel, double nsVel);

  /* panels
  
  void hideCfacs();

  // setting text

  */

private slots:

  //////////////
  // Qt slots //
  //////////////

  //void _refresh();

private:
    bool _show_radar_name;
    bool _show_site_name;
    bool _show_fixed_angle;
    bool _show_volume_number;
    bool _show_sweep_number;
    bool _show_n_samples;
    bool _show_n_gates;
    bool _show_gate_length;
    bool _show_pulse_width;
    bool _show_prf_mode;
    bool _show_prf;
    bool _show_nyquist;
    bool _show_max_range;
    bool _show_unambiguous_range;
    bool _show_measured_power_h;
    bool _show_measured_power_v;
    bool _show_scan_name;
    bool _show_scan_mode;
    bool _show_polarization_mode;
    bool _show_latitude;
    bool _show_longitude;
    bool _show_altitude;
    bool _show_altitude_rate;
    bool _show_speed;
    bool _show_heading;
    bool _show_track;
    bool _show_sun_elevation;
    bool _show_sun_azimuth;

};

#endif

