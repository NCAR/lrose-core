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

// C++ Standard Include Files
#include <cstdio>
#include <cstdlib> 
#include <cstring>  

// RAP Include Files
#include <dataport/port_types.h>
#include <toolsa/pmu.h>

// Local Include Files
#include "ProcessGiniFile.hh"
#include "Params.hh"
#include "GiniCalibrationCurve.hh"
#include "GiniPDB.hh"
#include "Gini2MdvUtilities.hh"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                                 CONSTRUCTOR                                //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
ProcessGiniFile::ProcessGiniFile(FILE *fp, string inputFileNameWithPath, string shortFieldName, GiniCalibrationCurve *calibrationObj, Params *params)
   {
   _inputFilePtr = fp;
   _giniInputFileNameWithPath = inputFileNameWithPath;
   _shortFieldName = shortFieldName;
   _calibrationObj = calibrationObj;
   _params = params;

   _calibratedDataBuffer = 0;

   if(!_giniInputFileNameWithPath.empty())
      {
      setInputFileName(); // Removes path from _giniInputFileNameWithPath and assigns the
                          //  remaining string to the private data member _giniInputFileName
      }
   } // End of ProcessGiniFile constructor


ProcessGiniFile::~ProcessGiniFile()
   {
   delete [] _calibratedDataBuffer;
   delete _decodedProductDefinitionBlock;
   _inputFilePtr = 0;
   _calibrationObj = 0;
   _params = 0;

   } // End of ProcessGiniFile Destructor.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  ProcessGiniFile::readGiniFile()                                           //
//                                                                            //
// Description:                                                               //
//  Public method to read the data parameters from the input file and assign  //
//  the data to private class data members.                                   //
//                                                                            //
// Input: None (Access to class data members).                                //
//                                                                            //
// Output:                                                                    //
//  Returns true for success, false otherwise.                                //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
bool ProcessGiniFile::readGiniFile()
   {
   const string methodName = "ProcessGiniFile::readGiniFile()";

   // Check for valid file pointer.
   if(0 == _inputFilePtr) // 0 is NULL
      {
      return false;
      }

   ////////////////////
   // Read Gini data //
   ////////////////////
   // Product identification label.
   fread(&_productIdentification, sizeof(char), 1, _inputFilePtr);

   // Product data type.
   fread(&_dataType, sizeof(char), 1, _inputFilePtr);

   // Satellite type.
   fread(&_satelliteType, sizeof(char), 1, _inputFilePtr);

   // Area-subtype designator.
   fread(&_areaSubtype, sizeof(char), 1, _inputFilePtr);

   // Field number.
   char fieldStr[3];
   fread(fieldStr, sizeof(char), 2, _inputFilePtr);
   fieldStr[2]=(char)0;
   _fieldNumber = atoi(fieldStr);

   // Originating station.
   fread(_origin, sizeof(char), 5, _inputFilePtr);
   _origin[5]=(char)0;

   ///////////////////////////////////////
   //
   // This 10 byte date string is not in the 
   // documentation that I have - Niles.
   //
   fread(_dateStr, sizeof(unsigned char), 10, _inputFilePtr);
   _dateStr[10]=(char)0;

   // Product definition block.
   fread(_productDefinitionBlock, sizeof(unsigned char), 512, _inputFilePtr);

   /////////////////////////////////////////
   // Decode the product definition block //
   /////////////////////////////////////////
   _decodedProductDefinitionBlock = new GiniPDB();
   if(0 == _decodedProductDefinitionBlock) // 0 is NULL
      {
      cerr << "ERROR: " << methodName << endl;
      cerr << "   Could not instantiate GiniPDB object (_decodedProductDefinitionBlock)" << endl;
      return false;
      }
   _decodedProductDefinitionBlock->decodeGiniPDB(_productDefinitionBlock);
  
   _giniDataSize = _decodedProductDefinitionBlock->getNumRecords()*_decodedProductDefinitionBlock->getSizeRecords();

   unsigned char *rawDataBuffer = new unsigned char[_giniDataSize];
   if(0 == rawDataBuffer) // 0 is NULL
      {
      cerr << "ERROR: " << methodName << endl;
      cerr << "   Could not allocate memory for raw data buffer" << endl;
      return false;
      }

   int p=fread(rawDataBuffer, sizeof(unsigned char), _giniDataSize, _inputFilePtr);

   if (p != _giniDataSize)
      {
      cerr << "ERROR: " << methodName << endl;
      cerr << "   Read error: " << p << " vs " << _giniDataSize << endl;
      if (!_params->process_partial_files)
	      {
		      cerr << "   Giving up on this file/time." << endl;
		      return false;
	      }
      }

   unsigned char uc;

   fread(&uc, sizeof(unsigned char), 1, _inputFilePtr);
   if ((int)uc != 255)
      {
      fprintf(stderr,"\nDANGER! Last char was not correct. val = %d\n", (int)uc);
      }

   //
   // Check the trailer - this seems to alternate between 0 and 255.
   // Do this in debug mode only. Also print the total number of bytes read.
   //
   if (_params->debug)
      {
      int OK = 1;
      unsigned char alternating = 0;
      while(!(feof(_inputFilePtr)))
         {
         int n=fread(&uc, sizeof(unsigned char), 1, _inputFilePtr);
         if (1==n)
            {
            if (uc != alternating)
               {
               OK = 0;
               }
            if (0 == alternating)
               {
               alternating = 255;
               }
            else
               {
               alternating = 0;
               }
            }
         }

      if (OK)
         {
         fprintf(stderr,"Trailing bytes check passed.\n");
         }
      else
         {
         fprintf(stderr,"WARNING : Trailing bytes check failed.\n");
         }
      //      long bytesRead = ftell(_inputFilePtr);
      //cerr << bytesRead << " bytes read from file " << inFileName << endl;
      }

   // Calibrate the raw data.
   calibrateGiniData(rawDataBuffer);

   delete [] rawDataBuffer;

   return true;
   } // End of readGiniFile() method.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  ProcessGiniFile::fillGiniData()                                           //
