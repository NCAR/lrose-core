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

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * ByteDataProcessor.cc : Class for processing byte data from a
 *                        Terascan file.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 2001
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <new>
#include <iostream>

#include "ByteDataProcessor.hh"

using namespace std;

/*********************************************************************
 * Constructor
 */

ByteDataProcessor::ByteDataProcessor() :
  DataProcessor()
{
  // Do nothing
}


/*********************************************************************
 * Destructor
 */

ByteDataProcessor::~ByteDataProcessor()
{
  // Do nothing
}


/*********************************************************************
 * convertData() - Convert the indicated satellite data into byte
 *                 format using the given array.
 */

bool ByteDataProcessor::convertData(VARP sat_var,
				    const int first_line,
				    const int first_sample,
				    const int num_lines,
				    const int num_samples,
				    float *output_data)
{
  // Setup arrays needed for the Terascan call

  int num_elements = num_lines * num_samples;
  
  //cout << "in ByteDataProcessor" << endl;
  int start[2];
  int count[2];
  
  start[0] = first_line;
  start[1] = first_sample;
  
  count[0] = num_lines;
  count[1] = num_samples;
  
  char *sat_data_ptr;
  if(!(sat_data_ptr = new(nothrow) char[num_elements])) {
    cerr << "new failed allocating " << sizeof(char)*num_elements << " bytes." << endl;
    return false;
  }

  // Retrieve the raw satellite data from the file

  if (gpgetvar(sat_var, start, count, sat_data_ptr) < 0)
    return false;

  for (int i = 0; i < num_elements; ++i)
  {

    // bad data value 
    if ((int) ((unsigned char) sat_data_ptr[i]) == sat_var->badval)
    {
	output_data[i] = 999999.;
        //cout << "Bad value" << endl;
    }
    else
        output_data[i] = 
          (float) (((unsigned char) sat_data_ptr[i] * sat_var->scale) + sat_var->offset);


  } /* endfor - i */
  
  return true;
}

