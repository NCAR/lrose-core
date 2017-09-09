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
 * GridTemplateList.hh: class implementing a list of grid templates.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1999
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef GridTemplateList_HH
#define GridTemplateList_HH

/*
 **************************** includes **********************************
 */

#include <vector>

#include <cstdio>

#include "GridTemplate.hh"

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

class GridTemplateList
{
 public:

  // Constructor

  GridTemplateList(void);
  
  // Destructor

  ~GridTemplateList(void);
  
  // Add the given template to the list.

  void addTemplate(GridTemplate *templ);
  
  // Iterate through the templates in the list.  These routines
  // return NULL if there is no template to return.  To iterate
  // through the template list, do something like the following:
  //
  //   for (GridTemplate *templ = template_list.getFirstTemplate();
  //        templ != (GridTemplate *)NULL;
  //        templ = template_list.getNextTemplate())
  //              ...

  GridTemplate *getFirstTemplate(void);
  GridTemplate *getNextTemplate(void);
  
  GridTemplate *getTemplate(const int template_index)
  {
    return _templateList[template_index];
  }
  
  // Access methods

  int size(void)
  {
    return _templateList.size();
  }
  
 protected:

  // The template list

  vector< GridTemplate* > _templateList;
  
  // The template list iterator, manipulated using getFirstTemplate()
  // and getNextTemplate()

  vector< GridTemplate* >::iterator _templateListIterator;
  
 private:

  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("GridTemplateList");
  }
  
};


#endif
