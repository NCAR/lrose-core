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
#ifndef _ProcessGiniFile_
#define _ProcessGiniFile_

// C++ Standard Include Files
#include <cstdio>

// No RAP Include Files

// Local Include Files
#include "Params.hh"
#include "GiniPDB.hh"

// Forward Declarations
class GiniCalibrationCurve;

using namespace std;

// Class Declaration
class ProcessGiniFile
   {
   public:
      ////////////////////////////
      // NO PUBLIC DATA MEMBERS //
      ////////////////////////////

      ////////////////////
      // PUBLIC METHODS //
      ////////////////////

      // Constructors and Destructors
      ProcessGiniFile(FILE *inputFilePtr, string inputFileNameWithPath, string shortFieldName, GiniCalibrationCurve *calibrationCurve, Params *params);
      virtual ~ProcessGiniFile();

      // Set Methods
      void setInputFileNameWithPath(string inputFileNameWithPath) { _giniInputFileNameWithPath = inputFileNameWithPath; }
      void setInputFileName(); // Uses _giniInputFileNameWithPath to determine and set file name.
      void setCalibratedDataBuffer(fl32 *calibratedDataBuffer) { _calibratedDataBuffer = calibratedDataBuffer; }

      // Get Methods
      string   getGiniInputFileName()             { return _giniInputFileName; }
      fl32*    getCalibratedDataBuffer()          { return _calibratedDataBuffer; }
      GiniPDB* getDecodedProductDefinitionBlock() { return _decodedProductDefinitionBlock; }
      int      getNx()                            { return _decodedProductDefinitionBlock->getNx(); }
      int      getNy()                            { return _decodedProductDefinitionBlock->getNy(); }
      bool     getScanTopToBottom()               { return _decodedProductDefinitionBlock->getScanTopToBottom(); }
      bool     getScanLeftToRight()               { return _decodedProductDefinitionBlock->getScanLeftToRight(); }
      bool     getScanXInFirst()                  { return _decodedProductDefinitionBlock->getScanXInFirst(); }

      // General Methods
      bool readGiniFile();
      bool calibrateGiniData(unsigned char *rawDataBuffer);
      bool fillGiniData(string dataType, string sector);

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
      FILE *_inputFilePtr;
      string _shortFieldName;
      GiniCalibrationCurve *_calibrationObj;
      Params *_params;

      // Raw data from file
      string _giniInputFileName;
      string _giniInputFileNameWithPath;
      char _productIdentification;
      char _dataType;
      char _satelliteType;
      char _areaSubtype;
      int  _fieldNumber;
      char _origin[6];
      char _dateStr[11];
      unsigned  char _productDefinitionBlock[512];
      int  _giniDataSize;

      // Processed data from file
      GiniPDB *_decodedProductDefinitionBlock;
      fl32 *_calibratedDataBuffer;

      ////////////////////////
      // NO PRIVATE METHODS //
      ////////////////////////
   }; // End of ProcessGiniFile class
#endif // _ProcessGiniFile_