//                                                                            //
// Description:                                                               //
//  Public method to create a set of Gini data consisting entirely of missing //
//  data values. Used when an input file is missing and Gini2Mdv is allowed   //
//  to continue in the absence of a non-critical data set.                    //
//                                                                            //
// Input: None (Access to class data members).                                //
//                                                                            //
// Output:                                                                    //
//  Returns true for success, false otherwise.                                //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
bool ProcessGiniFile::fillGiniData(string dataType, string sector)
   {
   const string methodName = "ProcessGiniFile::fillGiniData()";

   _decodedProductDefinitionBlock = new GiniPDB();
   if(0 == _decodedProductDefinitionBlock) // 0 is NULL
      {
      cerr << "ERROR: " << methodName << endl;
      cerr << "Could not instantiate GiniPDB object" << endl;
      exit(-1);
      }

  _decodedProductDefinitionBlock->setSource("NESDIS");

   if("EastCONUS" == sector)
      {
     _decodedProductDefinitionBlock->setCreator("GOES-12");
     _decodedProductDefinitionBlock->setSector(sector);
      if(("VIS" == gini2MdvUtilities::str2UpperCase(dataType)) || ("VISIBLE" == gini2MdvUtilities::str2UpperCase(dataType)))
         {
	 //
	 // Values taken from 'normal' EAST-CONUS 1km, visible file 
	 //
         _decodedProductDefinitionBlock->setChannel("Visible");
         _decodedProductDefinitionBlock->setNumRecords(5120);
         _decodedProductDefinitionBlock->setSizeRecords(5120);
         // Set time to "now"
	 time_t currentTime = time(0);
	 struct tm *currentTimePtr = gmtime(&currentTime);
         _decodedProductDefinitionBlock->setYear(currentTimePtr->tm_year);
         _decodedProductDefinitionBlock->setMonth(currentTimePtr->tm_mon+1);
         _decodedProductDefinitionBlock->setDay(currentTimePtr->tm_mday);
         _decodedProductDefinitionBlock->setHour(currentTimePtr->tm_hour);
         _decodedProductDefinitionBlock->setMin(currentTimePtr->tm_min);
         _decodedProductDefinitionBlock->setSec(currentTimePtr->tm_sec);
         _decodedProductDefinitionBlock->setHSec(0);
         _decodedProductDefinitionBlock->setProjection(3);
         _decodedProductDefinitionBlock->setNx(5120);
         _decodedProductDefinitionBlock->setNy(5120);
         _decodedProductDefinitionBlock->setLat1(16.3691);
         _decodedProductDefinitionBlock->setLon1(-113.13330000000001);
         _decodedProductDefinitionBlock->setLat2(-9999);
         _decodedProductDefinitionBlock->setLon2(-9999);
         _decodedProductDefinitionBlock->setLov(-95);
         _decodedProductDefinitionBlock->setDx(1015.9);
         _decodedProductDefinitionBlock->setDy(1015.9);
         _decodedProductDefinitionBlock->setDi(-9999);
         _decodedProductDefinitionBlock->setDj(-9999);
         _decodedProductDefinitionBlock->setResFlag(7);
         _decodedProductDefinitionBlock->setSouthPole(0);
         _decodedProductDefinitionBlock->setScanLeftToRight(true);
         _decodedProductDefinitionBlock->setScanTopToBottom(true);
         _decodedProductDefinitionBlock->setScanXInFirst(true);
         _decodedProductDefinitionBlock->setLatin(25);
         _decodedProductDefinitionBlock->setImageRes(1);
         _decodedProductDefinitionBlock->setCompression(0);
         _decodedProductDefinitionBlock->setCreatorVersion(1);
         _decodedProductDefinitionBlock->setPDBSize(512);
         _decodedProductDefinitionBlock->setNavIncluded(false);
         _decodedProductDefinitionBlock->setCalIncluded(false);
	 }
      else
         {
	 cerr << "ERROR: " << methodName << endl;
	 cerr << "   Invalid data type" << endl;
	 cerr << "   Options are 'visible'" << endl << endl;
	 cerr << "   EXITING" << endl;
	 exit(-1);
	 }
      }
   else if("WestCONUS" == sector)
      {
     _decodedProductDefinitionBlock->setCreator("GOES-11");
     _decodedProductDefinitionBlock->setSector(sector);
      if(("VIS" == gini2MdvUtilities::str2UpperCase(dataType)) || ("VISIBLE" == gini2MdvUtilities::str2UpperCase(dataType)))
         {
	 //
	 // Values taken from 'normal' WEST-CONUS 1km, visible file 
	 //
         _decodedProductDefinitionBlock->setChannel("Visible");
         _decodedProductDefinitionBlock->setNumRecords(5120);
         _decodedProductDefinitionBlock->setSizeRecords(4400);
	 // Set time to "now"
	 time_t currentTime = time(0);
	 struct tm *currentTimePtr = gmtime(&currentTime);
         _decodedProductDefinitionBlock->setYear(currentTimePtr->tm_year);
         _decodedProductDefinitionBlock->setMonth(currentTimePtr->tm_mon+1);
         _decodedProductDefinitionBlock->setDay(currentTimePtr->tm_mday);
         _decodedProductDefinitionBlock->setHour(currentTimePtr->tm_hour);
         _decodedProductDefinitionBlock->setMin(currentTimePtr->tm_min);
         _decodedProductDefinitionBlock->setSec(currentTimePtr->tm_sec);
         _decodedProductDefinitionBlock->setHSec(0);
         _decodedProductDefinitionBlock->setProjection(3);
         _decodedProductDefinitionBlock->setNx(5120);
         _decodedProductDefinitionBlock->setNy(4400);
         _decodedProductDefinitionBlock->setLat1(12.19);
         _decodedProductDefinitionBlock->setLon1(-133.4588);
         _decodedProductDefinitionBlock->setLat2(-9999);
         _decodedProductDefinitionBlock->setLon2(-9999);
         _decodedProductDefinitionBlock->setLov(-95);
         _decodedProductDefinitionBlock->setDx(1015.9);
         _decodedProductDefinitionBlock->setDy(1015.9);
         _decodedProductDefinitionBlock->setDi(-9999);
         _decodedProductDefinitionBlock->setDj(-9999);
         _decodedProductDefinitionBlock->setResFlag(7); // This value is from east file (west file was 66918).
         _decodedProductDefinitionBlock->setSouthPole(0);
         _decodedProductDefinitionBlock->setScanLeftToRight(true);
         _decodedProductDefinitionBlock->setScanTopToBottom(true);
         _decodedProductDefinitionBlock->setScanXInFirst(true);
         _decodedProductDefinitionBlock->setLatin(25);
         _decodedProductDefinitionBlock->setImageRes(1);
         _decodedProductDefinitionBlock->setCompression(0);
         _decodedProductDefinitionBlock->setCreatorVersion(1);
         _decodedProductDefinitionBlock->setPDBSize(512);
         _decodedProductDefinitionBlock->setNavIncluded(false);
         _decodedProductDefinitionBlock->setCalIncluded(false);
	 }
      else
         {
	 cerr << "ERROR: " << methodName << endl;
	 cerr << "   Invalid data type" << endl;
	 cerr << "   Options are 'visible'" << endl << endl;
	 cerr << "   EXITING" << endl;
	 exit(-1);
	 }
      }
   else
      {
      cerr << "ERROR: " << methodName << endl;
      cerr << "   Invalid sector designator for data fill" << endl;
      cerr << "   Options are 'EastCONUS' and 'WestCONUS'" << endl << endl;
      cerr << "   EXITING" << endl;
      exit(-1);
      }


   _giniDataSize =_decodedProductDefinitionBlock->getNumRecords()*_decodedProductDefinitionBlock->getSizeRecords();

   _calibratedDataBuffer = new fl32[_giniDataSize];
   if (0 == _calibratedDataBuffer) // 0 is NULL
      {
      cerr << "ERROR: " << methodName << endl;
      cerr << "   memory allocation failure for _calibratedDataBuffer" << endl;
      return false;
      }

   for(int calibratedDataIndex=0; calibratedDataIndex < _giniDataSize; calibratedDataIndex++)
      {
      _calibratedDataBuffer[calibratedDataIndex] = _calibrationObj->getBadDataValue();
      }

   return true;
   } // End of fillGiniData() method.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  ProcessGiniFile::calibrateGiniData()                                      //
