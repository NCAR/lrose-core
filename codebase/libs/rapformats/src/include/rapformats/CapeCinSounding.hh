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
// CapeCinSounding.hh
//
/////////////////////////////////////////////////////////////

#ifndef CAPECINSOUNDING_H
#define CAPECINSOUNDING_H

#include <string>
#include <physics/PhysicsLib.hh> 
#include <physics/thermo.h>
#include <rapformats/Sndg.hh>

class CapeCinSounding {
  
public:
  
  //
  // Vertical reference variable
  //
  enum var_t {
    PRESSURE = 0,
    ALTITUDE = 1 }; 
                 
  //
  // Constructors
  //
  CapeCinSounding(const bool debug_flag = false);

  CapeCinSounding(bool debug_flag, Sndg *sndgPtr, 
		  CapeCinSounding::var_t verticalVariable,
		  float minCalcLevel, float maxCalcLevel, bool resample,
		  float altIncr, bool surfAve, float surfLB, 
		  float surfUB, string temp_lookuptable);
  //
  // destructor
  //
  ~CapeCinSounding();

  //
  // Set the debug flag
  //
  void setDebugFlag(const bool debug_flag)
  {
    debug = debug_flag;
  }
  
  //
  // Set the adiabat temperature lookup table
  //
  bool setTempLookupTable(const string &temp_lookup_filename)
  {
    static const string method_name = "CapeCinSounding::setTempLookupTable()";
    
    lookup_table = new AdiabatTempLookupTable(temp_lookup_filename.c_str());
    
    if (lookup_table == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Problem creating adiabatic temp lookup table. Check file path.\n" << endl;
      
      isOK = false;
      return false;
    }
    
    return true;
  }
  
  //
  // Set the vertical calculation bounds.
  //
  void setCalcBounds(const var_t vertical_variable,
		     const float min_calc_level, const float max_calc_level)
  {
    verticalLevelVariable = vertical_variable;
    minVarLevel = min_calc_level;
    maxVarLevel = max_calc_level;
  }
  
  //
  // Set resampling.
  //
  void setResampling(const double altitude_incr)
  {
    resampleData = true;
    altIncrement = altitude_incr;
  }
  
  //
  // Set surface layer averaging
  //
  void setSurfaceLayerAveraging(const double lower_bound,
				const double upper_bound)
  {
    surfaceLayerAveraging = true;
    surfaceLayerLowerBound = lower_bound;
    surfaceLayerUpperBound = upper_bound;
  }
  
  //
  // Set the sounding to use.
  //
  void setSounding(const Sndg *sndg_ptr)
  {
    extractSoundingData(sndg_ptr);
  }
  
  //
  //  Calculate cape and cin with the following input surface obs.
  //  Write output to output_url
  //
  int surfaceObsCapeCin(float p, float t, float a, float q);

   //
  // callCalcCapeCin3D:  Execute CalcCapeCin3D, Write output to a output_url.
  //
  int callCalcCapeCin3D();

  //
  // flag which indicated init process in constructor was successful.
  //
  bool isOK;

  //
  // Get methods
  //
  float* getCape() {return cape; }
  
  float* getCin() {return cin; }

  float* getOutputPressure() { return (pr + min_calc_index); }

  float* getEl() { return el; }

  float* getLfc() { return lfc; }

  float* getLcl() { return lcl; }

  float getSoundingTop() { return alt[nz-1]; }
  
  int getOutputNz() { return output_nz; } 

  Sndg& getSounding() { return newSndg; } 

protected:
  
private:

  // 
  // extractSoundingData():
  //   1) get sounding data, check for missing or bad data,
  //    derive mixing ratio for input to CalcCapeCin3D,
  //   2) Perform surface averaging if specified in param file 
  //   3) Perform data sampling if specified in param file 
  //
  void extractSoundingData(const Sndg *sndgPtr);

  //
  // computeMinMaxAltIndices(): Given the upper and lower bounds of altitude
  //                            which define the layer over which cape and cin 
  //                            are calculated, we find the corresponding sounding 
  //                            data array indices for these bounds.
  //
  void computeMinMaxAltIndices(float *alt, int nz, int &min_index, int &max_index);

  //
  // computeMinMaxAltIndices(): Given the upper and lower bounds of pressure
  //                            which define the layer over which cape and cin 
  //                            are calculated, we find the corresponding sounding
  //                            data array indices for these bounds.
  //
  void computeMinMaxPressureIndices(float *p, int nz, int &min_index, int &max_index);

