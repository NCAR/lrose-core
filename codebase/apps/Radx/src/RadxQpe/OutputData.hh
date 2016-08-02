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
/**
 * @file OutputData.hh
 * @brief Handles the data that is created and written out
 * @class OutputData
 * @brief Handles the data that is created and written out
 */

#ifndef OUTPUT_DATA_HH 
#define OUTPUT_DATA_HH 

#include "Data.hh"

class RadxFile;

class OutputData : public Data
{
public:

  /**
   * Empty constructor
   *
   * @param[in] params
   */
  OutputData (const Parms &params);

  /**
   * Constructor for output using lowest elevation from input
   *
   * @param[in] params
   * @param[in] input  Use the loweest elevation as a template
   */
  OutputData (const Parms &params, const Data &input);

  /**
   * Destructor
   */
  ~OutputData(void);

  /**
   *Write contents to a volume
   *
   * @param[in] t  Time
   * @param[in] vol_index  Index of volume
   */
  void writeVolume(const time_t &t, const Data &inp, int vol_index);

  RadxVol &getOutputVol(void) {return _outVol;}

protected:
private:

  std::vector<double> _inputElev; /**< elevation angles of input data */
  RadxVol _outVol;                /**< Used for output */

  void _addFieldsToRay(const Sweep &s, int iaz, RadxRay *ray);
  void _addField(const Params::output_field_t &pfield, string &name, 
		 const Field &fld, int iaz, RadxRay *ray);
  void _addRateField(const Params::rainrate_field_t &pfield, string &name,
		     const Field &fld,  int iaz, RadxRay *ray);
  void _writeVolume(const Data &inp);
  void _setupWrite(RadxFile &file);

};

#endif
