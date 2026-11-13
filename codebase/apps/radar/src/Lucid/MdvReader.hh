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

#ifndef MDV_READER_HH
#define MDV_READER_HH

#include <QObject>
#include <QMutex>
#include <QSet>
#include <QMetaType>

#include <memory>

#include <toolsa/DateTime.hh>
#include <toolsa/mem.h>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxRadar.hh>
#include <qtplot/ColorMap.hh>

#include "GlobalData.hh"
#include "Params.hh"

#include "LatLonBox.hh"
#include "WayPts.hh"

class MdvReader;
class ReadVolH;
class ReadVolV;
class QThread;
class QTimer;

// Lucid permits interactive color-scale edits while retaining the detailed
// spacing defined by the source color map.  ColorMap's standard range setters
// rebuild equal-width entries, so this specialization exposes that distinct
// rescaling operation while keeping its lookup-table internals encapsulated.
class LucidColorMap : public ColorMap {
public:
  LucidColorMap() :
          _originalRangeSet(false),
          _originalRangeMin(0.0),
          _originalRangeMax(0.0) {}

  bool saveOriginalRange();
  bool rescaleRange(double rangeMin, double rangeMax);
  bool resetRange();
  bool hasOriginalRange() const { return _originalRangeSet; }

private:
  bool _originalRangeSet;
  double _originalRangeMin;
  double _originalRangeMax;
};

// Vertical reads use self-contained request and result objects.  The blocking
// worker never mutates MdvReader, so horizontal and vertical requests can be
// in flight together without sharing transient request state.

struct VertReadRequest {
  DateTime requestTime;
  string url;
  string fieldName;
  WayPts wayPts;
  int page;
  int generation;
  int gatherMode;
  int searchMarginSecs;
  time_t modelRunTime;
  bool isForecast;
  bool requestCompressedData;
  bool requestGzipCompression;
};

struct VertReadResult {
  int status;
  int page;
  int generation;
  string error;
  Mdvx::master_header_t mhdr;
  Mdvx::field_header_t fhdr;
  Mdvx::vlevel_header_t vhdr;
  vector<fl32> data;
  WayPts returnedWayPts;
  vector<Mdvx::vsect_segment_t> segments;
  double sampleSpacingKm;
  double totalLengthKm;
};

using VertReadResultPtr = std::shared_ptr<VertReadResult>;
Q_DECLARE_METATYPE(VertReadResultPtr)

class MdvReader : public QObject {

  Q_OBJECT

public:

  // constructor
  
  MdvReader(QObject* parent);

  // destructor

  ~MdvReader() override;

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

  void invalidateTimeList() { _timeListValid = false; }
  bool getTimeListValid() const { return _timeListValid; }
  const vector<time_t> &getTimeList() const { return _timeList; }
  
  // Get data for a horiz plane
  // This is implemented in a thread.
  // On error, isValidH is set to false.

  int requestHorizPlane(const DateTime &midTime,
                        double vLevel,
                        int page);

  int getHorizGeneration() const { return _horizGeneration; }
  int getHorizCompletedGeneration() const {
    return _horizCompletedGeneration;
  }
  void cancelHorizRequest(int generation);

  int getHorizPlane();
  
  // Get data for a vert section

  int requestVertSection(const DateTime &midTime,
                         int page,
                         const WayPts &wayPts);

  const WayPts &getVertWayPts() const { return _vertWayPts; }
  const vector<Mdvx::vsect_segment_t> &getVertSegments() const {
    return _vertSegments;
  }
  double getVertSampleSpacingKm() const { return _vertSampleSpacingKm; }
  double getVertTotalLengthKm() const { return _vertTotalLengthKm; }
  int getVertGeneration() const { return _vertGeneration; }
  int getVertCompletedGeneration() const {
    return _vertCompletedGeneration;
  }
  void cancelVertRequest(int generation);
  
  // perform the vol read for Horiz

  void startReadVolH();

  // get labels for legends
  
  string vlevelLabel() const;
  string fieldLabelHoriz() const;
  string fieldLabelVert() const;
  string heightLabel() const;
  
  // public data members
  
  int plane; /* plane of data rendered in horiz visible area */
  int currently_displayed; /* flag indicating if field in field list */
  int auto_render; /* flag indicating html/ automatic rendering */
  // Keep the base fill independent from its optional line overlay so either
  // polygon or filled-contour rendering can carry the same contour lines.
  int render_method;
  bool overlay_line_contours;
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
  
  vector<vert_spacing_t> vert; // Holds min,cent and max for each plane.
  
  double ht_pixel; // Slope of function between elevation height and screen Y
  double y_intercept; // Intercept of function between elevation and screen Y
  
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
  
  string units_label_cols; /* units of columns- "km", etc */
  string units_label_rows; /* units of rows- "km", etc */
  string units_label_sects;/* units of sections- "mbar", etc */
  string vunits_label_cols;/* units of vert columns- "km", etc */
  string vunits_label_rows;/* units of vert rows- "km", etc */
  string vunits_label_sects;/* units of vert sections- "mbar", etc */

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
  
  // vector<unsigned short> h_data; /* pointer to Horizontal int8 data */
  // vector<unsigned short> v_data; /* pointer to Vertical d int8 data */ 
  
