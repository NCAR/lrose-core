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
 * @file GuiConfigParams.cc
 *
 * @class GuiConfigParams
 *
 * Class controlling access to the TERRAIN section of the CIDD parameter
 * file.
 *  
 * @date 2/14/2011
 *
 */

#include "Cgui_P.hh"
#include "GuiConfigParams.hh"

using namespace std;

/**********************************************************************
 * Constructor
 */

GuiConfigParams::GuiConfigParams () :
  TdrpParamSection()
{
}


/**********************************************************************
 * Destructor
 */

GuiConfigParams::~GuiConfigParams(void)
{
}
  

/**********************************************************************
 * getMenus()
 */

vector< string > GuiConfigParams::getMenus(const string &grid_name) const
{
  vector< string > menus;
  
  // Look through all of the menus for this grid

  vector< GridMenu >::const_iterator menu;
  
  for (menu = _menuList.begin(); menu != _menuList.end(); ++menu)
  {
    vector< string >::const_iterator grid;
    
    for (grid = menu->gridList.begin(); grid != menu->gridList.end(); ++grid)
    {
      // CIDD menus contain any grid whose label matches the menu grid string
      // up to the length of the menu grid string

      if (grid_name.compare(0, grid->length(), *grid) == 0)
	menus.push_back(menu->menuName);
      
    } /* endfor - grid */
  } /* endfor - menu */
  
  return menus;
}


/**********************************************************************
 * init()
 */

bool GuiConfigParams::init(const MainParams &main_params,
			   const char *params_buf, const size_t buf_size)
{
  static const string method_name = "GuiConfigParams::init()";
  
  // Pull out the GUI_CONFIG section of the parameters buffer

  const char *param_text;
  long param_text_line_no = 0;
  long param_text_len = 0;
  
  if ((param_text = _findTagText(params_buf, "GUI_CONFIG",
				 &param_text_len, &param_text_line_no)) == 0 ||
      param_text == 0 || param_text_len <= 0)
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "Couldn't find GUI_CONFIG section in CIDD parameter file" << endl;
    cerr << "Not processing GUI_CONFIG parameters" << endl;
    
    return true;
  }
  
  // Load the parameters from the buffer

  Cgui_P gui_params;
  
  if (gui_params.loadFromBuf("GUI_CONFIG TDRP Section",
			     0, param_text, param_text_len, param_text_line_no,
			     false, false) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error loading TDRP parameters from <GUI_CONFIG> section" << endl;
    
    return false;
  }
  
  // Extract the GUI config information

  for (int i = 0; i < gui_params.field_list_n; ++i)
  {
    GridMenu grid_menu;
    
    grid_menu.init(i + 1, gui_params._field_list[i].id_label,
		   gui_params._field_list[i].grid_list,
		   main_params.isReplaceUnderscores());
    
    _menuList.push_back(grid_menu);
    
  } /* endfor -- i */
  
  return true;
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
