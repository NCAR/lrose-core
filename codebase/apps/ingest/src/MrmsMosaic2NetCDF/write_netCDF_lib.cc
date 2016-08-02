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
#ifndef WRITE_NETCDF_LIB_H
#define WRITE_NETCDF_LIB_H

#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <cstdlib>
#include <netcdf.h>
#include <vector>

#include "HeaderAttribute.h"


using namespace std;


/*------------------------------------------------------------------

	Function:	check_err
		
	Purpose:	If an error occurs while reading or writing
	            a NetCDF file, this function can be called to 
	            give more details.  Program will exit if
	            an error occurs	
				
	Input:		stat = integer code for NetCDF error
	
	      		line = line number where error occurred
	      		
	      		file = name of file in which error occurred
				
	Output:		Text message to user.
	
------------------------------------------------------------------*/

void check_err(const int stat, const int line, const char *file) 
{
    if (stat != NC_NOERR) 
    {
	   (void) fprintf(stderr, "line %d of %s: %s\n", line, file, 
	                                              nc_strerror(stat));
       exit(1);
    }
}



/*------------------------------------------------------------------

	Function:	soft_check_err_wrt
		
	Purpose:	If an error occurs while writing a NetCDF 
	            file, this function can be called to give 
	            more details.  Unlike check_err, this
	            function will not exit the program if an 
	            error occurs, given us a chance to handle 
	            the problem	
				
	Input:		stat = integer code for NetCDF error
	
	      		line = line number where error occurred
	      		
	      		file = name of file in which error occurred
				
	Output:		Text message to user and int to indicate error
	
------------------------------------------------------------------*/

int soft_check_err_wrt(const int stat, const int line, const char *file) 
{
    if (stat != NC_NOERR) 
    {
	   (void) fprintf(stderr, "line %d of %s: %s\n", line, file, 
	                                              nc_strerror(stat));
       //exit(1);
       return -1;
    }
    
    return 1;
}



/*------------------------------------------------------------------

	Method:		  write_extra_attributes
	
	
	Purpose:	  Given a variable ID, write to a NetCDF file 
	            several variable attributes. These attributes 
	            can vary in number.  The only requirement is that 
	            they are listed under the variable attribute name 
	            "attributes" and they have associated "unit" and 
	            "value" global string attributes. 	      
	
	Input:      file_handle = a file handle for an open NetCDF file 	
	            varID = a variable ID (can be NC_GLOBAL)
	            attrs = vector of HeaderAttribute objects.
				
	Output:		  variable attributes written to NetCDF file
	            int returned to indicate success for failure
	
------------------------------------------------------------------*/
    
int write_extra_attributes(int file_handle, int varID, 
                                    vector<HeaderAttribute>& attrs)
{

    char charArray[NC_MAX_NAME];
    string attr_list;
    string tmp_str;
    int stat;
    
    //Handle case where there are no extra attributes
    if(attrs.size() == 0)
    {
      strcpy(charArray, "");
      stat = nc_put_att_text(file_handle, varID, "attributes", strlen(charArray), charArray);
      if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) return -1;    
      
      return 1;
    }
        
        
    //Generate list of attributes
    attr_list = " ";
    
    for(size_t i = 0; i < attrs.size(); i++)
    {
      attr_list = attr_list + " " + attrs[i].name;
    }
    
    
    //Write attribute list to ncdf file
    strcpy(charArray, attr_list.c_str());
    stat = nc_put_att_text(file_handle, varID, "attributes", strlen(charArray), charArray);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) return -1;


    //Write out attribute values and names
    for(size_t i = 0; i < attrs.size(); i++)
    {      
      //Write the attribute unit
      tmp_str = attrs[i].name + HeaderAttribute::unit_str;
      strcpy(charArray, attrs[i].unit.c_str());
      
      stat = nc_put_att_text(file_handle, varID, tmp_str.c_str(), strlen(charArray), charArray);
      if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) return -1;    


      //Write the attribute value
      tmp_str = attrs[i].name + HeaderAttribute::val_str;
      strcpy(charArray, attrs[i].value.c_str());
      
      stat = nc_put_att_text(file_handle, varID, tmp_str.c_str(), strlen(charArray), charArray);
      if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) return -1;    
    }
    
    return 1;
    
}//end function write_extra_attributes

#endif
