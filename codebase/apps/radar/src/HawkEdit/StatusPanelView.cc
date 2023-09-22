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
///////////////////////////////////////////////////////////////
// StatusPanelView.cc
//
// Status Panel View
//
// Brenda Javornik, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2023
//
///////////////////////////////////////////////////////////////
//
// StatusPanelView manages the display of metadata for the 
// selected (file, sweep, ray, range) = (time, elevation, azimuth, range)
//
///////////////////////////////////////////////////////////////

#include "StatusPanelView.hh"

#include <string>
#include <cmath>
#include <iostream>

#include <QFont>
#include <QLabel>
#include <QGridLayout>
#include <QDateTime>
#include <cstdlib>
#include <toolsa/DateTime.hh>

#include "CloseEventFilter.hh"

using namespace std;


// Constructor


StatusPanelView::StatusPanelView(QWidget *parent)
{

  _parent = parent;

  // initialize

  //_radarLat = -9999.0;
  //_radarLon = -9999.0;
  //_radarAltKm = -9999.0;

  _altitudeInFeet = false;
  // end from DisplayManager    

  //_nGates = 1000;
  //_maxRangeKm = 1.0;

  _radarNameRow = 0;
  _dateRow = 1;
  _timeRow = 2; 
  _startingRow = 3;  
  _nrows = _startingRow;  // first three rows are radar name, date, and time

  cerr << "StatusPanelView: _nrows = " << _nrows << "startingRow = " << _startingRow << endl;

  // install event filter to catch when the StatusPanel is closed
  CloseEventFilter *closeFilter = new CloseEventFilter(this);
  installEventFilter(closeFilter);

  setAttribute(Qt::WA_DeleteOnClose);

  //setGeometry(30, 30, 300, 100);

QPalette pal = QPalette();

// set black background
// Qt::black / "#000000" / "black"
//pal.setColor(QPalette::Window, Qt::yellow);

//setAutoFillBackground(true); 
//setPalette(pal);
show();

  _init();
}

// destructor

StatusPanelView::~StatusPanelView()
{

  cerr << "StatusPanelView destructor called " << endl;

}

void StatusPanelView::reset() {
   // delete all the labels and set the number of rows to zero
  _nrows = StatusPanelView::_startingRow;
}


////////////////////////////////////////////
// refresh

void StatusPanelView::_refresh()
{
}

//
// There are set one (when the parameter file changes) 
//       - set font size
// and there are set multiple times:
//   1. when the ray, range (selected azimuth)
//   2. sweep changes (what azimuth to use? )
//   3. when the file changes (reset to 0 azimuth)
// 
void StatusPanelView::setFontSize(int params_label_font_size) {
  QLabel dummy;
  font = dummy.font();
  font2 = dummy.font();
  font6 = dummy.font();
  int fsize = params_label_font_size;
  int fsize2 = params_label_font_size + 2;
  int fsize6 = params_label_font_size + 6;
  font.setPixelSize(fsize);
  font2.setPixelSize(fsize2);
  font6.setPixelSize(fsize6);
}

void StatusPanelView::printHash() {

  HashyIt it;
  for (it=hashy.begin(); it != hashy.end(); ++it) {
    int key = it->first;
    switch (key) {
      case FixedAngleKey:
        cerr << "FixedAngleKey" << endl;
        break;
      case VolumeNumberKey:
        cerr << "VolumeNumberKey" << endl;
        break;
      default:
        cerr << "not found" << endl;

    }
  }
}


void StatusPanelView::clear() {
  // reset values to original 

}


//////////////////////////////////////////////
// create the status panel

