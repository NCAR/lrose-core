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
 *   $Date: 2016/03/06 23:15:37 $
 *   $Id: ShearForLeadingEdge.hh,v 1.3 2016/03/06 23:15:37 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * ShearForLeadingEdge: ShearForLeadingEdge program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 2005
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef ShearForLeadingEdge_HH
#define ShearForLeadingEdge_HH

#include <string>
#include <vector>

#include <dsdata/DsTrigger.hh>
#include <euclid/GridPoint.hh>
#include <Mdv/MdvxField.hh>
#include <rapformats/Bdry.hh>
#include <rapformats/BdryPoint.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;


class ShearForLeadingEdge
{
 public:

  ////////////////////
  // Public members //
  ////////////////////

  // Flag indicating whether the program status is currently okay.

  bool okay;


  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /*********************************************************************
   * Destructor
   */

  ~ShearForLeadingEdge(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static ShearForLeadingEdge *Inst(int argc, char **argv);
  static ShearForLeadingEdge *Inst();
  

  /*********************************************************************
   * init() - Initialize the local data.
   *
   * Returns true if the initialization was successful, false otherwise.
   */

  bool init();
  

  /////////////////////
  // Running methods //
  /////////////////////

  /*********************************************************************
   * run() - run the program.
   */

  void run();
  

 private:

  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static ShearForLeadingEdge *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // Triggering object

  DsTrigger *_dataTrigger;
  
  // Leading edge dataset

  vector< Bdry > _boundaries;
  
  // Model data fields

  MdvxField *_uField;
  MdvxField *_vField;
  MdvxField *_capeField;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  ShearForLeadingEdge(int argc, char **argv);
  

  /*********************************************************************
   * _calcCapeMeans() - Calculate the horizontally averaged CAPE values
   *                    for the points in the template.
   */

  void _calcCapeMeans(const vector< GridPoint > &point_list,
		      const int point_x, const int point_y,
		      vector< double > &cape_means) const;
  

  /*********************************************************************
   * _calcWindComponents() - Calculate the average wind components for
   *                         each vertical level within the vertical
   *                         shear layer.
   */

  void _calcInflows(const int kmin_layer, const int kmax_layer,
		    const double point_u, const double point_v,
		    const int point_x, const int point_y,
		    const vector< GridPoint > &point_list,
		    vector< double > &inflows) const;
  

  /*********************************************************************
   * _clearBoundaries() - Clear out the current boundaries
   */

  void _clearBoundaries();
  

  /*********************************************************************
   * _findVerticalShearLayer() - Find the vertical shear layer.
   *
   * If the vertical shear layer isn't found, returns -1 for kmin_layer
   * and kamx_layer.
   */

  void _findVerticalShearLayer(const vector< double > cape_means,
			       int &kmin_layer,
			       int &kmax_layer,
			       double &zbar_cape) const;
  

  /*********************************************************************
   * _getPointTemplate() - Get the point list for the given point.
   *
   * Returns the calculated point template.
   */

  vector< GridPoint > _getPointList(const double point_lat,
				    const double point_lon,
				    const double point_u,
				    const double point_v) const;
  

  /*********************************************************************
   * _initTrigger() - Initialize the data trigger.
   *
   * Returns true on success, false on failure.
   */

  bool _initTrigger(void);
  

  /*********************************************************************
   * _processData() - Process data for the given trigger time.
   *
   * Returns true on success, false on failure.
   */

  bool _processData(const DateTime &trigger_time);
  

  /*********************************************************************
   * _processPoint() - Calculate the following values for the given point:
   *                     - number of horizontal points going with the leading
   *                       edge point (num_pts)
   *                     - CAPE value in shear layer (zbar_cape)
   *                     - maximum shear in the layer (max_shear)
   *                     - mean shear in the layer (mean_shear)
   *                     - bottom level of shear layer (kmin)
   *                     - top level of shear layer (kmax)
   *
   * Returns true on success, false on failure.
   */

  bool _processPoint(BdryPoint &point) const;
  

  /*********************************************************************
   * _processPoints() - Process all of the points in all of the boundaries.
   *
   * Returns true on success, false on failure.
   */

  bool _processPoints();
  

  /*********************************************************************
   * _projectionsMatch() - Compare the projections of the two fields.
   *
   * Returns true if the projections match, false otherwise.
   */

  static bool _projectionsMatch(const MdvxField &field1,
				const MdvxField &field2);
  

  /*********************************************************************
   * _readLeadingEdgeData() - Read in the leading edge data.
   *
   * Returns true on success, false on failure.
   */

  bool _readLeadingEdgeData(const DateTime &trigger_time);
  

  /*********************************************************************
   * _readMdvData() - Read in the MDV data.
   *
   * Returns true on success, false on failure.
   */

  bool _readMdvData(const DateTime &trigger_time);
  

  /*********************************************************************
   * _readMdvField() - Read the indicated field from an MDV file.
   *
   * Returns a pointer to the read field on success, 0 on failure.
   */

  MdvxField *_readMdvField(const string &url,
			   const string &field_name,
			   const int field_num,
			   const DateTime &trigger_time) const;
  

};


#endif
