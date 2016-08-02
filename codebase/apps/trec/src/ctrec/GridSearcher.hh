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

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/06 23:28:58 $
 *   $Id: GridSearcher.hh,v 1.5 2016/03/06 23:28:58 dixon Exp $
 *   $Revision: 1.5 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * GridSearcher.hh : This class searches for the min or max value in a
 *                   SimpleGrid object or in a subgrid of such an object.
 *                   The GridSearcherTypes class just defines types used
 *                   by the GridSearcher class.  I had to add the
 *                   GridSearcherTypes class because I couldn't figure
 *                   out how to define these types within a template
 *                   class, which is what SimpleGrid is.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 2000
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef GridSearcher_HH
#define GridSearcher_HH

/*
 **************************** includes **********************************
 */

#include <vector>

#include <euclid/GridPoint.hh>

#include "SimpleGrid.hh"
using namespace std;


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


class GridSearcherTypes
{
 public:

  //////////////////
  // Public types //
  //////////////////

  typedef enum
  {
    SEARCH_START_LOWER_LEFT,
    SEARCH_START_LOWER_RIGHT,
    SEARCH_START_UPPER_LEFT,
    SEARCH_START_UPPER_RIGHT
  } search_start_t;
  
  typedef enum
  {
    SEARCH_COLUMN_FIRST,
    SEARCH_ROW_FIRST
  } search_direction_t;
  
  typedef enum
  {
    SEARCH_TAKE_FIRST,
    SEARCH_TAKE_LAST,
    SEARCH_TAKE_MIDDLE
  } search_result_t;
};


template <class T>
class GridSearcher
{
public:

  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////
  // Constructors //
  //////////////////

  GridSearcher(const GridSearcherTypes::search_start_t start,         /* I */
	       const GridSearcherTypes::search_direction_t direction, /* I */
	       const GridSearcherTypes::search_result_t result,       /* I */
	       const bool debug_flag) :                               /* I */
    _start(start),
    _direction(direction),
    _result(result),
    _debugFlag(debug_flag)
  {
    // Do nothing
  }
  
  
  ////////////////
  // Destructor //
  ////////////////

  ~GridSearcher(void)
  {
    // Do nothing
  }
  

  ////////////////////
  // Search methods //
  ////////////////////

  /////////////////////////////////////////////////////////////////
  // getMaxValue() - Find the maximum value in the grid or subgrid.
  //                 Also returns the location of the maximum value
  //                 and the number of times the maximum value
  //                 appeared in the subgrid.

  T getMaxValue(SimpleGrid<T> &grid,                /* I */
		const T bad_data_value,             /* I */
		GridPoint &max_location,            /* O */
		int &max_count) const               /* O */
  {
    getMaxValue(grid,
		0, grid.getNx() - 1,
		0, grid.getNy() - 1,
		bad_data_value,
		max_location, max_count);
  }
  


  T getMaxValue(SimpleGrid<T> &grid,               /* I */
		const int min_x, const int max_x,  /* I */
		const int min_y, const int max_y,  /* I */
		const T bad_data_value,            /* I */
		GridPoint &max_location,           /* O */
		int &max_count) const              /* O */
  {
    int min_x_search = min_x;
    int max_x_search = max_x;
    int min_y_search = min_y;
    int max_y_search = max_y;
    
    // Make sure the search range is valid

    if (min_x_search < 0)
      min_x_search = 0;
    
    if (min_y_search < 0)
      min_y_search = 0;
    
    if (max_x_search >= grid.getNx())
      max_x_search = grid.getNx() - 1;
    
    if (max_y_search >= grid.getNy())
      max_y_search = grid.getNy() - 1;
    
    // Perform the search

    switch(_direction)
    {
    case GridSearcherTypes::SEARCH_COLUMN_FIRST :
      return _getMaxValueByColumn(grid,
				  min_x_search, max_x_search,
				  min_y_search, max_y_search, bad_data_value,
				  max_location, max_count);
      
    case GridSearcherTypes::SEARCH_ROW_FIRST :
      return _getMaxValueByRow(grid,
			       min_x_search, max_x_search,
			       min_y_search, max_y_search, bad_data_value,
			       max_location, max_count);
      
    }
    
    return bad_data_value;
  }
  
 private:

  /////////////////////
  // Private members //
  /////////////////////

  // Search parameters

  GridSearcherTypes::search_start_t _start;
  GridSearcherTypes::search_direction_t _direction;
  GridSearcherTypes::search_result_t _result;
  
  // Debug flag

  bool _debugFlag;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /////////////////////////////////////////////////////////////////
  // _getmaxValueByColumn() - Get the maximum value from the subgrid,
  //                          doing a column-first search.