void StatusPanelView::_init()
{
 
  Qt::Alignment alignLeft(Qt::AlignLeft);
  Qt::Alignment alignRight(Qt::AlignRight);
  Qt::Alignment alignCenter(Qt::AlignCenter);
  Qt::Alignment alignTop(Qt::AlignTop);

  // status panel - rows of label value pairs
  
  _statusLayout = new QGridLayout(this);
  setLayout(_statusLayout);
  //_statusLayout->setVerticalSpacing(5);

  _elevVal = _createStatusVal("Elev", "-99.99", _fsize2);
  _azVal = _createStatusVal("Az", "-999.99", _fsize2);

/*
struct S {
  string label; // =   "Volume",
  string emptyValue; // =  "0",
  string format;
  QLabel *value; 
  int fontSize;
};
*/

  hashy.reserve(LastKey);

  // what to do about float format?
  _metaDataS[FixedAngleKey] =   {"Fixed ang", "-99.99", "f6.2???", _fsize2, NULL, NULL, 0, 0};
  _metaDataS[VolumeNumberKey] = {"Volume", "0", "%d", _fsize, NULL, NULL, 0, 0};  
  _metaDataS[SweepNumberKey] =  {"Sweep", "0", "%d", _fsize, NULL, NULL, 0, 0};  

  _metaDataS[NSamplesKey] =   {"N samp", "0", "%d", _fsize, NULL, NULL, 0, 0};
  _metaDataS[NGatesKey] = {"N gates", "0", "%d", _fsize, NULL, NULL, 0, 0};  
  _metaDataS[GateSpacingKey] =  {"Gate len", "0", "%d", _fsize, NULL, NULL, 0, 4};  
  _metaDataS[PulseWidthKey] =   {"Pulse width", "-9999", "f6.2???", _fsize, NULL, NULL, 0, 2};
  _metaDataS[PrfModeKey] = {"PRF mode", "fixed", "%d", _fsize, NULL, NULL, 0, 0};  
  _metaDataS[PrfKey] =  {"PRF", "-9999", "%d", _fsize, NULL, NULL, 0, 0};  
  _metaDataS[NyquistKey] =   {"Nyquist", "-9999", "f6.2???", _fsize, NULL, NULL, 0, 1};
  _metaDataS[MaxRangeKey] = {"Max range", "-9999", "%d", _fsize, NULL, NULL, 0, 1};  
  _metaDataS[UnambiguousRangeKey] =  {"U-A range", "-9999", "%d", _fsize, NULL, NULL, 0, 1};  
  _metaDataS[PowerHKey] =   {"Power H", "-9999", "f6.2???", _fsize, NULL, NULL, 0, 1};
  _metaDataS[PowerVKey] = {"Power V", "-9999", "%d", _fsize, NULL, NULL, 0, 1};  
  _metaDataS[ScanNameKey] =  {"Scan name", "unknown", "%s", _fsize, NULL, NULL, 0, 0};  

  _metaDataS[SweepModeKey] =   {"Scan mode", "SUR", "f6.2???", _fsize, NULL, NULL, 0, 0};
  _metaDataS[PolarizationModeKey] = {"Pol mode", "Single", "%d", _fsize, NULL, NULL, 0, 0};  
  _metaDataS[LatitudeKey] =  {"Lat", "-999.999", "%d", _fsize, NULL, NULL, 0, 0};  
  _metaDataS[LongitudeKey] =   {"Lon", "-999.999", "f6.2???", _fsize, NULL, NULL, 0, 0};
  _metaDataS[AltitudeInFeetKey] = {"Alt(kft)", "-999.999", "%d", _fsize, NULL, NULL, 0, 0};  
  _metaDataS[AltitudeInKmKey] =  {"Alt(km)", "-999.999", "%d", _fsize, NULL, NULL, 0, 0};  

  _metaDataS[AltitudeRateFtsKey] =   {"AltRate(ft/s)", "-999.999", "f6.2???", _fsize, NULL, NULL, 0, 0};
  _metaDataS[AltitudeRateMsKey] = {"AltRate(m/s)", "-999.999", "%d", _fsize, NULL, NULL, 0, 0};  
  _metaDataS[SpeedKey] =  {"Speed(m/s)", "-999.999", "%d", _fsize, NULL, NULL, 0, 0};  
  _metaDataS[HeadingKey] =   {"Heading(deg)", "-999.999", "f6.2???", _fsize, NULL, NULL, 0, 0};
  _metaDataS[TrackKey] = {"Track(deg)", "-999.999", "%d", _fsize, NULL, NULL, 0, 0};  
  _metaDataS[SunElevationKey] =  {"Sun el (deg)", "-999.999", "%d", _fsize, NULL, NULL, 0, 3};  
  _metaDataS[SunAzimuthKey] =   {"Sun az (deg)", "-999.999", "f6.2???", _fsize, NULL, NULL, 0, 3};


  _metaDataS[GeoRefsAppliedKey] = {"Georefs applied?", "T/F", "%s", _fsize, NULL, NULL, 0, 0};  
  _metaDataS[GeoRefRotationKey] =  {"Georef Rot (deg)", "0.0", "%d", _fsize, NULL, NULL, 0, 3};
  _metaDataS[GeoRefRollKey] =  {"Georef Roll (deg)", "0.0", "%d", _fsize, NULL, NULL, 0, 3};            
  _metaDataS[GeoRefTiltKey] =   {"Georef Tilt (deg)", "0.0", "f6.2???", _fsize, NULL, NULL, 0, 3};

  _metaDataS[GeoRefTrackRelRotationKey] = {"Track Rel Rot (deg)", "0.0", "%d", _fsize, NULL, NULL, 0, 3};  
  _metaDataS[GeoRefTrackRelTiltKey] =  {"Track Rel  Tilt (deg)", "0.0", "%d", _fsize, NULL, NULL, 0, 3};  
  _metaDataS[GeoRefTrackRelAzimuthKey] =   {"Track Rel  Az (deg)", "0.0", "f6.2???", _fsize, NULL, NULL, 0, 3};
  _metaDataS[GeoRefTrackRelElevationKey] = {"Track Rel  El (deg)", "0.0", "%d", _fsize, NULL, NULL, 0, 3};  

  _metaDataS[CfacRotationKey] =  {"Cfac Rot (deg)", "0.0", "%d", _fsize, NULL, NULL, 0, 3};  
  _metaDataS[CfacRollKey] =   {"Cfac Roll (deg)", "0.0", "f6.2???", _fsize, NULL, NULL, 0, 3};
  _metaDataS[CfacTiltKey] = {"Cfac Tilt (deg)", "0.0", "%d", _fsize, NULL, NULL, 0, 3};  
        

}

  // radar and site name
  

