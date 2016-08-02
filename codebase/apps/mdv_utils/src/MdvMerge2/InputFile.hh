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
// Input file handling
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2004
//
/////////////////////////////////////////////////////////////

#ifndef INPUTFILE_HH
#define INPUTFILE_HH

#include <string>
#include <vector>
#include <ctime>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>
#include "XyLookup.hh"
#include "Params.hh"
using namespace std;

class InputFile {
  
public:
  
  InputFile(const string &prog_name,
	    const Params &params,
	    Params::input_url_t &input_url,
	    const MdvxProj &output_grid);
  
  ~InputFile();

  // read in the relevant file, process it and add to the merge
  // returns 0 on success, -1 on failure
  
  int process(const time_t& requestTime,
	      int fcstLeadTime,
	      int nxyOut,
              int nzOut,
	      vector<void *> merged,
	      vector<ui08 *> count,
	      vector<time_t *> latest_time,
              fl32 *closestRange,
              int *closestFlag,
	      Mdvx::master_header_t &mhdr,
	      vector<Mdvx::field_header_t> &fhdrs);
  
  // get methods
  
  const DsMdvx &getMdvx() const { return _mdvx; }
  const MdvxProj &getProj() const { return _proj; }
  const vector<MdvxField *> &getFields() const { return _fields; }
  const bool getIsRequired() const { return _isRequired; }

  // get data times
  
  time_t getStartTime() const { return _timeStart; }
  time_t getCentroidTime() const { return _timeCentroid; }
  time_t getEndTime() const { return _timeEnd; }
  
  // did the object construct OK
  
  bool OK() const { return (_OK); } 
  
  // get file path in use
  
  string getPathInUse() const { return _pathInUse; }

protected:
  
private:
  
  bool _OK;
  const string &_progName;
  const Params &_params;
  const MdvxProj &_outputProj; // output proj params
  
  bool _isRequired; //as set by the params

  // data request

  string _url;                 // data URL
  vector<string> _fieldNames; // if names are requested
  vector<int> _fieldNums;     // if numbers are requested
  
  // after a read
  
  DsMdvx _mdvx;                 // Mdvx input object
  MdvxProj _proj;               // projection from latest file read
  string _pathInUse;            // actual path used on read
  vector<MdvxField *> _fields;  // pointer to each field in returned data
  time_t _timeStart;            // start time for data
  time_t _timeCentroid;         // centroid time for data
  time_t _timeEnd;              // end time for data

  // lookup tables
  
  vector<XyLookup *> _lookupTables; // lookup tables for each field
  
  // functions
  
  void _addToMerged(const vector<MdvxField *> &fields,
		    int nxyOut, int nzOut,
		    vector<void *> merged,
		    vector<ui08 *> count,
		    vector<time_t *> latest_time,
                    fl32 *closestRange,
                    int *closestFlag);
  
  void _setClosestFlag(const vector<MdvxField *> &fields,
                       int nxyOut, int nzOut,
                       fl32 *closestRange,
                       int *closestFlag);

  int _computeOutputPlaneIndex(int iz,
                               const Mdvx::coord_t &outCoord,
                               const Mdvx::field_header_t &fhdr,
                               const Mdvx::vlevel_header_t &vhdr);

  void _tokenize(const string &str, const string &spacer,
		 vector<string> &toks);
  
};

#endif