  //
  // resampleArrays(): Class member arrays pr (pressure), temp (temperature),
  //                   mixingr( mixing ratio) and alt (altitude) are 
  //                   resampled by increments specified by parameter
  //                   altIncrement. The units of this increment are 
  //                   the same as parameter verticalLevelVariable( which is either in 
  //                   mb for pressure levels or km for alt levels). 
  //                  
  void resampleArrays();

  //
  //
  // surfaceLayerAve(): Given the parameters surfaceLayerLowerBound and
  //                    surfaceLayerUpperBound, we find the average of the
  //                    variables pressure, temperature, altitude, and mixing 
  //                    ratio in this layer. We determine the place
  //                    in the pressure array where the average pressure of the surface
  //                    layer falls and we keep only the data from     
  //                    this index forward and rewrite all of the arrays ( pr, temp, mixingr,
  //                    and alt) to start with the surface layer average of the variable 
  //                    followed by the rest of the data.
  //
  int surfaceLayerAve();


  //
  // Create a new sounding, using the modified (resampled or averaged) data arrays,
  //
  //
  void createNewSounding();


  //
  // Delete member arrays if necessary, reset variables
  //
  void clearAll();

  //
  // writeOutput(): Load GenPt object and to write it.
  //
  int writeOutput(int output_nz, int offset);

  //
  // Input to calcCapeCin3D()
  //
  AdiabatTempLookupTable *lookup_table;

  //
  // pressure in mb
  //
  float *pr ;
 
  //
  // temp in deg K
  // 
  float *temp ;  
  
  //
  // altitude in gpm
  //
  float *alt ;
  
  //
  // mixing ratio in g/kg
  //
  float *mixingr;

  //
  // Other sounding variables
  //
  float *rh;
  float *dewpt;
  float *time;
  float *u;
  float *v;
  float *w;
  float *tempC;
  float *windSpeed;
  float *windDir;
  float *ascensionRate;
  float *longitude;
  float *latitude;

  int nz;
 
  //
  // output of calcCapeCin3D()
  //
  float *cape;

  float *cin; 

  float *lcl;

  float *lfc;
 
  float *el;

  int output_nz;

  int min_calc_index;
  
  int max_calc_index;

  //
  // User defined parameters passed into the constructor:
  //

  // flag for debug messaging
  //
  bool debug;
 
  //
  // This is either PRESSURE or ALTITUDE. This sets the variable 
  // according to which resampling and surface layer averaging
  // will take place. It specifies the units(either m or mb) for 
  // minVarLevel, maxVarLevel,surfaceLayerUpperBound, and
  // surfaceLayerLowerBound, and for altIncrement.
  //
  var_t verticalLevelVariable;

  //
  // Lower bound of either pressure or altitude for layer in
  // which cape and cin is computed.
  // Units should be the same as verticalLevelVariable.
  // 
  float minVarLevel;

  //
  // Upper bound of either pressure or altitude for layer in
  // which cape and cin is computed.
  // Units should be the same as verticalLevelVariable.
  //
  float maxVarLevel;

  //
  // Flag to resample data
  //
  bool resampleData;

  //
  // If resampleData == TRUE then data will be resampled with this increment
  // starting either at the minVarLevel (if verticalLevelVariable
  // is ALTITUDE ) or  maxVarLevel(f verticalLevelVariable is PRESSURE ).
  // Units should be the same as verticalLevelVariable.
  // 
  float altIncrement;

  //
  // Flag to do  surface layer averaging 
  //
  bool surfaceLayerAveraging;

  // 
  // Upper bound for surface level averages.
  // Units should be the same as verticalLevelVariable.
  //
  float surfaceLayerUpperBound;

  //
  // Lower bound for surface level averages.
  // Units should be the same as verticalLevelVariable.
  //
  float surfaceLayerLowerBound;

  //
  // File path for adiabatic temperature lookup table/
  //
  string adiabat_temp_lookup_filename;

  //
  // other flags: override_least_calc_index flag is used
  // if cape and cin are to be calculated with a particular input surface 
  // observation. In this case, despite the values of minVarLevel or
  // maxVarLevel, we want to calculate cape and cin at the 
  // pressure level in the observation.
  //
  bool override_least_calc_index;
 
  //
  // constants from sounding data
  //
  static const float  CAPE_MISSING;
  
  static const float  CIN_MISSING;

  static const float BAD_DATA;
  
  static const float MISSING_SNDG_DATA;

  Sndg newSndg;

  Sndg::header_t newSndgHeader;

  vector <Sndg::point_t*> pointsVec;

};

#endif
