void StatusPanelView::setRadarName(string radarName, string siteName) {
  _radarName = new QLabel(this);
  string rname(radarName);
  if (siteName.length() > 0) {
    rname.append(":");
    rname.append(siteName);
  }
  _radarName->setText(rname.c_str());
  _radarName->setFont(font6);

  // always put radar name at row zero
  //Qt::Alignment alignCenter(Qt::AlignCenter); 
  _statusLayout->addWidget(_radarName, _radarNameRow, 0, 1, 2, Qt::AlignCenter);
}

void StatusPanelView::createDateTime() {
  // date and time

  _dateVal = new QLabel("9999/99/99", this);
  _dateVal->setFont(font2);
  _statusLayout->addWidget(_dateVal, _dateRow, 0, 1, 2, Qt::AlignCenter); // , row, 0, 1, 2, alignCenter);
  

  _timeVal = new QLabel("99:99:99.999", this);
  _timeVal->setFont(font2);
  _statusLayout->addWidget(_timeVal, _timeRow, 0, 1, 2, Qt::AlignCenter); // , row, 0, 1, 2, alignCenter);
  
}

void StatusPanelView::createGeoreferenceLabels() {

  for (int key = GeoRefsAppliedKey; key <= LastKey; key++) {
    createOnly(key);  
  }
  hideCfacs();
}

/*

  // other labels.  Note that we set the minimum size of the column
  // containing the right hand labels in timerEvent() to prevent the
  // wiggling we were seeing in certain circumstances.  For this to work,
  // the default values for these fields must represent the maximum digits
  // posible for each field.

*/

/*
// Q_OBJECT does not support templates (unless the template class inherits from the Q_OBJECT class)
// s is the value
// template<typename T>
*/
    void StatusPanelView::setInt(int s, QLabel *label) { // , string format) {
      if (label != NULL) {

        int value = -9999;

        if (abs(s) < 9999) {
          value = s;
        }
        label->setText(QString("%1").arg(value));
      }
    }
    
    void StatusPanelView::setDouble(double s, QLabel *label, int fieldWidth, int precision) {
      if (label != NULL) {

        double value = -9999;

        if (abs(s) < 9999) {
          value = s;
        }
        label->setText(QString("%1").arg(value, fieldWidth, 'f', precision));
      }
    }

     void StatusPanelView::setString(string s, QLabel *label) { // , string format) {
      if (label != NULL) {
        label->setText(QString(s.c_str()));
      }
    }   



