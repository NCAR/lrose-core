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
#ifndef PRODUCTINFO_H
#define PRODUCTINFO_H

#include <string>
#include <iostream>

using namespace std;

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

	File:		ProductInfo
	
	Date:		April 2011
	
	Author:		Carrie Langston (CIMMS/NSSL)
		
	Purpose:	Stores product information required to convert
	            HMRG binary files to WG-displayable WDSS-II NetCDF
																
	_____________________________________________________________			
	Modification History:
       

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

class ProductInfo
{
  public:

    static const float UNDEFINED;
  
    string varName;
    string varUnit;
    float varMissing;
    float varNoCoverage;
    
    float fcstTime;
    
    string cfName;
    string cfUnit;
    string cfLongName;
    float cfMissing;
    float cfNoCoverage;
    
    
    //default constructor  
    ProductInfo();
        
    //2nd constuctor
    ProductInfo(string vN, string vU, float vM, float vNC, float fT,
                string cN, string cU, string cLN, float cM, float cNC); 
  
    //copy constructor
    ProductInfo(const ProductInfo& pI);
    
    //destructor
    ~ProductInfo();
    
    
    //public methods
    void setValues(string vN, string vU, float vM, float vNC, float fT,
                   string cN, string cU, string cLN, float cM, float cNC); 
    bool isMatch(string testName, string testUnit);
    bool incomplete();
    void clear();
    
    
    //overloaded operators
    void operator= (ProductInfo pI);
  
};
//end class ProductInfo

#endif

