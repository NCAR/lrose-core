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
////////////////////////////////////////////////////////////////
// NcOutput:  class that handles netCDF output
//
// Jaimi Yee, RAP, NCAR, Boulder, CO, 80307, USA
// August 2004
//
// $Id: NcOutput.hh,v 1.18 2018/02/16 20:45:38 jcraig Exp $
//
////////////////////////////////////////////////////////////////
#ifndef _NC_OUTPUT_INC_
#define _NC_OUTPUT_INC_

#include <string>
#include <toolsa/Path.hh>
#include <Ncxx/Nc3File.hh>
#include "Nexrad2Netcdf.hh"
#include "SweepData.hh"
#include "RangeTable.hh"

class NcOutput 
{
public:

   //
   // Constructor
   //  tdrpParams = reference to tdrp parameters
   //
   NcOutput( Params *tdrpParams );

   //
   // Destructor
   //
   ~NcOutput();
   
   //
   // Initialize
   //   See Status.hh for meaning of return value
   //
   status_t init();

  // 
  // Set basic radar info, from either params or input data
  //
  void setRadarInfo(char *radarName, double radarHeight, double radarLat, double radarLon,
		    int radarID = -1, char *siteName = NULL);

   //
   // Set the volume start time
   //   startTime = time at the start of the volume
   //
   void setVolStartTime( int startTime );

   //
   // Set the base time for the sweep
   //   startTime = time at the start of the sweep
   //   msPastMidnight = milliseconds past midnight of current message
   //
   void setBaseTime( int startTime, int msPastMidnight );

   //
   // Write the file
   //   currentScan = container object for data fields related
   //   to the current scan
   //
   //   See Status.hh for meaning of return value
   //
   status_t writeFile( SweepData* currentScan );

   const char *getFileName() { return fileName.c_str(); }

   //
   // Constants associated with radar
   //
   static const float  RADAR_CONSTANT;
   static const float  RECEIVER_GAIN;
   static const float  ANTENNA_GAIN;
   static const float  SYSTEM_GAIN;
   static const float  WAVELENGTH;
   static const float  XMIT_PEAK_POWER;

   //
   // Constants associated with file naming
   //
   static const int    MAX_FILE_NAME_LEN;
   static const string DIR_DELIM;

   //
   // Constants associated with netCDF output
   //
   static const int    SHORT_STR_LEN;
   
   static const string AZIMUTH_NAME;
   static const string ELEV_NAME;
   static const string TIME_NAME;

   static const string DZ_NAME;
   static const string VE_NAME;
   static const string SW_NAME;
   static const string ZDR_NAME;
   static const string PHI_NAME;
   static const string RHO_NAME;
   static const string CMAP_NAME;
   static const string BMAP_NAME;
   static const string SNR_NAME;
   static const string PR_NAME;
   static const string REC_NAME;

   static const string DZ_UNITS;
   static const string VE_UNITS;
   static const string SW_UNITS;
   static const string ZDR_UNITS;
   static const string PHI_UNITS;
   static const string RHO_UNITS;
   static const string SNR_UNITS;
   static const string PR_UNITS;
   static const string REC_UNITS;

   //
   // Other constants
   //
   static const float KM_TO_M;
   
private:
   
   //
   // Reference to parameters
   //
   Params *params;
   
   //
   // Used to create a file
   //
   string  outputPath;
   Path    filePath;
   Nc3File *ncFile;

   //
   // Used to name the file
   //
   int    millisecsPastMidnight;
   string radarName;
   int    volumeNum;
   int    sweepNum;
   string fileName;
  char _radarName[5];
  double _radarHeight;
  double _radarLat;
  double _radarLon;
  char _siteName[255];
  int _radarID;

   //
   // List of field names that will be
   // written to output file -- this is
   // set up once
   //
   char *fieldNames;

   //
   // Time information for file as a whole
   //
   int volumeStartTime;
   int baseTime;
   
   //
   // Single value data for output file
   //
   float rangeToFirstCell;
   float nyquistVelocity;
   float unambiguousRange;
   float prf;

  int _numCellsVel;
  int _numCellsRefl;
  int _numRays;

  //
  // One dimensional arrays
  //
  float  *_azimuth;
  float  *_elevation;
  double *_dataTime;

