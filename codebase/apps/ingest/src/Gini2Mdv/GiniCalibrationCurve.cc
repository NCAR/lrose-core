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
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Class:       GiniCalibrationCurve                                          //
//                                                                            //
// Author:      Steve Mueller                                                 //
//                                                                            //
// Date:        June 2006                                                     //
//                                                                            //
// Description: Class representing Gini (satellite) calibration curves.       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// C++ Standard Include Files
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

// Local Include Files
#include "Params.hh"
#include "GiniCalibrationCurve.hh"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
//                              Main Constructor                             //
///////////////////////////////////////////////////////////////////////////////
GiniCalibrationCurve::GiniCalibrationCurve(Params::calibration_t calibrationParams)
   {
   _calibrationDataStruct = buildCalibrationDataStruct(calibrationParams);

   setCalibrationCurve(_calibrationDataStruct);
   } // End of GiniCalibrationCurve constructor.


///////////////////////////////////////////////////////////////////////////////
//                             Default Destructor                            //
///////////////////////////////////////////////////////////////////////////////
GiniCalibrationCurve::~GiniCalibrationCurve()
   {
   } // End of GiniCalibrationCurve destructor.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  GiniCalibrationCurve::buildCalibrationDataStruct()                        //
//                                                                            //
// Description:                                                               //
//  Public method that converts a Params::calibration_t structure to a        //
//  GiniCalibrationCurve::calibration_data_t structure. The comma-delimited   //
//  strings in the parameter file are separated into vectors of integers and  //
//  floats in this method.                                                    //
//                                                                            //
// Input:                                                                     //
//  A Params::calibration_t structure.                                        //
//                                                                            //
// Output:                                                                    //
//  A GiniCalibrationCurve::calibration_data_t structure.                     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
GiniCalibrationCurve::calibration_data_t GiniCalibrationCurve::buildCalibrationDataStruct(Params::calibration_t calibrationParams)
   {
   const string methodName = "GiniCalibrationCurve::buildCalibrationDataStruct()";

   string value;
   //   int leftDelimiter;
   //   int rightDelimiter;
   size_t leftDelimiter;
   size_t rightDelimiter;
   int badInputValueInt;
   int xValueInt;
   float offsetValueFloat;
   float linearValueFloat;
   float quadraticValueFloat;
   calibration_data_t calibrationDataStructOutput;

   calibrationDataStructOutput.name                    = calibrationParams.name;
   calibrationDataStructOutput.units                   = calibrationParams.units;
   calibrationDataStructOutput.badOrMissingOutputValue = calibrationParams.bad_or_missing_output_value;
   calibrationDataStructOutput.minThreshold            = calibrationParams.min_threshold;
   calibrationDataStructOutput.maxThreshold            = calibrationParams.max_threshold;

   // Parse bad input values
   string badInputValuesStr = calibrationParams.bad_input_values;
   leftDelimiter = 0;
   // First check for single value (easier to parse than comma-delimited multiple values).
   if(badInputValuesStr.find(',') == string::npos)
      {
      badInputValueInt = atoi(badInputValuesStr.c_str());
      calibrationDataStructOutput.badInputValuesVec.push_back(badInputValueInt);
      }
   else
      {
      while((rightDelimiter = badInputValuesStr.find(',', leftDelimiter)) != string::npos)
         {
         value = badInputValuesStr.substr(leftDelimiter, rightDelimiter);
         badInputValueInt = atoi(value.c_str());
         calibrationDataStructOutput.badInputValuesVec.push_back(badInputValueInt);
         leftDelimiter = rightDelimiter+1;
         }
      // Need to get last element (which isn't followed by a comma)
      value = badInputValuesStr.substr(leftDelimiter, badInputValuesStr.size());
      badInputValueInt = atoi(value.c_str());
      calibrationDataStructOutput.badInputValuesVec.push_back(badInputValueInt);
      }

   // Parse x values
   string xValuesStr = calibrationParams.x_values;
   leftDelimiter = 0;
   // First check for "one-piece quadratic" (i.e., only one x Value with no commas).
   //  These are much easier to parse.
   if(xValuesStr.find(',') == string::npos)
      {
      xValueInt = atoi(xValuesStr.c_str());
      calibrationDataStructOutput.xValuesVec.push_back(xValueInt);
      }
   else
      {
      while((rightDelimiter = xValuesStr.find(',', leftDelimiter)) != string::npos)
         {
         value = xValuesStr.substr(leftDelimiter, rightDelimiter);
         xValueInt = atoi(value.c_str());
         calibrationDataStructOutput.xValuesVec.push_back(xValueInt);
         leftDelimiter = rightDelimiter+1;
         }
      // Need to get last element (which isn't followed by a comma)
      value = xValuesStr.substr(leftDelimiter, xValuesStr.size());
      xValueInt = atoi(value.c_str());
      calibrationDataStructOutput.xValuesVec.push_back(xValueInt);
      }

   // Parse offset coefficients
   string offsetCoeffsStr = calibrationParams.offset_coeffs;
   leftDelimiter = 0;
   // First check for "one-piece quadratic" (these are much easier to parse).
   if(offsetCoeffsStr.find(',') == string::npos)
      {
      offsetValueFloat = atof(offsetCoeffsStr.c_str());
      calibrationDataStructOutput.offsetVec.push_back(offsetValueFloat);
      }
   else
      {
      while((rightDelimiter = offsetCoeffsStr.find(',', leftDelimiter)) != string::npos)
         {
         value = offsetCoeffsStr.substr(leftDelimiter, rightDelimiter);
         offsetValueFloat = atof(value.c_str());
         calibrationDataStructOutput.offsetVec.push_back(offsetValueFloat);
         leftDelimiter = rightDelimiter+1;
         }
      // Need to get last element (which isn't followed by a comma)
      value = offsetCoeffsStr.substr(leftDelimiter, offsetCoeffsStr.size());
      offsetValueFloat = atof(value.c_str());
      calibrationDataStructOutput.offsetVec.push_back(offsetValueFloat);
      }

   // Parse linear coefficients
   string linearCoeffsStr = calibrationParams.linear_coeffs;
   leftDelimiter = 0;
   // First check for "one-piece quadratic" (these are much easier to parse).
   if(linearCoeffsStr.find(',') == string::npos)
      {
      linearValueFloat = atof(linearCoeffsStr.c_str());
      calibrationDataStructOutput.linearVec.push_back(linearValueFloat);
      }
   else
      {
      while((rightDelimiter = linearCoeffsStr.find(',', leftDelimiter)) != string::npos)
         {
         string value = linearCoeffsStr.substr(leftDelimiter, rightDelimiter);
         linearValueFloat = atof(value.c_str());
         calibrationDataStructOutput.linearVec.push_back(linearValueFloat);
         leftDelimiter = rightDelimiter+1;
         }
      // Need to get last element (which isn't followed by a comma)
      value = linearCoeffsStr.substr(leftDelimiter, linearCoeffsStr.size());
      linearValueFloat = atof(value.c_str());
      calibrationDataStructOutput.linearVec.push_back(linearValueFloat);
      }

   // Parse quadratic coefficients
   string quadraticCoeffsStr = calibrationParams.quadratic_coeffs;
   leftDelimiter = 0;
   // First check for "one-piece quadratic" (these are much easier to parse).
   if(quadraticCoeffsStr.find(',') == string::npos)
      {
      quadraticValueFloat = atof(quadraticCoeffsStr.c_str());
      calibrationDataStructOutput.quadraticVec.push_back(quadraticValueFloat);
      }
   else
      {
      while((rightDelimiter = quadraticCoeffsStr.find(',', leftDelimiter)) != string::npos)
         {
         string value = quadraticCoeffsStr.substr(leftDelimiter, rightDelimiter);
         quadraticValueFloat = atof(value.c_str());
         calibrationDataStructOutput.quadraticVec.push_back(quadraticValueFloat);

         leftDelimiter = rightDelimiter+1;
         }
      // Need to get last element (which isn't followed by a comma)
      value = quadraticCoeffsStr.substr(leftDelimiter, quadraticCoeffsStr.size());
      quadraticValueFloat= atof(value.c_str());
      calibrationDataStructOutput.quadraticVec.push_back(quadraticValueFloat);
      }

   return calibrationDataStructOutput;
   } // End of buildCalibrationDataStruct() method.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  GiniCalibrationCurve::setCalibrationCurve()                               //
