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
//
// Niles Oien, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 2006
//
/////////////////////////////////////////////////////////////

#ifndef OUTPUT_MDV_HH
#define OUTPUT_MDV_HH

#include <string>
#include <vector>
#include <ctime>

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>


/**
 *
 * @file outputMdv.hh
 *
 * @class outputMdv
 *
 * Class to handle output of intermediate grids to MDV.
 * Deals with two and three dimensional data, float and ui08.
 *
 * @author Niles Oien
 *
 */
using namespace std;

class outputMdv {
  
public :

/**
 * The constructor. Makes copies of the three dimensional
 * master, field and vlevel header and sets up a two dimensional
 * surface vlevel header for local internal use.
 *
 * @param url The MDV URL to write to.
 * @param name The name for this particular threshold. Appended to the URL.
 * @param forecastFilename The forecast file name.
 * @param truthFilename The truth file name.
 * @param mhdr The MDV master header
 * @param fhdr The MDV field header, three dimensional.
 * @param vhdr The MDV three dimensional vlevel header.
 *
 * @return  No return value.
 *
 *
 * @author Niles Oien oien@ucar.edu
 *
 */ 
  outputMdv(string url,
	    string name,
	    string forecastFilename,
	    string truthFilename,
	    Mdvx::master_header_t mhdr,
	    Mdvx::field_header_t fhdr,
	    Mdvx::vlevel_header_t vhdr);



/**
 * Add a data field of type fl32. Overloaded with adding a type of ui08.
 *
 * @param data Pointer to the fl32 data. If NULL no field is added.
 * @param bad_data_value fl32 value to use for bad data.
 * @param missing_data_value fl32 value to use for missing data.
 * @param name Name of the output field
 * @param longName Long name for output field
 * @param units Units of output field.
 * @param is3D Boolean indicating if data dimensions are nx by ny
 *             or nx by ny by nz
 *
 * @return  None
 *
 * @author Niles Oien oien@ucar.edu
 *
 */ 
  void addField(fl32 *data,
		fl32 bad_data_value,
		fl32 missing_data_value,
		string name,
		string longName,
		string units,
		bool is3D);


/**
 * Add a data field of type ui08. Overloaded with adding a type of fl32.
 *
 * @param data Pointer to the ui08 data. If NULL no field is added.
 * @param bad_data_value fl32 value to use for bad data.
 * @param missing_data_value fl32 value to use for missing data.
 * @param name Name of the output field
 * @param longName Long name for output field
 * @param units Units of output field.
 * @param is3D Boolean indicating if data dimensions are nx by ny
 *             or nx by ny by nz
 *
 * @return  None
 *
 * @author Niles Oien oien@ucar.edu
 *
 */ 
  void addField(ui08 *data,
		fl32 bad_data_value,
		fl32 missing_data_value,
		string name,
		string longName,
		string units,
		bool is3D);



/**
 * Set the MDV write mode to be forecast - or not.
 *
 * @param forecastMode Boolean indicating if we are in forecast mode. Or not.
 *
 * @return  None
 *
 * @author Niles Oien oien@ucar.edu
 *
 */ 
  void setForecastMode( bool forecastMode );

/**
 * Destructor. Does the MDV write, if fields were added.
 *
 * @return  None.
 *
 * @author Niles Oien oien@ucar.edu
 *
 */ 
  ~outputMdv();
  
protected:
  
private:

  bool _forecastMode;
  string _url;
  Mdvx::master_header_t _mhdr;
  Mdvx::field_header_t _fhdr;
  Mdvx::vlevel_header_t _vhdr;
  Mdvx::vlevel_header_t _vhdr2D;

  DsMdvx _outMdvx;

};

#endif