/* s is the value
template<typename T>
    QLabel *StatusPanelView::create(T s, string textLabel, string format) {
      if (label == NULL) {
        _nrows += 1;
        label = _createStatusVal(textLabel, s, _nrows, _fsize2);
      } 
      return label;
    }    
*/
// Q: How do we know which row in the display? only need the row when creating.

//                <Key,         Hash,       KeyEqual,    Allocator>::find
//                <data_type> 
//std::unordered_map<int, S> hashy;
//hashy.insert(obj);
//hashy.find("one"); // returns an iterator ...

/*
struct S {
  string label; // =   "Volume",
  string emptyValue; // =  "0",
  string format;
  QLabel *value; 
  int fontSize;
};
*/
void StatusPanelView::createOnly(int key) {
  S *s = &_metaDataS[key];
  s->value = _createStatusVal(s->label, s->emptyValue, s->fontSize, &(s->guiLeftLabel));
}

void StatusPanelView::create(int key) {
  createOnly(key);
  S *s = &_metaDataS[key];
  //s->value = _createStatusVal(s->label, s->emptyValue, s->fontSize);
  hashy[key] = s; 
  // hashy[key] now has the pointer to the QLabel for the sweep number
}

// _view->set(SweepNumKey, ray->getSweepNumber());
void StatusPanelView::set(int key, int value) {
  S *s = hashy[key];
  setInt(value, s->value); // , "%d");  // set for int always uses "%d" format.
  //setInt(value, hashy[key], hashyFormat[key]);
}

void StatusPanelView::set(int key, double value) {
  S *s = hashy[key];
  setDouble(value, s->value, s->fieldWidth, s->precision); // 6, 2);
}

void StatusPanelView::set(int key, string value) {
  S *s = hashy[key];
  setString(value, s->value);
}

// Why keep the georefs separate? why not just put them into the hash table too?
// because the georefs are hidden sometimes if the data are NOT airborne.
void StatusPanelView::setNoHash(int key, int value) {
  setInt(value, _metaDataS[key].value);
}

void StatusPanelView::setNoHash(int key, double value) {
  setDouble(value, _metaDataS[key].value, _metaDataS[key].fieldWidth, _metaDataS[key].precision);
  // (double s, QLabel *label, int fieldWidth, int precision) 
}

void StatusPanelView::setNoHash(int key, string value) {
  setString(value, _metaDataS[key].value);
}

/*
void StatusPanelView::set(int key, double value) {
  S *s = hashy[key];
  setDouble(value, s->value); // , "%d");  // set for int always uses "%d" format.
  //setInt(value, hashy[key], hashyFormat[key]);
}
*/

// each of theses should be a class/factory object that are placed in a list
// the class must associate the ray varable with the panel variable.
// Use parallel lists / arrays.
// clear and initialize when new parameter file is read.
// stuff the label pointer into a list
// stuff the values from the ray into a list
// create: stuff the (strings, value as a string, font size) into a list
//    set: stuff the ray values into a list
// loop through the lists in parallel and set the values.
// loop through the list and create the labels.
// so much less code to maintain.



//////////////////////////////////////////////////
// create a row in the status panel

QLabel *StatusPanelView::_createStatusVal(const string &leftLabel,
                                         const string &rightLabel,
                                         int fontSize,
                                         QLabel **guiLeftLabel)  
{
  _nrows += 1;
  int row = _nrows;
  QLabel *left = new QLabel(this);
  left->setText(leftLabel.c_str());
  Qt::Alignment alignRight(Qt::AlignRight);
  _statusLayout->addWidget(left, row, 0, alignRight);
  if (guiLeftLabel != NULL) {
    *guiLeftLabel = left;
  }

  QLabel *right = new QLabel(this);
  right->setText(rightLabel.c_str());
  Qt::Alignment alignCenter(Qt::AlignCenter);
  _statusLayout->addWidget(right, row, 1, alignRight);

  if (fontSize > 0) {
    QFont font = left->font();
    font.setPixelSize(fontSize);
    left->setFont(font);
    right->setFont(font);
  }

  //_valsRight.push_back(right);

  return right;
}

