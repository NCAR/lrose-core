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

#ifndef _MDVX_GRID_INC_
#define _MDVX_GRID_INC_

#include <string>
#include <unistd.h>
#include <Mdv/DsMdvx.hh>
#include <euclid/TypeGrid.hh>
using namespace std;


class MdvxGrid
{
public:

   MdvxGrid(){};
  ~MdvxGrid(){};

   //
   // Input data sequence
   //
   static int   setMdvxReadFromGrid( DsMdvx& mdvx, const Grid& grid );

   static int   setGridDataFromMdvxField( Grid& grid, 
                                          const DsMdvx& mdvx,
                                          const string& fieldName );

   static int   setGridDataFromMdvxField( Grid& grid, 
                                          const DsMdvx& mdvx,
                                          const int& fieldNum );

   static int   setGridDataFromMdvxField( Grid& grid, 
					  const MdvxField& field );
  
   //
   // Output data sequence
   //
   static int addMdvxFieldFromGrid( 
                 DsMdvx & mdvx,
                 const string & fieldName,
                 const Grid & grid,
                 Mdvx::encoding_type_t encoding       = Mdvx::ENCODING_INT8,
                 Mdvx::compression_type_t compression = Mdvx::COMPRESSION_GZIP,
                 Mdvx::scaling_type_t scaling         = Mdvx::SCALING_DYNAMIC,
                 const double scale                   = 1.0,
                 const double bias                    = 0.0 );


private:

};

# endif