//                                                                            //
// Description:                                                               //
//  Creates a calibration curve using the information in the input structure  //
//  GiniCalibrationCurve::calibration_data_t. The independent variables are   //
//  always integers from 0-255.                                               //
//                                                                            //
// Input:                                                                     //
//  A GiniCalibrationCurve::calibration_data_t structure.                     //
//                                                                            //
// Output:                                                                    //
//  True for success, false otherwise.                                        //
//  Assigns the _calibrationCurve private data member.                        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
bool GiniCalibrationCurve::setCalibrationCurve(calibration_data_t calibrationDataStruct)
   {
   const string methodName = "GiniCalibrationCurve::setCalibrationCurve()";

   vector<int>::const_iterator xValueVecIterator;
   int xValueIndex;
   float offset;
   float linear;
   float quadratic;

   vector<int>   xValueVec    = calibrationDataStruct.xValuesVec;
   vector<float> offsetVec    = calibrationDataStruct.offsetVec;
   vector<float> linearVec    = calibrationDataStruct.linearVec;
   vector<float> quadraticVec = calibrationDataStruct.quadraticVec;

   size_t nbrDataPts = xValueVec.size();
   if(nbrDataPts != offsetVec.size() || nbrDataPts != linearVec.size() || nbrDataPts != quadraticVec.size())
      {
      cerr << "ERROR: " << methodName << endl;
      cerr << "   Calibration set for " << getName() << " does not have equal number of elements for each term" << endl;
      return false;
      }

   for(int index=0; index<256; index++)
      {
      // Determine appropriate piecewise linear coefficients.
      xValueIndex = 0;
      for(xValueVecIterator=xValueVec.begin(); xValueVecIterator!=xValueVec.end(); xValueVecIterator++)
         {
         // Compare index to each element of xValueVec. Increment xValueIndex for each value of xValueVec
	 //  that is less than index.
         if(index > *xValueVecIterator)
            {
            xValueIndex++;
            }
         else
            {
            break;
            }
         }

      offset    = offsetVec[xValueIndex];
      linear    = linearVec[xValueIndex];
      quadratic = quadraticVec[xValueIndex];

      float indexFloat = (float) index;
      _calibrationCurve[index] = offset + linear*indexFloat + quadratic*(indexFloat*indexFloat);
      }

   // Override bad input data values with output flag (badOrMissingOutputValue).
   vector<int>::const_iterator badInputValuesVecIterator;
   for(badInputValuesVecIterator=calibrationDataStruct.badInputValuesVec.begin(); badInputValuesVecIterator!=calibrationDataStruct.badInputValuesVec.end(); badInputValuesVecIterator++)
      {
      _calibrationCurve[*badInputValuesVecIterator] = calibrationDataStruct.badOrMissingOutputValue;
      }

      return true;
   } // End of setCalibrationCurve() method.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  GiniCalibrationCurve::printCalibrationCurve()                             //