  T _getMaxValueByColumn(const SimpleGrid<T> &grid,           /* I */
			 const int min_x, const int max_x,    /* I */
			 const int min_y, const int max_y,    /* I */
			 const T bad_data_value,              /* I */
			 GridPoint &max_location,             /* O */
			 int &max_count) const                /* O */
  {
    // Initialize the return values

    max_location.setPoint(0, 0);
    
    // Create a vector to contain the places where the maximum value
    // occurred.

    vector<GridPoint> max_points;

    // Set up the min/max values for the loops

    int x_start, x_end, x_increment;
    int y_start, y_end, y_increment;
    
    _setLoopValues(min_x, max_x, min_y, max_y,
		   x_start, x_end, x_increment,
		   y_start, y_end, y_increment);
    
    // Find the maximum values.

    double max_value = bad_data_value;
    bool max_value_found = false;
    
    for (int x = x_start; x != x_end; x += x_increment)
    {
      for (int y = y_start; y != y_end; y += y_increment)
      {
	T data_value = grid.get(x, y);
	
	if (data_value == bad_data_value)
	  continue;
	
	if (!max_value_found ||
	    data_value > max_value)
	{
	  max_value = data_value;
	  GridPoint data_point(x, y);
	  
	  max_points.erase(max_points.begin(), max_points.end());
	  max_points.push_back(data_point);
	  
	  max_value_found = true;
	}
	else if (data_value == max_value)
	{
	  GridPoint data_point(x, y);
	  
	  max_points.push_back(data_point);
	}
	
      } /* endfor - y */
    } /* endfor - x */
    
    // Retrieve the max value count to return.

    max_count = max_points.size();
    if (max_points.size() == 0) {
      return max_value;
    }
    
    // Find the max value position to return.

    int max_index = 0;
    
    switch(_result)
    {
    case GridSearcherTypes::SEARCH_TAKE_FIRST :
      max_index = 0;
      break;
      
    case GridSearcherTypes::SEARCH_TAKE_LAST :
      max_index = max_count - 1;
      break;
      
    case GridSearcherTypes::SEARCH_TAKE_MIDDLE :
      max_index = max_count / 2;
      break;
    }
    
    max_location.setPoint(&max_points[max_index]);
    
    return max_value;
  }
  

  /////////////////////////////////////////////////////////////////
  // _getmaxValueByRow() - Get the maximum value from the subgrid,
  //                       doing a row-first search.

  T _getMaxValueByRow(const SimpleGrid<T> &grid,           /* I */
		      const int min_x, const int max_x,    /* I */
		      const int min_y, const int max_y,    /* I */
		      const T bad_data_value,              /* I */
		      GridPoint &max_location,             /* O */
		      int &max_count) const                /* O */
  {
    // Initialize the return values

    max_location.setPoint(0, 0);
    
    // Create a vector to contain the places where the maximum value
    // occurred.

    vector<GridPoint> max_points;

    // Set up the min/max values for the loops

    int x_start, x_end, x_increment;
    int y_start, y_end, y_increment;
    
    _setLoopValues(min_x, max_x, min_y, max_y,
		   x_start, x_end, x_increment,
		   y_start, y_end, y_increment);
    
    // Find the maximum values.

    double max_value = bad_data_value;
    bool max_value_found = false;
    
    for (int y = y_start; y != y_end; y += y_increment)
    {
      for (int x = x_start; x != x_end; x += x_increment)
      {
	T data_value = grid.get(x, y);
	
	if (data_value == bad_data_value)
	  continue;
	
	if (!max_value_found ||
	    data_value > max_value)
	{
	  max_value = data_value;
	  GridPoint data_point(x, y);
	  
	  max_points.erase(max_points.begin(), max_points.end());
	  max_points.push_back(data_point);
	  
	  max_value_found = true;
	}
	else if (data_value == max_value)
	{
	  GridPoint data_point(x, y);
	  
	  max_points.push_back(data_point);
	}
	
      } /* endfor - x */
    } /* endfor - y */
    
    // Retrieve the max value count to return.

    max_count = max_points.size();
    if (max_points.size() == 0) {
      return max_value;
    }
    
    // Find the max value position to return.

    int max_index = 0;
    
    switch(_result)
    {
    case GridSearcherTypes::SEARCH_TAKE_FIRST :
      max_index = 0;
      break;
      
    case GridSearcherTypes::SEARCH_TAKE_LAST :
      max_index = max_count - 1;
      break;
      
    case GridSearcherTypes::SEARCH_TAKE_MIDDLE :
      max_index = max_count / 2;
      break;
    }
    
    max_location.setPoint(&max_points[max_index]);
    
    return max_value;
  }
  

  /////////////////////////////////////////////////////////////////
  // _setLoopValues() - Set the values to use in the for loops when
  //                    searching.

  void _setLoopValues(const int min_x, const int max_x,
		      const int min_y, const int max_y,
		      int &x_start, int &x_end, int &x_increment,
		      int &y_start, int &y_end, int &y_increment) const
  {
    switch (_start)
    {
    case GridSearcherTypes::SEARCH_START_LOWER_LEFT :
      x_start = min_x;
      x_end = max_x + 1;
      x_increment = 1;
    
      y_start = min_y;
      y_end = max_y + 1;
      y_increment = 1;
    
      break;
    
    case GridSearcherTypes::SEARCH_START_LOWER_RIGHT :
      x_start = max_x;
      x_end = min_x - 1;
      x_increment = -1;
    
      y_start = min_y;
      y_end = max_y + 1;
      y_increment = 1;
    
      break;
    
    case GridSearcherTypes::SEARCH_START_UPPER_LEFT :
      x_start = min_x;
      x_end = max_x + 1;
      x_increment = 1;
    
      y_start = max_y;
      y_end = min_y - 1;
      y_increment = -1;
    
      break;
    
    case GridSearcherTypes::SEARCH_START_UPPER_RIGHT :
      x_start = max_x;
      x_end = min_x - 1;
      x_increment = -1;
    
      y_start = max_y;
      y_end = min_y - 1;
      y_increment = -1;
      
      break;
    } /* endswitch - _start */
  }
  

  /////////////////////////////////////////////////////////////////
  // _className() - Return the class name for error messages.

  static const char *_className(void)
  {
    return("GridSearcher");
  }
  
};


#endif
