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
 *   $Date: 2016/03/04 02:22:12 $
 *   $Id: AsciiTablePlotter.hh,v 1.4 2016/03/04 02:22:12 dixon Exp $
 *   $Revision: 1.4 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * AsciiTablePlotter: Class that creates scatter plots as ASCII tables
 *                    that can be read into other programs for creating
 *                    the plots.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2007
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef AsciiTablePlotter_H
#define AsciiTablePlotter_H

#include "Plotter.hh"

using namespace std;


class AsciiTablePlotter : public Plotter
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  AsciiTablePlotter(const string &delimiter = ",",
		    const bool include_header = false,
		    const string &output_dir = ".",
		    const string &output_ext = "txt",
		    const string &x_field_name = "x",
		    const string &y_field_name = "y",
		    const bool debug_flag = false);
  
  /*********************************************************************
   * Destructor
   */

  virtual ~AsciiTablePlotter();


  //////////////////////////
  // Input/output methods //
  //////////////////////////

  /*********************************************************************
   * createPlot() - Create the scatter plot for the given fields.
   *
   * Returns true on success, false on failure.
   */

  virtual bool createPlot(const DateTime &data_time,
			  const MdvxField &x_field,
			  const MdvxField &y_field);


  virtual bool createPlot(const string &output_path,
			  const DateTime &data_time,
			  const MdvxField &x_field,
			  const MdvxField &y_field);
  

protected:
  
  ///////////////////////
  // Protected members //
  ///////////////////////

  string _delimiter;
  bool _includeHeader;

};

#endif