//                                                                            //
// Description:                                                               //
//  Public method to print the calibration curve of the current instance.     //
//                                                                            //
// Input: None                                                                //
//                                                                            //
// Output:                                                                    //
//  Screen dump.                                                              //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void GiniCalibrationCurve::printCalibrationCurve(string logFile)
   {
   const string methodName = "GiniCalibrationCurve::printCalibrationCurve()";

   ofstream calibrationLogFile;
   calibrationLogFile.open(logFile.c_str(), ios::out | ios::app);

   for(int index=0; index<256; index++)
      {
      calibrationLogFile << index << " " << _calibrationCurve[index] << endl;
      }

   calibrationLogFile.close();
   } // End of printCalibrationCurve() method.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  GiniCalibrationCurve::printCalibrationDataStruct()                        //
//                                                                            //
// Description:                                                               //
//  Public method to print the calibration data of the current instance.      //
//                                                                            //
// Input:                                                                     //
//  GiniCalibrationCurve::calibration_data_t structure.                       //
//                                                                            //
// Output:                                                                    //
//  Dumped to calibration log file specified in the parameter file.           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void GiniCalibrationCurve::printCalibrationDataStruct(calibration_data_t calibrationDataStruct, string logFile)
   {
   const string methodName = "GiniCalibrationCurve::printCalibrationDataStruct()";

   ofstream calibrationLogFile;
   calibrationLogFile.open(logFile.c_str(), ios::out | ios::app);

   calibrationLogFile << endl << endl;
   calibrationLogFile << "name: " << calibrationDataStruct.name << endl;
   calibrationLogFile << "units: " << calibrationDataStruct.units << endl;
   calibrationLogFile << "badOrMissingOutputValue: " << calibrationDataStruct.badOrMissingOutputValue << endl;
   calibrationLogFile << "minThreshold: " << calibrationDataStruct.minThreshold << endl;
   calibrationLogFile << "maxThreshold: " << calibrationDataStruct.maxThreshold << endl;

   vector<int>::const_iterator badInputValuesVecIterator;
   calibrationLogFile << "badInputValuesVec: ";
   for(badInputValuesVecIterator=calibrationDataStruct.badInputValuesVec.begin(); badInputValuesVecIterator!=calibrationDataStruct.badInputValuesVec.end(); badInputValuesVecIterator++)
      {
      calibrationLogFile << *badInputValuesVecIterator << " ";
      }
   calibrationLogFile << endl;

   vector<int>::const_iterator xValuesVecIterator;
   calibrationLogFile << "xValuesVec: ";
   for(xValuesVecIterator=calibrationDataStruct.xValuesVec.begin(); xValuesVecIterator!=calibrationDataStruct.xValuesVec.end(); xValuesVecIterator++)
      {
      calibrationLogFile << *xValuesVecIterator << " ";
      }
   calibrationLogFile << endl;

   vector<float>::const_iterator offsetVecIterator;
   calibrationLogFile << "offsetVec: ";
   for(offsetVecIterator=calibrationDataStruct.offsetVec.begin(); offsetVecIterator!=calibrationDataStruct.offsetVec.end(); offsetVecIterator++)
      {
      calibrationLogFile << *offsetVecIterator << " ";
      }
   calibrationLogFile << endl;

   vector<float>::const_iterator linearVecIterator;
   calibrationLogFile << "linearVec: ";
   for(linearVecIterator=calibrationDataStruct.linearVec.begin(); linearVecIterator!=calibrationDataStruct.linearVec.end(); linearVecIterator++)
      {
      calibrationLogFile << *linearVecIterator << " ";
      }
   calibrationLogFile << endl;

   vector<float>::const_iterator quadraticVecIterator;
   calibrationLogFile << "quadraticVec: ";
   for(quadraticVecIterator=calibrationDataStruct.quadraticVec.begin(); quadraticVecIterator!=calibrationDataStruct.quadraticVec.end(); quadraticVecIterator++)
      {
      calibrationLogFile << *quadraticVecIterator << " ";
      }
   calibrationLogFile << endl;

   calibrationLogFile.close();
   } // End of printCalibrationDataStruct() method.
