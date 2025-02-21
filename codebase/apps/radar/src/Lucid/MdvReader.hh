/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
///////////////////////////////////////////////////////////////
// MdvReader.hh
//
// Field data object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2024
//
///////////////////////////////////////////////////////////////
// METEROLOGICAL DATA record info - for each data field
///////////////////////////////////////////////////////////////

#ifndef MET_RECORD_HH
#define MET_RECORD_HH

#include <QObject>
#include <QMutex>

#include <toolsa/DateTime.hh>
#include <toolsa/mem.h>
#include <Mdv/DsMdvx.hh>

#include "cidd_macros.h"
#include "cidd_structs.h"
#include "cidd_colorscales.h"
#include "LatLonBox.hh"
#include "WayPts.hh"

class ColorMap;
class MdvReader;
class ReadVolH;

class MdvReader : public QObject {

  Q_OBJECT

public:

  // constructor
  
  MdvReader(QObject* parent = nullptr);

  // is the data valid?
  
  bool isValidH() const;
  bool isValidV() const;

  // is the data new?
  // if new is true, sets to false and returns true
  
  bool isNewH() const;
  bool isNewV() const;

  // is the previous read busy

  bool getReadBusyH() { return _readBusyH; }

  // time list
  
  // void setTimeListValid(bool state) { _timeListValid = state; }
  bool getTimeListValid() const { return _timeListValid; }
  
  // Get data for a horiz plane
  // This is implemented in a thread.
  // On error, isValidH is set to false.

  void requestHorizPlane(const DateTime &midTime,
                         double vLevel,
                         int page);

  int getHorizPlane();
  
  // Get data for a vert section

  int requestVertSection(const DateTime &midTime,
                         int page);
  
  // perform the vol read for Horiz

  void startReadVolH();

  // get labels for legends
  
  string vlevelLabel();
  string fieldLabel();
  string heightLabel();
  
  // public members
  
  int plane; /* plane of data rendered in horiz visible area */
  int currently_displayed; /* flag indicating if field in field list */
  int auto_render; /* flag indicating html/ automatic rendering */
  int render_method; /* Either RECTANGLES, FILLED_CONTOURS or LINE_CONTOURS */
  int composite_mode; /* Set to 1 to request a composite of this data field */
  int auto_scale; /* Set to 1 to map the data's dynamic range into the color scale */
  int use_landuse; /* activate the Land Use Field */
  int num_display_pts; /* approx. number of data points to render */
  time_t last_collected; /* Unix time that data was loaded */
 
  int h_data_valid; /* 1 = Valid data pointer, 0 = invalid */
  int v_data_valid; /* 1 = Valid data pointer, 0 = invalid */

  int vert_type; /* TYPE of vertical grid - See Mdv/Mdvx_enums.hh */

  // double alt_offset; /* In native units - km/deg/sigma/pressure/etc */

  double detail_thresh_min; /* Grids are visible when distance (km) across */
  double detail_thresh_max; /* across the screen is between min and max */
  
  vert_spacing_t vert[MAX_SECTS]; // Holds min,cent and max for each plane.

  double ht_pixel; // Slope of function between elevation height and screen Y
  double y_intercept; // Intercept of function between elevation and screen Y
  
  char *last_elev; /* last elevation data received from server */
  int elev_size; /* bytes in last_elev */
  
  /* Dynamic Values */
  double h_last_scale; /* Horizontal scale factor & bias to restore original value */
  double h_last_bias;
  double h_last_missing; /* Store bad, missing and transform to see if we need to recompute color lookup */
  double h_last_bad;
  int h_last_transform;
  
  double v_last_scale; /* Vertical scale factor & bias to restore original value */
  double v_last_bias;
  double v_last_missing; /* Store bad, missing and transform to see if we need to recompute color lookup */
  double v_last_bad;
  int v_last_transform;
  
  double cscale_min; /* Range of data to display within chosen colorscale */
  double cscale_delta; 
  
  double overlay_min; /* Range of data to display as an overlaid field */
  double overlay_max; 
  
  double cont_low; /* contour lower limit */
  double cont_high; /* contour upper limit */
  double cont_interv; /* contour interval */
  
  double time_allowance; /* Valid while less than this number of minutes out of date */
  // double time_offset; /* Offsets data requests by this amount - minutes*/
  
  char units_label_cols[LABEL_LENGTH]; /* units of columns- "km", etc */
  char units_label_rows[LABEL_LENGTH]; /* units of rows- "km", etc */
  char units_label_sects[LABEL_LENGTH];/* units of sections- "mbar", etc */
  char vunits_label_cols[LABEL_LENGTH];/* units of vert columns- "km", etc */
  char vunits_label_rows[LABEL_LENGTH];/* units of vert rows- "km", etc */
  char vunits_label_sects[LABEL_LENGTH];/* units of vert sections- "mbar", etc */

