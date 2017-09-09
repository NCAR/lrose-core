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
 * CircularTemplateList.cc: class implementing a list of circular
 *                          templates.
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

#include <euclid/CircularTemplate.hh>
#include <euclid/CircularTemplateList.hh>
#include <euclid/GridTemplateList.hh>
using namespace std;

/**********************************************************************
 * Constructor
 */

CircularTemplateList::CircularTemplateList(void) :
  GridTemplateList()
{
}


/**********************************************************************
 * Destructor
 */

CircularTemplateList::~CircularTemplateList(void)
{
}
  

/**********************************************************************
 * addTemplate() - Add a template with the given radius to the list.
 */

void CircularTemplateList::addTemplate(double radius)
{
  CircularTemplate *templ;
  
  // First make sure there's not already a template with this radius
  // in the list.

  if ((templ = getTemplate(radius)) != (CircularTemplate *)NULL)
    return;
  
  // Create the template

  templ = new CircularTemplate(radius);
  
  // Add the template to the list

  _templateList.push_back(templ);
  
  return;
}
  

/**********************************************************************
 * getTemplate() - Retrieve the indicated template from the list.
 */

CircularTemplate *CircularTemplateList::getTemplate(double radius)
{
  CircularTemplate *templ;
  
  vector< GridTemplate* >::iterator templ_iterator;
  
  for (templ_iterator = _templateList.begin();
       templ_iterator != _templateList.end();
       templ_iterator++)
  {
    templ = (CircularTemplate *)*templ_iterator;
    
    if (templ->getRadius() == radius)
      return templ;
  }
  
  return (CircularTemplate *)NULL;
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

