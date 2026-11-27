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
 *   $Date: 2016/03/04 02:29:42 $
 *   $Id: IconDef.hh,v 1.3 2016/03/04 02:29:42 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * IconDef.hh : IconDef methods.  This class represents a stroked
 *              icon definition for a Symprod object.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 2000
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef IconDef_HH
#define IconDef_HH

#include <string>
#include <vector>

#include <euclid/GridPoint.hh>
#include <Spdb/Symprod.hh>

using namespace std;


class IconDef
{      
public:

  // Constructors

  IconDef();

  IconDef(const string &icon_name,
	  vector< GridPoint > &icon_points);

  // Destructors

  ~IconDef();


  ////////////////////
  // Access methods //
  ////////////////////

  const vector< Symprod::ppt_t > &getPointList() const
  {
    return _points;
  }
  

  /**
   * @brief Draw the icon in the given Symprod buffer.
   *
   * @param[in] color_to_use Color to use.
   * @param[in] line_width Line width to use.
   * @param[in] center_lat Center latitude of icon.
   * @param[in] center_lon Center longitude of icon.
   * @param[in] icon_scale Icon scale.
   * @param[in] allow_client_scaling Flag indicating whether to allow client
   *                                 scaling.
   * @param[in,out] symprod Symprod buffer.
   */

  void draw(const string &color_to_use,
	    const int line_width,
	    const double center_lat,
	    const double center_lon,
	    const double icon_scale,
	    const bool allow_client_scaling,
	    Symprod *symprod) const;
  

private:

  // Constants

  static const int INPUT_PENUP;
  

  // Members

  string _iconName;
  vector< Symprod::ppt_t > _points;
  

};

# endif
