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
 *
 * @file GridMenu.hh
 *
 * @class GridMenu
 *
 * Class representing a menu of grid fields.
 *  
 * @date 1/14/2011
 *
 */

#ifndef GridMenu_HH
#define GridMenu_HH

#include <iostream>
#include <string>
#include <vector>

using namespace std;


/** 
 * @class GridMenu
 */

class GridMenu
{
 public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor
   */

  GridMenu();
  
  /**
   * @brief Initialize the object.
   */

  bool init(const int menu_num, const string &label,
	    const string &grid_list,
	    const bool replace_underscores);
  
  /**
   * @brief Destructor
   */

  virtual ~GridMenu();
  

  ////////////////////
  // Public members //
  ////////////////////

  /**
   * @brief Menu name.
   */

  string menuName;
  
  /**
   * @brief Menu label.
   */

  string menuLabel;
  
  /**
   * @brief List of grids in the menu.
   */

  vector< string > gridList;
  
  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Print the object to the given stream.
   */

  virtual void print(ostream &out) const;
  

protected:

  ///////////////////////
  // Protected methods //
  ///////////////////////

  void _addGrid(const string &grid_name,
		vector< string > &grid_list,
		const bool replace_underscores) const;
  

  bool _pullGridsFromList(const string &grid_list_str,
			  vector< string > &grid_list,
			  const bool replace_underscores) const;
  

};


#endif
