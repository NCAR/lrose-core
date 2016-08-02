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
 * @file GridMenu.cc
 *
 * @class GridMenu
 *
 * Class representing a menu of grid fields.
 *  
 * @date 1/14/2011
 *
 */

#include <iostream>
#include <cstdio>

#include "GridMenu.hh"

using namespace std;

/**********************************************************************
 * Constructor
 */

GridMenu::GridMenu () :
  menuName(""),
  menuLabel("")
{
}


/**********************************************************************
 * Destructor
 */

GridMenu::~GridMenu(void)
{
}
  

/**********************************************************************
 * init()
 */

bool GridMenu::init(const int menu_num, const string &label,
		    const string &grid_list, const bool replace_underscores)
{
  // Set the menu name

  char name[80];
  sprintf(name, "menu%d", menu_num);
  menuName = name;
  
  // Set the menu label

  menuLabel = label;
  
  // Set the grid list

  if (!_pullGridsFromList(grid_list, gridList, replace_underscores))
    return false;
  
  return true;
}
  

/**********************************************************************
 * print()
 */

void GridMenu::print(ostream &out) const
{
  out << "GridMenu" << endl;
  out << "--------" << endl;
  out << "name = <" << menuName << ">" << endl;
  out << "label = <" << menuLabel << ">" << endl;
  
  vector< string >::const_iterator grid_name;
  
  for (grid_name = gridList.begin(); grid_name != gridList.end(); ++grid_name)
    out << "    <" << *grid_name << ">" << endl;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _addGrid()
 */

void GridMenu::_addGrid(const string &grid_name,
			vector< string > &grid_list,
			const bool replace_underscores) const
{
  if (grid_name == "")
    return;
  
  string grid = grid_name;
  
  if (replace_underscores)
  {
    for (size_t i = 0; i < grid.length(); ++i)
    {
      if (grid[i] == '_')
	grid[i] = ' ';
    }
  }
      
  grid_list.push_back(grid);
    
}


/**********************************************************************
 * _pullGridsFromList()
 */

bool GridMenu::_pullGridsFromList(const string &grid_list_str,
				  vector< string > &grid_list,
				  const bool replace_underscores) const
{
  size_t start_pos = 0;
  size_t space_pos;
  string grid;

  while ((space_pos = grid_list_str.find(' ', start_pos)) != string::npos)
  {
    grid.assign(grid_list_str, start_pos, space_pos - start_pos);
    
    // Add the grid to the list

    _addGrid(grid, grid_list, replace_underscores);
    
    // Get ready for the next loop

    start_pos = space_pos + 1;
    space_pos = grid_list_str.find(' ', start_pos);

  } /* endwhile */
  
  // Handle the last grid

  grid.assign(grid_list_str, start_pos, space_pos - start_pos);
  _addGrid(grid, grid_list, replace_underscores);
  
  return true;
}
