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

////////////////////////////////////////////////////////////////////
// mdv/MdvVsection.hh
//
// Class for representing vertical sections from Mdv files
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 1999
//
////////////////////////////////////////////////////////////////////

#ifndef MdvVsection_HH
#define MdvVsection_HH

#include <didss/DsDataFile.hh>
#include <Mdv/mdv/mdv_file.h>
#include <Mdv/mdv/MdvVsectionField.hh>
#include <vector>
#include <string>
#include <iostream>
using namespace std;

class MdvVsection
{

  friend class DsMdvMsg;

public:

  typedef struct {
    double lat;
    double lon;
  } wayPt_t;

  typedef struct {
    double lat;
    double lon;
    int segNum;
  } samplePt_t;

  typedef struct {
    double length;
    double azimuth;
  } segment_t;

  ///////////////////////
  // constructor
  
  MdvVsection();

  ///////////////////////
  // destructor

  virtual ~MdvVsection();

  /////////////////////////////////
  // building up a vsection request
  //
  // 1. Call clearRequest() unless object has just been constructed
  //    and has not yet been used.
  //
  // 2. Call setEncodingType() if you do not want MDV_INT8.
  //
  // 3. Call addFieldRequest() if you want to specify fields.
  //    A field may be specified by number or name. 
  //    Multiple fields may be added.
  //    If none added, all fields returned.
  //
  // 4. Add plane limits if you want to specify a range of planes.
  //    Planes may be specified either by number or vlevel.
  //    If no planes are specified, all planes are returned.
  //
  // 5. Call addWayPt() to add way-points.
  //    If only a single way-point is added, you will get column data.
  //
  // Steps 2 through 5 may be performed in any order.

  // clear request

  void clearRequest();

  // set encoding type

  void setEncodingType(int encoding_type = MDV_INT8);

  // add field request - either numbers or names may be used,
  // but do not mix them, they are mutually exclusive.
  // If no fields are specified, all fields are requested.

  void addFieldRequest(const int field_num);
  void addFieldRequest(const string &field_name);

  // set plane num limits
  // num and vlevel limits are mutually exclusive

  void setPlaneNumLimits(const int lower_plane_num, const int upper_plane_num);

  // set plane vlevel limits
  // num and vlevel limits are mutually exclusive

  void setPlaneVlevelLimits(const double lower_plane_vlevel,
			    const double upper_plane_vlevel);

  // add a way point

  void addWayPt(const double lat, const double lon);
  
  // print request

  void printRequest(ostream &out);

  // print sampling summary
  
  void printSampleSummary(ostream &out);

  // print field summary

  void printFieldSummary(ostream &out, int field_num);

  // print field data

  void printFieldData(ostream &out, int field_num);

  // Load vsection from data in MDV file

  int load(DsDataFile &file, int n_sample, string &errStr);

  ////////////////////////////////////////
  // data member access

  // get master header

  MDV_master_header_t &getMasterHeader() { return (_masterHeader); }

  // encoding type

  int getEncodingType() { return (_encodingType); }

  // get field objects

  int getNFields() { return (_fields.size()); }
  MdvVsectionField &getField(const int i) { return (_fields[i]); }
  vector<MdvVsectionField> &getFields() { return (_fields); }

  // get field requests

  vector<int> &getRequestFieldNums() { return (_requestFieldNums); }
  vector<string> &getRequestFieldNames() { return (_requestFieldNames); }

  // get way points

  vector<MdvVsection::wayPt_t> &getWayPts() { return (_wayPts); }
  
  // get sample points

  vector<MdvVsection::samplePt_t> &getSamplePts() { return (_samplePts); }
  double getDxKm() { return (_dxKm); }
  
  // get segments

  vector<segment_t> &getSegments() { return (_segments); }
  double getTotalLength() { return (_totalLength); }

  // plane limits

  bool planeNumLimits() { return (_planeNumLimits); }
  bool planeVlevelLimits() { return (_planeVlevelLimits); }
  int getLowerPlaneNum() { return (_lowerPlaneNum); }
  int getUpperPlaneNum() { return (_upperPlaneNum); }
  double getLowerPlaneVlevel() { return (_lowerPlaneVlevel); }
  double getUpperPlaneVlevel() { return (_upperPlaneVlevel); }

protected:

  MDV_master_header_t _masterHeader;

  vector<int> _requestFieldNums;
  vector<string> _requestFieldNames;
  int _encodingType;

  vector<MdvVsectionField> _fields;

  vector<MdvVsection::wayPt_t> _wayPts;

  vector<MdvVsection::samplePt_t> _samplePts;
  double _dxKm;

  vector<segment_t> _segments;
  double _totalLength;

  bool _planeNumLimits;
  bool _planeVlevelLimits;
  int _lowerPlaneNum;
  int _upperPlaneNum;
  double _lowerPlaneVlevel;
  double _upperPlaneVlevel;
  
  // clear memory

  void _clearRequestFieldNums();
  void _clearRequestFieldNames();
  void _clearPlaneLimits();
  void _clearWayPts();
  void _clearFields();
  void _clearSamplePts();
  void _clearSegments();
  void _clearAll();
  void _clearFieldRequests();

  void _addSamplePt(const double lat, const double lon,
		    const int segment_num);

  void _addSegment(const double length, const double azimuth);

private:

};

#endif




