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
// LtgSpdb2Mdv.hh
//
// LtgSpdb2Mdv object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2000
//
///////////////////////////////////////////////////////////////
//
// Performs gridding of lightning data
//
///////////////////////////////////////////////////////////////

#ifndef LtgSpdb2Mdv_H
#define LtgSpdb2Mdv_H

#include <string>
#include <vector>

#include <dsdata/DsTrigger.hh>
#include <Mdv/DsMdvxInput.hh>
#include <Mdv/MdvxProj.hh>
#include <rapformats/ltg.h>
#include <Spdb/DsSpdb.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;


////////////////////////
// This class

class LtgSpdb2Mdv {
  
public:

  typedef struct {
    int x_offset;
    int y_offset;
  } grid_offset_t;

  // constructor

  LtgSpdb2Mdv (int argc, char **argv);

  // destructor
  
  ~LtgSpdb2Mdv();

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
  DsTrigger *_trigger;
  vector<grid_offset_t> _kernel;

  void _loadKernel();

  int _readLtg(time_t trigger_time,
	       const MdvxProj &proj,
	       vector<LTG_extended_t> &strikes);

  void _readGroups(const Spdb::chunk_t &chunk,
		   const double min_lon,
		   const bool normalize_lon,
		   int &nstrikesFound,
		   vector<LTG_extended_t> &strikes) const;
  
  void _readGenPts(const Spdb::chunk_t &chunk,
		   const double min_lon,
		   const bool normalize_lon,
		   int &nstrikesFound,
		   vector<LTG_extended_t> &strikes) const;
  
  void _readExtendedStrikes(const Spdb::chunk_t &chunk,
			    const double min_lon,
			    const bool normalize_lon,
			    int &nstrikesFound,
			    vector<LTG_extended_t> &strikes) const;
  
  void _readOriginalStrikes(const Spdb::chunk_t &chunk,
			    const double min_lon,
			    const bool normalize_lon,
			    int &nstrikesFound,
			    vector<LTG_extended_t> &strikes) const;
  
  bool _acceptStrike(const LTG_extended_t &strike) const;

  void _loadRateField(const vector<LTG_extended_t> &strikes,
		     DsMdvx &mdvx);

  void _loadDistanceField(const vector<LTG_extended_t> &strikes,
			 DsMdvx &mdvx);

  void _loadDerivedField(DsMdvx &mdvx);

  double _getDerivedVal(const int num_strikes) const;

};

#endif

