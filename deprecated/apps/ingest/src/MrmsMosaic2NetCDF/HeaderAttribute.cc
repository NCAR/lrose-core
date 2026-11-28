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
#include "HeaderAttribute.h"

using namespace std;

/*************************************/
/*************************************/
/** S T A T I C  C O N S T A N T S  **/
/*************************************/

string HeaderAttribute::unit_str = "-unit";
string HeaderAttribute::val_str = "-value";

/********************************************/
/** E N D  S T A T I C  C O N S T A N T S  **/
/********************************************/
/********************************************/
    


/*****************************/	
/*****************************/
/** C O N S T R U C T O R S **/
/*****************************/
    
//default constructor  
HeaderAttribute::HeaderAttribute() {}
    
    
//2nd constuctor
HeaderAttribute::HeaderAttribute(string initial_name)
{ 
    name = initial_name; 
}  
  
//3nd constuctor
HeaderAttribute::HeaderAttribute(string init_name, string init_unit, string init_value)
{ 
    name = init_name; 
    unit = init_unit; 
    value = init_value; 
}  
  
//copy constructor
HeaderAttribute::HeaderAttribute(const HeaderAttribute& hdrA)
{
    name = hdrA.name;
    unit = hdrA.unit;
    value = hdrA.value;
}
    
    
//deconstructor
HeaderAttribute::~HeaderAttribute() { }
    
/************************************/
/** E N D  C O N S T R U C T O R S **/
/************************************/
/************************************/	
   


/********************************/
/********************************/
/** P U B L I C  M E T H O D S **/
/********************************/
    
/*------------------------------------------------------------------

	Method:		clear
		
	Purpose:	Clears object to original (blank) state
		
------------------------------------------------------------------*/
    
void HeaderAttribute::clear()
{
    name.empty();
    unit.empty();
    value.empty();
      
}//end clear method
  
/***************************************/
/** E N D  P U B L I C  M E T H O D S **/
/***************************************/
/***************************************/



/**********************************/
/**********************************/
/** P R I V A T E  M E T H O D S **/
/**********************************/

//NONE

/*****************************************/
/** E N D  P R I V A T E  M E T H O D S **/
/*****************************************/
/*****************************************/



/********************************************/
/********************************************/
/** O V E R L O A D E D  O P E R A T O R S **/
/********************************************/		
    
void HeaderAttribute::operator= (HeaderAttribute hdrA)
{
    //cout<<"LatLonField operator="<<endl;
    name = hdrA.name;
    unit = hdrA.unit;      
    value = hdrA.value;
    
}//end operator= method

/***************************************************/
/** E N D  O V E R L O A D E D  O P E R A T O R S **/
/***************************************************/
/***************************************************/

//End Class HeaderAttribute




