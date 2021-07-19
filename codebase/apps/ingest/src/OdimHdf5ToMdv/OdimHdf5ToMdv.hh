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
// OdimHdf5ToMdv.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2021
//
///////////////////////////////////////////////////////////////
//
// OdimHdf5ToMdv reads ODIM OPERA data in HDF5 format, and
// converts to MDV.
//
////////////////////////////////////////////////////////////////

#ifndef OdimHdf5ToMdv_H
#define OdimHdf5ToMdv_H

#include <string>
#include <toolsa/TaArray.hh>
#include <toolsa/DateTime.hh>
#include <didss/DsInputPath.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxRemapLut.hh>
#include <Ncxx/Hdf5xx.hh>
#include <euclid/point.h>
#include "Args.hh"
#include "Params.hh"
using namespace std;

////////////////////////
// This class

class OdimHdf5ToMdv {
  
public:

  // constructor

  OdimHdf5ToMdv (int argc, char **argv);

  // destructor
  
  ~OdimHdf5ToMdv();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  DsInputPath *_input;

  // times

  vector<DateTime> _times;
  DateTime _startTime, _endTime;
  DateTime _midTime;
  
  // metadata

  string _conventions;
  string _version;
  string _source;
  string _history;

  // geometry and dimensions

  string _projStr;

  size_t _nx, _ny;
  double _dxM, _dyM, _dxKm, _dyKm;
  double _minxM, _minxKm, _minyM, _minyKm;
  double _llLon, _llLat;
  double _ulLon, _ulLat;
  double _lrLon, _lrLat;
  double _urLon, _urLat;

  MdvxRemapLut _remapLut;

  // nz is forced to 1 for now
  
  size_t _nz;
  double _minzKm, _dzKm;
  vector<double> _zLevels;

  // inner class for field data

  class OutputField
  {
  public:

    OutputField(const Params::output_field_t &p) :
            params(p)
    {
      valid = false;
    }

    void clear() {
      fl32Input.clear();
      valid = false;
    }
    
    Params::output_field_t params;
    bool valid;

    Hdf5xx hdf5;
    vector<size_t> dims;
    string units;
    H5T_class_t h5class;
    H5T_sign_t h5sign;
    H5T_order_t h5order;
    size_t h5size;
    
    vector<NcxxPort::fl32> fl32Input;
    NcxxPort::fl32 fl32Missing;

    DateTime startTime;
    DateTime endTime;
    
    double gain;
    double offset;
    double nodata;
    double undetect;
    
  private:
    
  };
  
  vector<OutputField *> _outputFields;

  // hdf5 utilities

  Hdf5xx _utils;

  // private methods

  int _processFile(const char *input_path);

  void _readMetadata(Group &ns);
  void _readWhere(Group &root);

  int _readFields(Group &root);
  void _readField(Group &dataGrp);
  int _readFieldAttributes(Group &what,
                           const string &fieldName,
                           OutputField *fld);

  int _readFieldData(Group &dataGrp,
                     const string &fieldName,
                     OutputField *fld);

  int _readField3D(Group &grp,
                   const string &dsetName,
                   vector<NcxxPort::fl32> &vals,
                   NcxxPort::fl32 &missingVal,
                   string &units);
  
  int _readField2D(Group &grp,
                   const string &dsetName,
                   vector<NcxxPort::fl32> &vals,
                   NcxxPort::fl32 &missingVal,
                   string &units);
  
  /// set MDV headers and data

  void _setMasterHeader(DsMdvx &mdvx);

  // add the mdvx fields
  
  void _addFieldToMdvx(DsMdvx &mdvx, OutputField *fld);

  MdvxField *_createMdvxField(const string &fieldName,
                              const string &longName,
                              const string &units,
                              size_t nx, size_t ny, size_t nz,
                              double minx, double miny, double minz,
                              double dx, double dy, double dz,
                              NcxxPort::fl32 missingVal,
                              NcxxPort::fl32 *vals);
};

#endif

