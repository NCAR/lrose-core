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
 * StatCalc: Base class for statistic calculators.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2004
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <Mdv/MdvxPjg.hh>
#include <Mdv/climo/StatCalc.hh>

using namespace std;


/**********************************************************************
 * Constructor
 */

StatCalc::StatCalc(const bool debug_flag,
		   const bool check_z_levels) :
  _debug(debug_flag),
  _checkZLevels(check_z_levels)
{
}


/**********************************************************************
 * Destructor
 */

StatCalc::~StatCalc(void)
{
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _fieldsMatch() - Check to see if the given fields match based on their
 *                  headers.
 *
 * Returns true if the fields match, false otherwise.
 */

bool StatCalc::_fieldsMatch(const MdvxField &data_field,
			    const MdvxField &climo_field) const
{
  static const string method_name = "StatCalc::_fieldsMatch()";
  
  Mdvx::field_header_t data_field_hdr = data_field.getFieldHeader();
  Mdvx::field_header_t climo_field_hdr = climo_field.getFieldHeader();
  
  // Check the projections to make sure they match.  If we are not
  // checking the actual Z level values (_checkZLevels is false), then
  // reset the climo proj minz  and dz values to the data proj minz and
  // dz values so that the check won't fail based on these.

  MdvxPjg data_proj(data_field_hdr);
  MdvxPjg climo_proj(climo_field_hdr);
  
  if (!_checkZLevels)
  {
    climo_proj.setGridMins(climo_proj.getMinx(), climo_proj.getMiny(),
			   data_proj.getMinz());
    climo_proj.setGridDeltas(climo_proj.getDx(), climo_proj.getDy(),
			     data_proj.getDz());
  }
  
  if (!(data_proj == climo_proj))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Field projections don't match:" << endl;
    cerr << "data field: " << data_field_hdr.field_name_long << endl;
    data_proj.print(cerr);
    cerr << "climo field: " << climo_field_hdr.field_name_long << endl;
    climo_proj.print(cerr);
    
    return false;
  }
  
  // Check the vertical level types to make sure that they match

  if (data_field_hdr.vlevel_type != climo_field_hdr.vlevel_type)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Field projections don't match:" << endl;
    cerr << "data field: " << data_field_hdr.field_name_long << endl;
    cerr << "   vert_type = "
	 << Mdvx::vertType2Str(data_field_hdr.vlevel_type) << endl;
    cerr << "climo field: " << climo_field_hdr.field_name_long << endl;
    cerr << "   vert_type = "
	 << Mdvx::vertType2Str(climo_field_hdr.vlevel_type) << endl;
      
    return false;
  }
    
  // Only check the vertical levels if they aren't constant

  if (_checkZLevels &&
      (!data_field_hdr.dz_constant || !climo_field_hdr.dz_constant))
  {
    Mdvx::vlevel_header_t data_vlevel_hdr = data_field.getVlevelHeader();
    Mdvx::vlevel_header_t climo_vlevel_hdr = climo_field.getVlevelHeader();

    for (int z = 0; z < data_field_hdr.nz; ++z)
    {
      // Make sure all of the v levels match.  I'm assuming that if the type is set
      // to 0, then it really wasn't set at all and the check should be ignored.

      if (data_vlevel_hdr.type[z] != 0 &&
	  climo_vlevel_hdr.type[z] != 0 &&
	  (data_vlevel_hdr.type[z] != climo_vlevel_hdr.type[z] ||
	   data_vlevel_hdr.level[z] != climo_vlevel_hdr.level[z]))
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Field vertical levels don't match:" << endl;
	cerr << "data field: " << data_field_hdr.field_name_long << endl;
	cerr << "   type[" << z << "] = "
	     << Mdvx::vertType2Str(data_vlevel_hdr.type[z]) << endl;
	cerr << "   level[" << z << "] = " << data_vlevel_hdr.level[z] << endl;
	cerr << "climo field: " << climo_field_hdr.field_name_long << endl;
	cerr << "   type[" << z << "] = "
	     << Mdvx::vertType2Str(climo_vlevel_hdr.type[z]) << endl;
	cerr << "   level[" << z << "] = " << climo_vlevel_hdr.level[z] << endl;
	
	return false;
      }
    } /* endfor - z */
  }
  
  return true;
}
