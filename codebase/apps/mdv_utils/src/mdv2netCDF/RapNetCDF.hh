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
// RapNetCDF.hh - class that takes RAP data and
// writes it to netCDF files in the COARDS convention.
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// December 1999
//
/////////////////////////////////////////////////////////////

#ifndef RAP_NETCDF_HH
#define RAP_NETCDF_HH

#include <string>
#include <Mdv/MdvxField.hh>    // For MDV field header definition.
#include <toolsa/umisc.h>      // For MAX_PATH_LEN
#include <toolsa/udatetime.h>  // For time structure date_time_t 
#include "Params.hh"
using namespace std;
                                                                      
class RapNetCDF {

  public :

  // The NCD struct is used to write a field.
  
  typedef struct {

    unsigned int Nx, Ny, Nz;     // Data dimensions.
    float *x, *y, *z;            // Data indicies, usually lon, lat, height
    float *data;                 // Actual factual data - x changes fastest in memory
    float OriginX, OriginY;      // Origin of data, usually lon, lat.
    float missing, fill;         // Value for missing data, also value to use if no data written

    char *dataName, *dataUnits;  //  Name and units for the variable, eg. 'Pressure', 'KPa'
    char *longDataName;          //  Longer more descriptive name for dataset.
    char *xName, *xUnits;        //  X name typically 'lon', units 'degrees_east'
    char *yName, *yUnits;        //  Y name typically 'lat', units 'degrees_north'
    char *zName, *zUnits, *zUp;  //  *zUp is 'up' if increasing z => increasing height, else 'down'
    char *history;               // Usually set to 'From RAP' or some such thing.
    
    date_time_t dataTime;        // The time the data pertains to.

  } RapNCD_t;



  // Methods.

  // Constructor - init a netCDF file. Filenames under the COARDS convention
  // must end in '.nc' - if this is not the case, '.nc' will be 
  // appended to the filename. If T is non-NULL then the filename
  // will have the time prepended to it.

  RapNetCDF(const Params &params, date_time_t &data_time,
	    const string &out_path);

  // Destructor - closes the NetCDF file.
  ~RapNetCDF();
  
  // GetNCDFromMDVFieldHeader - returns a pointer to a RapNCD_t
  // structure given an MDV field header.

  void Mdv2NCD(MdvxField *field,
	       date_time_t dataTime,
	       RapNCD_t *R);

  // Frees up a RapNCD_t structure from Mdv2NCD
  void RapNCDFree( RapNCD_t N);


  // NCDWriteFirstVar - writes the first netCDF field to the open structure.
  // Returns 0 if all went well.
  int NCDWriteFirstVar( RapNCD_t N );

  //
  // NCDAddAnother adds another vaiable on the same co-ordinate axes
  // at the same time.
  //
  int NCDAddAnother(char *dataName,
		    char *dataUnits,
		    char *dataLongName,
		    float fill, float missing,
		    float *data);

  // Add global text and float attributes
  // returns 0 if OK
  int RapNCDAddGlobalText(char *AttName, char *Att);
  int RapNCDAddGlobalFloat(char *AttName, float *Att, int NumFloats);

  // Add text and float attributes - local to variable
  // Call after AddNCD
  // returns 0 if OK
  int RapNCDAddLocalText(char *AttName, char *Att);
  int RapNCDAddLocalFloat(char *AttName, float *Att, int NumFloats);



  private :

  const Params &_params;
  date_time_t _dataTime;
  string _outPath;

  int NcID;              // The NetCDF ID for the object.
  int varID;             // The ID for the variable
  bool IsOpen;           // Set to true when open.
  int DimID[4];          // Array of dimension ID's.
  int _Nz;               // Lest we forget Nz between calls
  char ActualFileName[MAX_PATH_LEN]; // File name actually used (as opposed to the one passed in).
  bool FirstVarDefined;

};


#endif
  




