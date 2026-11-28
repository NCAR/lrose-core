/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
#ifndef HEADERATTRIBUTE_H
#define HEADERATTRIBUTE_H

#include <string>

using namespace std;

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

	File:		HeaderAttribute
	
	Date:		January 2006
	
	Author:		Carrie Langston (CIMMS/NSSL)
		
	Purpose:	Stores attribute information as it would be
	            found in a WDSS-II NetCDF file header
																
	_____________________________________________________________			
	Modification History:
       

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

class HeaderAttribute
{
  public:
    static string unit_str;
    static string val_str;
  
    string name;
    string unit;
    string value;
    
    
    //default constructor  
    HeaderAttribute();
        
    //2nd and 3rd constuctor
    HeaderAttribute(string initial_name); 
    HeaderAttribute(string init_name, string init_unit, string init_value);
  
    //copy constructor
    HeaderAttribute(const HeaderAttribute& hdrA);
    
    //destructor
    ~HeaderAttribute();
    
    
    //clear
    void clear();
    
    void operator= (HeaderAttribute hdrA);
  
};
//end class HeaderAttribute

#endif

