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
/*********************************************************************
 * GridTemplateList.cc: class implementing a list of grid templates.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <vector>

#include <math.h>
#include <cstdio>

#include <euclid/GridTemplate.hh>
#include <euclid/GridTemplateList.hh>
using namespace std;

/**********************************************************************
 * Constructor
 */

GridTemplateList::GridTemplateList(void)
{
}


/**********************************************************************
 * Destructor
 */

GridTemplateList::~GridTemplateList(void)
{
  // Reclaim the space for the template list

  while (_templateList.size() > 0)
  {
    delete *_templateList.rbegin();
    _templateList.pop_back();
  }
  
}
  

/**********************************************************************
 * addTemplate() - Add the given template to the list.
 */

void GridTemplateList::addTemplate(GridTemplate *templ)
{
  // Add the template to the list

  _templateList.push_back(templ);
  
  return;
}
  

/**********************************************************************
 * getFirstTemplate() - Retrieve the first template from the list.  Returns
 *                      NULL if there are no templates in the list.
 */

GridTemplate *GridTemplateList::getFirstTemplate(void)
{
  _templateListIterator = _templateList.begin();
  
  if (_templateListIterator == _templateList.end())
    return (GridTemplate *)NULL;
  
  return *_templateListIterator;
}
  

/**********************************************************************
 * getNextTemplate() - Retrieve the next template from the list.  Returns
 *                     NULL if there are no more templates in the list.
 *
 * Note that getFirstTemplate() MUST be called before using this method.
 */

GridTemplate *GridTemplateList::getNextTemplate(void)
{
  _templateListIterator++;
  
  if (_templateListIterator == _templateList.end())
    return (GridTemplate *)NULL;
  
  return *_templateListIterator;
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