//                                                                            //
// Description:                                                               //
//  Public method to apply the calibration curve to the input data.           //
//                                                                            //
// Input:                                                                     //
//  rawDataBuffer - (unsigned char *) array of raw data values.               //
//                                                                            //
// Output:                                                                    //
//  Returns true for success, false otherwise.                                //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
bool ProcessGiniFile::calibrateGiniData(unsigned char *rawDataBuffer)
   {
   const string methodName = "ProcessGiniFile::calibrateGiniData()";

   float *calibrationCurve = _calibrationObj->getCalibrationCurve();

   float minThreshold = _calibrationObj->getMinThreshold();
   float maxThreshold = _calibrationObj->getMaxThreshold();

   // Apply calibration data to raw data buffer to generate
   // ProcessGiniFile::_calibratedDataBuffer. Keep track track of the min, max and
   // percent good - useful for debugging purposes.

   _calibratedDataBuffer = new fl32[_giniDataSize];
   if (0 == _calibratedDataBuffer) // 0 is NULL
      {
      cerr << "ERROR: " << methodName << endl;
      cerr << "   memory allocation failure" << endl;
      return false;
      }

   double min=0.0, max=0.0, total=0.0;
   int first = 1;
   int NumGood = 0;

   for(int k=0; k < _giniDataSize; k++)
      {
      _calibratedDataBuffer[k] = calibrationCurve[rawDataBuffer[k]];

      // No bounds checks if minThreshold >= maxThreshold.
      if(minThreshold < maxThreshold)
         {
         if(_calibratedDataBuffer[k] < minThreshold)
            {
            //_calibratedDataBuffer[k] = minThreshold;
            _calibratedDataBuffer[k] = _calibrationObj->getBadDataValue();
            }
         if(_calibratedDataBuffer[k] > maxThreshold)
            {
            //_calibratedDataBuffer[k] = maxThreshold;
            _calibratedDataBuffer[k] = _calibrationObj->getBadDataValue();
            }
         }

      if(first)
         {
         min = _calibratedDataBuffer[k];
         max = min;
         total = min;
         NumGood = 1;
         first = 0;
         }
      else
         {
         NumGood++;
         if(_calibratedDataBuffer[k] < min)
            {
            min = _calibratedDataBuffer[k];
            }

         if(_calibratedDataBuffer[k] > max)
            {
            max = _calibratedDataBuffer[k];
            }

         total = total + _calibratedDataBuffer[k];
         }
      }

   double mean;

   if(0 == NumGood)
      {
      mean = 0.0;
      }
   else
      {
      mean = total / double(NumGood);
      }

   //
   // Print min, max and percent good, if debug requested.
   //
   double pg = 100.0*NumGood/double(_giniDataSize);

   if (_params->debug)
      {
      cerr << "Data range from " << min << " to " << max << " with mean " << mean << " (" << pg << " percent good)." << endl;
      }

   //
   // Force check in with PMU as we are about to get busy
   //
   PMU_force_register("Processing data");

   return true;
   } // End of calibrateGiniData() method.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  ProcessGiniFile::setInputFileName()                                       //
//                                                                            //
// Description:                                                               //
//  Public method to remove the path from _giniInputFileNameWithPath and      //
//  assign the file name to the private data member _giniInputFileName.       //
//                                                                            //
// Input: None (Access to class data members).                                //
//                                                                            //
// Output: None                                                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void ProcessGiniFile::setInputFileName()
   {
   const string methodName = "ProcessGiniFile::setInputFileName()";

   // Extract the file name from the string that includes the path.
   string inputPathStr = _giniInputFileNameWithPath;
   size_t delimiterPosition = inputPathStr.rfind('/');
   _giniInputFileName = inputPathStr.substr(delimiterPosition+1);
   } // End of setInputFileName() method.