  vector<ui32> h_rgba32_data; /* image data - i.e. for maps */
  vector<fl32> h_fl32_data; /* horizontal fl32 data */
  vector<fl32> v_fl32_data; /* vertical fl32 data */ 
  
  // time_list_t time_list; // A list of data times
  
  /* Dynamic - Depends on scale & bias factors */
  Valcolormap_t h_vcm; /* Data value to X color info */
  Valcolormap_t v_vcm; /* Data value to X color info */
  
  DateTime h_date; /* date and time stamp of latest data - Horiz */
  DateTime v_date; /* date and time stamp of latest data - Vert */
  
  MdvxProj proj; /* Pointer to projection class */
  
  // MDV Data class sets - One for horizontal, one for vertical

  DsMdvx h_mdvx;
  // MdvxField h_mdvx_int16;
  Mdvx::master_header_t h_mhdr;	
  Mdvx::field_header_t h_fhdr;
  Mdvx::vlevel_header_t h_vhdr;
  int iret_h_mdvx_read;
  
  Mdvx::field_header_t ds_fhdr; // Data Set's Field header
  Mdvx::vlevel_header_t ds_vhdr; // Vertical height headers
  
  DsMdvx v_mdvx;
  // MdvxField v_mdvx_int16;
  Mdvx::master_header_t v_mhdr;
  Mdvx::field_header_t v_fhdr;
  Mdvx::vlevel_header_t v_vhdr;

  // radar info if applicable

  MdvxRadar mdvxRadar;
  DsRadarParams radarParams;
  bool isRadar;

  // color scale
  
  LucidColorMap colorMap;

  // copy of field data, saved for later
  
  class FieldCopy {
  public:
    FieldCopy(const string &fieldName_,
              const Mdvx::master_header_t &mhdr_,
              const Mdvx::field_header_t &fhdr_,
              const Mdvx::vlevel_header_t &vhdr_,
              const MdvxProj &proj_,
              const vector<fl32> &fl32_data_):
            fieldName(fieldName_),
            mhdr(mhdr_),
            fhdr(fhdr_),
            vhdr(vhdr_),
            proj(proj_),
            fl32_data(fl32_data_)
    {
      fieldName = fhdr.field_name;
    }
    string fieldName;
    Mdvx::master_header_t mhdr;
    Mdvx::field_header_t fhdr;
    Mdvx::vlevel_header_t vhdr;
    MdvxProj proj;
    vector<fl32> fl32_data;
  };

signals:
  void verticalReadComplete(MdvReader *reader);

public slots:
  void readDoneH(int readStatus, int generation);
  void readDoneV(VertReadResultPtr result);

private:

  Params &_params;
  GlobalData &_gd;
  QObject &_lucid;
  
  mutable QMutex _statusMutex;

  // data request details
  
  DateTime _midTime;
  DateTime _timeReq;
  LatLonBox _zoomBoxReq;                 // horiz data
  double _vLevelReq;                     // horiz data
  bool _compositeModeReq;                // immutable for an active H read
  double _compositeMinHtReq;
  double _compositeMaxHtReq;
  
  bool _validH, _validV;
  mutable bool _newH, _newV;

  // data read and returned
  
  int _page;
  bool _timeListValid;
  // The worker fills the pending list; successful horizontal completion
  // publishes it on the GUI thread alongside the matching grid data.
  vector<time_t> _pendingTimeList;
  vector<time_t> _timeList;
  double _vLevel;
  bool _readBusyH;
  int _horizGeneration;
  int _horizCompletedGeneration;

  // Horizontal MDV reads are performed by short-lived worker threads.  Keep
  // track of every thread because the worker calls back into this MdvReader;
  // destruction must wait until those calls have returned.  Failed reads are
  // retried from the Qt event loop using a single-shot timer, with the most
  // recent request details still stored in this object.

  QSet<QThread *> _readThreadsH;
  QSet<QThread *> _readThreadsV;
  QTimer *_retryTimerH;
  int _retryCountH;
  bool _shuttingDown;

  // Geometry returned by MDV can differ slightly from the requested route.
  // Preserve the returned definition with the data so rendering and labels
  // describe the section which was actually sampled.

  WayPts _vertWayPts;
  vector<Mdvx::vsect_segment_t> _vertSegments;
  double _vertSampleSpacingKm;
  double _vertTotalLengthKm;
  int _vertGeneration;
  int _vertCompletedGeneration;

  // private methods

  int _handleHorizReadDone();

  void _scheduleRetryH();

  void _startReadVolV(const VertReadRequest &request);
  
  bool _checkRequestChangedH(const DateTime &midTime, double vLevel);

  void _computeReqTime(const DateTime &midTime,
                       DateTime &reqTime);
  
  void _setReadTimes(const string &url,
                     const DateTime &reqTime,
                     Mdvx &mdvx);
  
  int _getTimeList(const string &url,
                   const DateTime &midTime,
                   int page,
                   Mdvx &mdvx);
  
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
  void readDone(int readStatus);

private:
  MdvReader* _mr;

};

// Worker for a vertical-section read.  It owns the complete request and emits
// an owned result; its lifetime is therefore independent of MdvReader.

class ReadVolV : public QObject {

  Q_OBJECT

public:

  explicit ReadVolV(const VertReadRequest &request,
                    QObject *parent = nullptr);

public slots:
  void doRead();

signals:
  void readDone(VertReadResultPtr result);

private:
  VertReadRequest _request;

};

#endif
