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
// InputFile.hh: Input file handling
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 1998
//
/////////////////////////////////////////////////////////////

#ifndef INPUTFILE_H
#define INPUTFILE_H

#include <string>
#include <vector>
#include <ctime>

#include <Mdv/DsMdvx.hh>

using namespace std;

typedef struct {
  double lat, lon;
} lat_lon_t;


class Args;
class Params;
class DsMdvxInput;
class MdvxField;
class MdvxProj;

class InputFile {
  
public:

  InputFile(string prog_name, Params *params,
	    Args *args, vector<int> field_list, 
	    const string& url, int master,
	    MdvxProj *output_grid);

  InputFile(string prog_name, Params *params,
	    Args *args, vector<string> field_list, 
	    const string& url, int master,
	    MdvxProj *output_grid);


  ~InputFile();

  // read in the relevant file
  int read(const time_t& request_time);

  // get min and max vals for a field
  int getMinAndMax(const int& field_num, double& min_val, double& max_val);
  
  // get handle
  DsMdvx *handle() { return (_input); }

  // get grid
  MdvxProj *grid() { return (_grid); }

  // get x-y lookup table
  vector<long> *xyLut() { return (&_xyLut); }

  // get z lookup table
  vector<int> *zLut() { return (&_zLut); }

  // update the start and end times
  void updateTimes(time_t& start_time, time_t& end_time);

  // was latest read a success?
  bool readSuccess() const { return (_readSuccess); } // set true if last read was a success

  // 
  bool OK() const { return (_OK); } 

  // file path
  string path() const { return (_path); }

protected:
  
private:

  string _progName;
  Params *_params;
  Args *_args;
  bool _readSuccess;
  bool _OK;
  string _url; // data URL

  MdvxProj *_grid; // grid from latest file read

  MdvxProj *_outputGrid; // output grid params

  // set _master to TRUE if this is the master data set - i.e. the
  // first in the list
  int _master;

  vector<int> _fieldNums; // list of field numbers
  vector<string> _fieldNames; // list of field names
  vector<int> _fieldList; // list of fields to be read

  DsMdvx *_input; // source for input

  string _path; // actual path used

  time_t _time; // actual time for data in file

  vector<lat_lon_t> _locArray;

  vector<long> _xyLut; // Lookup table in (x,y)
  vector<int> _zLut;   // Lookup table in z

  bool _isDzConstant;
  Mdvx::vlevel_header_t _vlevelHeader;

  // functions

  void _loadGrid();
  void _computeXYLookup();
  void _computeZLookup();
  void _computeZLookupByDz();
  void _computeZLookupByVLevel();

};

#endif
