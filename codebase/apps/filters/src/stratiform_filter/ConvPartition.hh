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
/************************************************************************
 * ConvPartition.hh : header file for the ConvPartition class.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1999
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef ConvPartition_HH
#define ConvPartition_HH

/*
 **************************** includes **********************************
 */

#include <vector>

#include <stdio.h>

#include <dataport/port_types.h>
#include <euclid/CircularTemplate.hh>
#include <euclid/CircularTemplateList.hh>
#include <euclid/SimpleGrid.hh>
#include <Mdv/MdvxField.hh>
#include <rapmath/PtFunction.hh>


/*
 ******************************* defines ********************************
 */

/*
 ******************************* structures *****************************
 */

/*
 ************************* global variables *****************************
 */

/*
 ***************************** function prototypes **********************
 */

/*
 ************************* class definitions ****************************
 */

class ConvPartition
{
 public:

  //////////////////
  // Public types //
  //////////////////

  typedef enum
  {
    MISSING_DATA,
    STRATIFORM,
    CONVECTIVE
  } data_type_t;
  

  ///////////////////////////////////////
  // Public constructors & destructors //
  ///////////////////////////////////////

  // Constructor

  ConvPartition(PtFunction *value_diff_function,
		PtFunction *conv_radius_function,
		double conv_center_min,
		double background_min_dbz,
		double background_radius,
	        const bool data_increasing = true,
		const bool debug = false);
  
  // Destructor

  ~ConvPartition();
  

  /////////////////
  // Run methods //
  /////////////////

  // Run the algorithm.

  void run(MdvxField &field);
  

  ////////////////////
  // Access methods //
  ////////////////////

  ui08 *getPartition()
  {
    return _partition.getGrid();
  }
  
  fl32 *getMeanData()
  {
    return _meanData.getGrid();
  }
  
  fl32 *getExcess()
  {
    return _excess.getGrid();
  }
  
  fl32 *getThreshold()
  {
    return _threshold.getGrid();
  }
  

 private:

  /////////////////////
  // Private members //
  /////////////////////

  bool _debug;

  bool _dataIncreasing;

  // Background template

  CircularTemplate *_backgroundTemplate;
  double _backgroundMinDbz;
  
  // Convective center minimum used for criteria 1

  double _convCenterMin;
  
  // Functions used in algorithm

  PtFunction *_valueDiffFunc;
  PtFunction *_convRadiusFunc;
  
  // Internal data grids

  SimpleGrid<fl32> _meanData;
  SimpleGrid<ui08> _partition;

  SimpleGrid<fl32> _excess;
  SimpleGrid<fl32> _threshold;
  
  // List of circular templates for all of the radii in the
  // _convRadiusFunction

  CircularTemplateList _convRadiusTemplates;
  

  /////////////////////
  // Private methods //
  /////////////////////

  // Allocate the internal data grids

  void _allocateDataGrids(int nx, int ny);
  
  // Calculate the internal data grids needed in the algorithm calculations.

  void _calculateDataGrids(const SimpleGrid<fl32> &grid,
			   const fl32 missing_data_value,
			   const fl32 bad_data_value);
  
  // Calculate the data mean at the given location using the given template

  static double _calculateDataMean(const SimpleGrid<fl32> &data,
				   const fl32 missing_data_value,
				   const fl32 bad_data_value,
                                   double background_min_dbz,
				   CircularTemplate *templ,
				   int x, int y);
  
  // Generate the convective partition of the data.

  void _generatePartition(const SimpleGrid<fl32> &data_grid,
			  const fl32 missing_data_value,
			  const fl32 bad_data_value);
  
  // Return the class name for error messages.

  static const char *_className()
  {
    return("ConvPartition");
  }
  
};


#endif