//////////////////////////////////////////////
// update the status panel

void StatusPanelView::setDateTime(int year, int month, int day,
  int hour, int minutes, int seconds,  int nanoSeconds) {

  // set time etc

  QDate rayDate(year, month, day);
  _dateVal->setText(rayDate.toString("yyyy/MM/dd"));

  QTime rayTime(hour, minutes, seconds, (int) nanoSeconds / 1000000);
  _timeVal->setText(rayTime.toString("hh:mm:ss.zzz"));

}

void StatusPanelView::updateAzEl(double azDeg, double elDeg) {
  if (abs(azDeg) < 1000) {
    setDouble(azDeg, _azVal, 6, 2);
  }
  if (abs(elDeg) < 1000) {
    setDouble(elDeg, _elevVal, 6, 2);
  }

  clear();
}
  /*

   

  if (_sweepModeVal) {
    switch (ray->getSweepMode()) {
      case Radx::SWEEP_MODE_SECTOR: {
        _sweepModeVal->setText("sector"); break;
      }
      case Radx::SWEEP_MODE_COPLANE: {
        _sweepModeVal->setText("coplane"); break;
      }
      case Radx::SWEEP_MODE_RHI: {
        _sweepModeVal->setText("RHI"); break;
      }
      case Radx::SWEEP_MODE_VERTICAL_POINTING: {
        _sweepModeVal->setText("vert"); break;
      }
      case Radx::SWEEP_MODE_IDLE: {
        _sweepModeVal->setText("idle"); break;
      }
      case Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE:
      case Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE: {
        _sweepModeVal->setText("SUR"); break;
      }
      case Radx::SWEEP_MODE_SUNSCAN: {
        _sweepModeVal->setText("sunscan"); break;
      }
      case Radx::SWEEP_MODE_SUNSCAN_RHI: {
        _sweepModeVal->setText("sun_rhi"); break;
      }
      case Radx::SWEEP_MODE_POINTING: {
        _sweepModeVal->setText("point"); break;
      }
      case Radx::SWEEP_MODE_CALIBRATION: {
        _sweepModeVal->setText("cal"); break;
      }
      default: {
        _sweepModeVal->setText("unknown");
      }
    }
  }

    // compute altitude rate every 2 secs
    
    if (_prevAltKm > -9990) {
      double deltaTime = ray->getRadxTime() - _prevAltTime;
      if (deltaTime > 2.0) {
        double altKm = ray->getGeoreference()->getAltitudeKmMsl();
        double deltaAltKm = altKm - _prevAltKm;
        _altRateMps = (deltaAltKm / deltaTime) * 1000.0;
        _prevAltKm = ray->getGeoreference()->getAltitudeKmMsl();
        _prevAltTime = ray->getRadxTime();
      }
    } else {
      _prevAltKm = ray->getGeoreference()->getAltitudeKmMsl();
      _prevAltTime = ray->getRadxTime();
    }
}
*/
void StatusPanelView::hideCfacs() {

  for (int key = GeoRefsAppliedKey; key <= LastKey; key++) {
    //S s = _metaDataS[key];
    QLabel *value = _metaDataS[key].value;
    value->hide();
    QLabel *leftLabel = _metaDataS[key].guiLeftLabel;
    leftLabel->hide();
  }
}

void StatusPanelView::showCfacs() {

  for (int key = GeoRefsAppliedKey; key <= LastKey; key++) {
    //S s = _metaDataS[key];
    QLabel *value = _metaDataS[key].value;
    value->show();
    QLabel *leftLabel = _metaDataS[key].guiLeftLabel;
    leftLabel->show();
  }
}



////////////////////////////////////////////////////////////////
/*
double StatusPanelView::_getInstHtKm(const RadxRay *ray)

{
  double instHtKm = _platform.getAltitudeKm();
  if (ray->getGeoreference() != NULL) {
    instHtKm = ray->getGeoreference()->getAltitudeKmMsl();
  }
  return instHtKm;
}
*/
/////////////////////////////////////////////////////////////////////  
// slots


void StatusPanelView::closeEvent(QEvent *event)
{
 
    event->accept();
    //emit close();

}



