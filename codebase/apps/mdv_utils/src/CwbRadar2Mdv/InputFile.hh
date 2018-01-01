/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 1997
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1997/9/26 14:18:54
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/////////////////////////////////////////////////////////////
// InputFile.hh
//
// InputFile object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2003
//
///////////////////////////////////////////////////////////////

#ifndef InputFile_H
#define InputFile_H

#include <dataport/port_types.h>
#include <toolsa/DateTime.hh>
#include <iostream>
#include <string>
#include <vector>
#include "Params.hh"
using namespace std;

class InputFile {
  
public:

  typedef struct {
    si32 year;
    si32 month;
    si32 day;
    si32 hour;
    si32 min;
    si32 sec;
    si32 nx;
    si32 ny;
    si32 nz;
    char proj[4];
    si32 mapScale;
    si32 trueLat1;
    si32 trueLat2;
    si32 trueLon;
    si32 nwLon;
    si32 nwLat;
    si32 xyScale;
    si32 dx;
    si32 dy;
    si32 dxyScale;
  } header_t;

  // constructor

  InputFile (const string &prog_name,
	     const Params &params);

  // destructor
  
  ~InputFile();

  // read in file

  int readFile(const string &file_path);
  
  // print the header and data array
  
  void printInfo(ostream &out) const;
  void printHeader(ostream &out) const;
  void printData(ostream &out) const;

  // access to data members
  
  const DateTime &getTime() const { return _time; }

  int getNx() const { return _nx; }
  int getNy() const { return _ny; }
  int getNz() const { return _nz; }
  int getMissing() const { return _missing; }
  int getIBBMode() const { return _iBBMode; }
  
  double getTrueLat1() const { return _trueLat1; }
  double getTrueLat2() const { return _trueLat2; }
  double getTrueLon() const { return _trueLon; }
  double getNwLat() const { return _nwLat; }
  double getNwLon() const { return _nwLon; }
  double getDx() const { return _dx; }
  double getDy() const { return _dy; }
  
  const vector<double> &getZLevels() const { return _zLevels; }

  const string &getProjType() const { return _projType; }
  const string &getFieldName() const { return _fieldName; }
  const string &getFieldUnits() const { return _fieldUnits; }
  const vector<string> &getRadarNames() const { return _radarNames; }

  const fl32 *getFieldData() const { return _fieldData; }

protected:
private:
  
  const string _progName;
  const Params &_params;

  bool _swapped;
  header_t _header;

  DateTime _time;

  int _nx, _ny, _nz;
  int _iBBMode;
  int _missing;

  double _trueLat1, _trueLat2, _trueLon;
  double _nwLat, _nwLon;
  double _dx, _dy;
  vector<double> _zLevels;
  
  string _projType;
  string _fieldName;
  string _fieldUnits;
  vector<string> _radarNames;

  fl32 *_fieldData;

  void _swapHeader();
  string _convert2String(const char *input_text, int max_len);

};

#endif
