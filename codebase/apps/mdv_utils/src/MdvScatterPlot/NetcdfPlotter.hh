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
 *   $Id: NetcdfPlotter.hh,v 1.3 2016/03/04 02:22:12 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * NetcdfPlotter: Class that creates scatter plots as netCDF files
 *                that can be read into other programs for creating
 *                the plots.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 2007
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef NetcdfPlotter_H
#define NetcdfPlotter_H

#include <Ncxx/Nc3File.hh>

#include "Plotter.hh"

using namespace std;


class NetcdfPlotter : public Plotter
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  NetcdfPlotter(const string &output_dir = ".",
		const string &output_ext = "txt",
		const string &x_field_name = "",
		const string &y_field_name = "",
		const bool debug_flag = false);
  
  /*********************************************************************
   * Destructor
   */

  virtual ~NetcdfPlotter();


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
  
  /////////////////////////
  // Protected constants //
  /////////////////////////

  static const string DIMENSION_NAME;
  
  static const string LAT_VAR_NAME;
  static const string LON_VAR_NAME;
  static const string TIME_VAR_NAME;

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /*********************************************************************
   * _appendData() - Append the given data to the currently existing data
   *                 for the given variable.
   *
   * Returns true on success, false on failure.
   */
  
  template< class T >
  bool _appendData(Nc3Var *variable,
		   const string &variable_name,
		   const int current_data_size,
		   const T *data,
		   const int data_size) const;
  

  /*********************************************************************
   * _getExistingFile() - Open the indicated existing netCDF file and return
   *                      a pointer to it.
   *
   * Returns a pointer to the netCDF file on success, 0 on failure.
   *
   * The caller must delete the file pointer once they are finished with the
   * file.
   */

  Nc3File *_getExistingFile(const string &output_path,
			   const string &x_field_name,
			   const string &y_field_name) const;
  

  /*********************************************************************
   * _getFile() - Open the indicated netCDF file and return a pointer to it.
   *              If the file currently exists on disk, we will open that
   *              file, check for any inconsistencies compared to what we
   *              expect to find in the file, and return the pointer.  If
   *              the file doesn't currently exist, we will create the file
   *              and all of the needed dimensions and variables.
   *
   * Returns a pointer to the netCDF file on success, 0 on failure.
   *
   * The caller must delete the file pointer once they are finished with the
   * file.
   */

  Nc3File *_getFile(const string &output_path,
		   const string &x_field_name,
		   const string &y_field_name) const;
  

  /*********************************************************************
   * _getNewFile() - Create the indicated netCDF file and return a pointer
   *                 to it.  Create the all of the needed dimensions and
   *                 variables.
   *
   * Returns a pointer to the netCDF file on success, 0 on failure.
   *
   * The caller must delete the file pointer once they are finished with the
   * file.
   */

  Nc3File *_getNewFile(const string &output_path,
		      const string &x_field_name,
		      const string &y_field_name) const;
  

};

#endif
