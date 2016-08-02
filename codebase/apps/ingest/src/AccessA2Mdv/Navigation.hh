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

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/*
 *  $Id: Navigation.hh,v 1.7 2016/03/07 01:22:59 dixon Exp $
 *
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

//////////////////////////////////////////////////////////////////////////
// 
// Header:	Navigation
// 
// Author:	G. M. Cunning
// 
// Date:	Fri Jul 22 21:30:38 2011
// 
// Description:	This class handle the mapping fields in the netcdf to 
//		the output projection specified in the parameter file.
// 
// 


# ifndef    NAVIGATION_H
# define    NAVIGATION_H

// C++ include files
#include <string>
#include <vector>

// System/RAP include files
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>
#include <netcdf.hh>

// Local include files
#include "Params.hh"

using namespace std;

class Navigation {
  
public:

  ////////////////////
  // public methods //
  ////////////////////

  // constructor
  Navigation();

  Navigation(const Params *params/*,
	     size_t grid_nx,
	     size_t grid_ny,
	     size_t grid_nz,
	     double grid_minx,
	     double grid_miny,
	     double grid_minz,
	     double grid_dx,
	     double grid_dy,
	     double grid_dz*/);


  // destructor
  virtual ~Navigation();

  bool getDimensionVarData(const string name,
                           size_t size,
                           vector<float>& dataVec,
                           NcFile &ncf);

  void initCoords(float lat0,float lat1,
                  float lon0, float lon1,
                  float lvl0, float lvl1);

  // initialize on new file
  bool initialize(NcFile &ncf);

  // process variable  
  bool processNcVar(const string &fieldName,
		    const string &fieldNameLong,
		    const string &units,
		    time_t dateTime,
		    const NcVar* nc_var, 
		    MdvxField** mdvx_field);

  // clean up after each file
  void cleanup();

  void setFhr(int fhr) { _fhr = fhr; }

protected:

  ///////////////////////
  // protected members //
  ///////////////////////
  

  ///////////////////////
  // protected methods //
  ///////////////////////

private:

  /////////////////////
  // private members //
  /////////////////////
  static const string _className;
  static const fl32 _missingFloat = -9999.0;

  const Params* _params;

  MdvxProj _proj;
  size_t _nx;
  size_t _ny;
  size_t _nz;
  double _minx;
  double _miny;
  double _minz;
  double _dx;
  double _dy;
  double _dz;
  size_t _nLat;
  size_t _nLon;
  size_t _nLvl;
  size_t _nTime;
  int  _fhr;
  bool _isFlipped;

  Mdvx::vlevel_header_t _vhdr;
  

};

# endif     /* NAVIGATION_H */

