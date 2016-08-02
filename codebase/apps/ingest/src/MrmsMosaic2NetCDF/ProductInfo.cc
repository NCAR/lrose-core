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
#include "ProductInfo.h"


using namespace std;

/*************************************/
/*************************************/
/** S T A T I C  C O N S T A N T S  **/
/*************************************/

const float ProductInfo::UNDEFINED = 12345.0;

/********************************************/
/** E N D  S T A T I C  C O N S T A N T S  **/
/********************************************/
/********************************************/
    


/*****************************/	
/*****************************/
/** C O N S T R U C T O R S **/
/*****************************/
    
//default constructor  
ProductInfo::ProductInfo() 
{
    varName.clear();
    varUnit.clear();
    varMissing = UNDEFINED;
    varNoCoverage = UNDEFINED;
    
    cfName.clear();
    cfUnit.clear();
    cfLongName.clear();
    cfMissing = UNDEFINED;
    cfNoCoverage = UNDEFINED;
}
    
    
//2nd constructor
ProductInfo::ProductInfo(string vN, string vU, float vM, float vNC, float fT,
                         string cN, string cU, string cLN, float cM, float cNC)
{
    varName = vN;
    varUnit = vU;
    varMissing = vM;
    varNoCoverage = vNC;
    
    fcstTime = fT;
    
    cfName = cN;
    cfUnit = cU;
    cfLongName = cLN;
    cfMissing = cM;
    cfNoCoverage = cNC;

}//end 2nd constructor 

    
//copy constructor
ProductInfo::ProductInfo(const ProductInfo& pI)
{
    varName = pI.varName;
    varUnit = pI.varUnit;
    varMissing = pI.varMissing;
    varNoCoverage = pI.varNoCoverage;
    
    fcstTime = pI.fcstTime;
    
    cfName = pI.cfName;
    cfUnit = pI.cfUnit;
    cfLongName = pI.cfLongName;
    cfMissing = pI.cfMissing;
    cfNoCoverage = pI.cfNoCoverage;
}
    
    
//deconstructor
ProductInfo::~ProductInfo() { }
    
/************************************/
/** E N D  C O N S T R U C T O R S **/
/************************************/
/************************************/	
   


/********************************/
/********************************/
/** P U B L I C  M E T H O D S **/
/********************************/
  
/*------------------------------------------------------------------

	Method:		setValues
		
	Purpose:	Sets objects values
		
------------------------------------------------------------------*/

void ProductInfo::setValues(string vN, string vU, float vM, float vNC, float fT,
                            string cN, string cU, string cLN, float cM, float cNC)
{
    varName = vN;
    varUnit = vU;
    varMissing = vM;
    varNoCoverage = vNC;
    
    fcstTime = fT;
    
    cfName = cN;
    cfUnit = cU;
    cfLongName = cLN;
    cfMissing = cM;
    cfNoCoverage = cNC;

}//end public method ProductInfo::setValues


/*------------------------------------------------------------------

	Method:		nameMatches
		
	Purpose:	Clears object to original (blank) state
		
------------------------------------------------------------------*/

bool ProductInfo::isMatch(string testName, string testUnit)
{
    bool nameMatch = false;
    bool unitMatch = false;
    
    if(varName == testName) nameMatch = true; 
    
    if(testUnit.empty()) unitMatch = true;
    else if(varUnit == testUnit) unitMatch = true;
    
    return (nameMatch && unitMatch);
    
}//end public method ProductInfo::isMatch


/*------------------------------------------------------------------

	Method:		empty
		
	Purpose:	returns true/false indicating of object is empty
		
------------------------------------------------------------------*/

bool ProductInfo::incomplete()
{
    if( varName.empty() || varUnit.empty() ) return true;
    if( fcstTime == UNDEFINED ) return true;
    if( cfName.empty() || cfLongName.empty() || cfUnit.empty() ) return true;
    if( (cfMissing == UNDEFINED) || (cfNoCoverage == UNDEFINED) ) return true; 
    else return false;
    
}//end public method ProductInfo::empty

  
/*------------------------------------------------------------------

	Method:		clear
		
	Purpose:	Clears object to original (blank) state
		
------------------------------------------------------------------*/
    
void ProductInfo::clear()
{
    varName.clear();
    varUnit.clear();
    varMissing = UNDEFINED;
    varNoCoverage = UNDEFINED;
    
    fcstTime = UNDEFINED;
    
    cfName.clear();
    cfUnit.clear();
    cfLongName.clear();
    cfMissing = UNDEFINED;
    cfNoCoverage = UNDEFINED;
      
}//end public method ProductInfo::clear
  
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
    
void ProductInfo::operator= (ProductInfo pI)
{
    varName = pI.varName;
    varUnit = pI.varUnit;
    varMissing = pI.varMissing;
    varNoCoverage = pI.varNoCoverage;
    
    fcstTime = pI.fcstTime;
    
    cfName = pI.cfName;
    cfUnit = pI.cfUnit;
    cfLongName = pI.cfLongName;
    cfMissing = pI.cfMissing;
    cfNoCoverage = pI.cfNoCoverage;
    
}//end operator= method

/***************************************************/
/** E N D  O V E R L O A D E D  O P E R A T O R S **/
/***************************************************/
/***************************************************/

//End Class ProductInfo




