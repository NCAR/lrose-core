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

#ifndef _GiniCalibrationCurve_
#define _GiniCalibrationCurve_

// C++ Standard Include Files
#include <cstdio>
#include <string>
#include <vector>

// No RAL Include Files

// Local Include Files
#include "Params.hh"

// No Forward Declarations

using namespace std;

// Class Declaration
class GiniCalibrationCurve
   {
   public:
      //////////////////////////////
      // PUBLIC TYPE DECLARATIONS //
      //////////////////////////////
      typedef struct
         {
         string name;
         string units;
         float badOrMissingOutputValue;
	 float minThreshold;
	 float maxThreshold;

	 vector<int>   badInputValuesVec;
         vector<int>   xValuesVec;
         vector<float> offsetVec;
         vector<float> linearVec;
         vector<float> quadraticVec;
         } calibration_data_t;

      ////////////////////////////
      // NO PUBLIC DATA MEMBERS //
      ////////////////////////////
 
      ////////////////////
      // PUBLIC METHODS //
      ////////////////////

      // Constructors and Destructors
      GiniCalibrationCurve(Params::calibration_t calibrationParams);
      virtual ~GiniCalibrationCurve();

      // Set Methods
      bool setCalibrationCurve(calibration_data_t calibrationDataStruct); // Assigns _calibrationCurveVec

      // Get Methods
      string getName()             { return _calibrationDataStruct.name; }
      string getUnits()            { return _calibrationDataStruct.units; }
      float  getBadDataValue()     { return _calibrationDataStruct.badOrMissingOutputValue; }
      float  getMinThreshold()     { return _calibrationDataStruct.minThreshold; }
      float  getMaxThreshold()     { return _calibrationDataStruct.maxThreshold; }
      float* getCalibrationCurve() { return _calibrationCurve; }

      // General Methods
      calibration_data_t buildCalibrationDataStruct(Params::calibration_t calibrationParams);
      void printCalibrationCurve(string logFile);
      void printCalibrationDataStruct(calibration_data_t calibrationDataStruct, string logFile);

   protected:
      ///////////////////////////////
      // NO PROTECTED DATA MEMBERS //
      ///////////////////////////////

      //////////////////////////
      // NO PROTECTED METHODS //
      //////////////////////////

   private:
      //////////////////////////
      // PRIVATE DATA MEMBERS //
      //////////////////////////
      calibration_data_t _calibrationDataStruct;
      float _calibrationCurve[256];

      ////////////////////////
      // NO PRIVATE METHODS //
      ////////////////////////
   }; // End of GiniCalibrationCurve class declaration.

#endif // _GiniCalibrationCurve_