  float *_hNoise;
  float *_vNoise;
   
  //
  // Two dimensional arrays
  //
  ui08 *_dz;
  ui08 *_ve;
  ui08 *_sw;
  ui08 *_zdr;
  si16 *_phi;
  ui08 *_rho;
  ui08 *_snr;
  ui08 *_pr;
  ui08 *_rec;

  bool _haveDz;
  bool _haveVe;
  bool _haveSw;
  bool _haveSnr;
  bool _havePr;
  bool _haveRec;
  bool _haveZdr;
  bool _havePhi;
  bool _haveRho;

   //
   // Range cutoff table
   //
   RangeTable rangeTable;

   //
   // Create the file
   //   fixedAngle = target elevation for this tilt
   //
   //   Note that the file is simply created here, not written or closed
   //   See Status.hh for meaning of return value
   //
   status_t createFile( float fixedAngle );

   //
   // Delete the file
   // Called only on error to delete a partially created file.
  //
   void deleteCurrentFileName();

   //
   // Set the file name
   //   fixedAngle = target elevation for this tilt
   //
   //   See Status.hh for meaning of return value
   //
   status_t setFileName( float fixedAngle );

   //
   // Set the constant and single value variables in the
   // current file
   //   currentSweep = SweepData object for data we are
   //                  writing
   //
   //   See Status.hh for meaning of return value
   //
   status_t setFileVals( SweepData* currentSweep ); 

   //
   // Add global attributes to the file
   //   vcp    = volume coverage pattern id
   //   merged = flag to tell us if this data originally
   //            consisted of two sweeps that were merged
   //
   void addGlobalAtt( int vcp, bool merged );

   //
   // Overloaded addVar functions.  
   //   All of these functions add data to the variable
   //   called varName in the current netcdf file
   //
   //     varName = name of variable
   //     value   = single value to add to the variable
   //     values  = array of values to add to the variable
   //     c0      = number of values in the first dimension
   //     c1      = number of values in the second dimension
   status_t addVar( const char* varName, int value );
   status_t addVar( const char* varName, float value );
   status_t addVar( const char* varName, double value );
   status_t addVar( const char* varName, float* values, long c0 );
   status_t addVar( const char* varName, double* values, long c0 );

   //
   // Overloaded addNewVar functions
   //
   //   varName  = name of variable
   //   dim1     = netcdf NcDim pointer for first dimension
   //   dim2     = netcdf NcDim pointer for second dimension
   //   longName = long name for variable
   //   units    = string containing units for variable
   //   scale    = scale value for variable
   //   bias     = bias value for variable
   //   badValue = value used for bad or missing data
   //   values   = array of values to add to the variable
   //   c0       = number of values in the first dimension
   //   c1       = number of values in the second dimension
   status_t addNewVar( const char* varName, Nc3Dim* dim1,
                             Nc3Dim* dim2, char* values, 
                             long c0, long c1 );
   status_t addNewVar( const char* varName, Nc3Dim* dim1,
                             Nc3Dim* dim2, const char* longName,
                             const char* units, double scale,
                             double bias, ui08 badValue,
		             float rangeToFirst, float cellSpacing, 
                             ui08* values, long c0, long c1 );
   status_t addNewVar( const char* varName, Nc3Dim* dim1,
                             Nc3Dim* dim2, const char* longName,
                             const char* units, double scale,
                             double bias, si16 badValue,
		             float rangeToFirst, float cellSpacing, 
                             si16* values, long c0, long c1 );
   status_t addNewVar( const char* varName, Nc3Dim* dim1,
		             Nc3Dim* dim2, ui08* values, 
			     long c0, long c1 );
   status_t addBypassVar( const char* varName, Nc3Dim* dim1,
		             Nc3Dim* dim2, short* values, 
		             long c0, long c1 ); 
   status_t addClutterVar( const char* varName, Nc3Dim* dim1,
		             Nc3Dim* dim2, Nc3Dim* dim3, short* values, 
		             long c0, long c1, long c2 ); 
  //
  // Assemble the beams from the current sweep into one
  // dimensional arrays of variables
  // Fills arrays dz,ve,sw,snr,pr,rec
  void assemble( SweepData& currentSweep);

   //
   // Clear out all the data arrays, etc.
   //
   void clear();
   
};

#endif
   
   
   