  string field_units; /* Units label of the data */
  string button_name; /* Field name for buttons, label, - "dbZ" etc - From Config file */
  string legend_name; /* name for legends , - "MHR" etc - From Config file */
  string field_label; /* field name, label, - "dbZ" etc - As reported by Data source */
  string url; /* server URL- mdvp:://host:port:dir */
  string color_file; /* color scale file name */
  
  // char field_units[LABEL_LENGTH]; /* Units label of the data */
  // char button_name[NAME_LENGTH]; /* Field name for buttons, label, - "dbZ" etc - From Config file */
  // char legend_name[NAME_LENGTH]; /* name for legends , - "MHR" etc - From Config file */
  // char field_label[NAME_LENGTH]; /* field name, label, - "dbZ" etc - As reported by Data source */
  // char url[URL_LENGTH]; /* server URL- mdvp:://host:port:dir */
  // char color_file[NAME_LENGTH]; /* color scale file name */
  
  unsigned short *h_data; /* pointer to Horizontal int8 data */
  unsigned short *v_data; /* pointer to Vertical d int8 data */ 
  
  fl32 *h_fl32_data; /* pointer to Horizontal fl32 data */
  fl32 *v_fl32_data; /* pointer to Vertical fl32 data */ 
  
  time_list_t time_list; // A list of data times
  
  /* Dynamic - Depends on scale & bias factors */
  Valcolormap_t h_vcm; /* Data value to X color info */
  Valcolormap_t v_vcm; /* Data value to X color info */
  
  DateTime h_date; /* date and time stamp of latest data - Horiz */
  DateTime v_date; /* date and time stamp of latest data - Vert */
  
  MdvxProj *proj; /* Pointer to projection class */
  
  // MDV Data class sets - One for horizontal, one for vertical
  // DsMdvxThreaded *h_mdvx;
  DsMdvx *h_mdvx;
  MdvxField *h_mdvx_int16;
  Mdvx::master_header_t h_mhdr;	
  Mdvx::field_header_t h_fhdr;
  Mdvx::vlevel_header_t h_vhdr;
  int iret_h_mdvx_read;
  
  Mdvx::field_header_t ds_fhdr; // Data Set's Field header
  Mdvx::vlevel_header_t ds_vhdr; // Vertical height headers
  
  // DsMdvxThreaded *v_mdvx;
  DsMdvx *v_mdvx;
  MdvxField *v_mdvx_int16;
  Mdvx::master_header_t v_mhdr;
  Mdvx::field_header_t v_fhdr;
  Mdvx::vlevel_header_t v_vhdr;
  
  ColorMap *colorMap;

public slots:
  void readDoneH();

private:

  QObject *_lucid;
  mutable QMutex _statusMutex;

  // data request details
  
  DateTime _midTime;
  DateTime _timeReq;
  LatLonBox _zoomBoxReq;                 // horiz data
  double _vLevelReq;                     // horiz data
  WayPts _wayPtsReq;                     // vert section data
  
  bool _validH, _validV;
  mutable bool _newH, _newV;

  // data read and returned
  
  int _page;
  bool _timeListValid;
  double _vLevel;
  bool _readBusyH;

  // data status

  bool _checkRequestChangedH(const DateTime &midTime, double vLevel);
  bool _checkRequestChangedV(const DateTime &midTime);

  void _computeReqTime(const DateTime &midTime,
                       DateTime &reqTime);
  
  void _setReadTimes(const string &url,
                     const DateTime &reqTime,
                     Mdvx *mdvx);
  
  int _getTimeList(const string &url,
                   const DateTime &midTime,
                   int page,
                   Mdvx *mdvx);
  
  string _getFullUrl();
  
  string _getFieldName();

  void _setValidH(bool state);
  void _setValidV(bool state);
  void _setNewH(bool state);
  void _setNewV(bool state);

  void _getBoundingBox(double &min_lat, double &max_lat,
                       double &min_lon, double &max_lon);
  
  void _adjustBoundingBox(double lat, double lon,
                          double &minLat, double &maxLat,
                          double &minLon, double &maxLon);

  void _autoscaleVcm(Valcolormap_t *vcm, double min, double max);
  
};

// Worker for read H volume in thread

class ReadVolH : public QObject {
  
  Q_OBJECT

public:
  
  ReadVolH(MdvReader* parentObject, QObject* parent = nullptr);

public slots:
  void doRead();
  
signals:
  void readDone();

private:
  MdvReader* _mr;

};

#endif
