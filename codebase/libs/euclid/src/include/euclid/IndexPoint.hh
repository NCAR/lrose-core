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

#ifndef _INDEX_POINT_INC_
#define _INDEX_POINT_INC_

#include <vector>
#include <cstdio>
using namespace std;

//
// Forward class declarations
//
class Grid;

class IndexPoint {
public:

   IndexPoint( vector< float >& vals, int ix, int iy=0, int iz=0, 
               Grid* grid=NULL );
   IndexPoint( int ix, int iy=0, int iz=0, Grid* grid=NULL );
   IndexPoint( const IndexPoint& pt );
   IndexPoint();
   
   ~IndexPoint();

   void    copy( const IndexPoint& pt );

   int     getXIdex(){ return( xIdex ); }
   int     getYIdex(){ return( yIdex ); }
   int     getZIdex(){ return( zIdex ); }

   vector< float >* getValues()
       { return( &values ); }
   int getNumVals(){ return( (int) values.size() ); }

   void    set( int ix, int iy=0, int iz=0 );
   void    setValue( int valueNum, float val ){ values[valueNum] = val; }
   float   getValue( int valueNum ){ return( values[valueNum] ); }
   
protected:

   int xIdex;
   int yIdex;
   int zIdex;

   Grid *refGrid;

   vector< float > values;
};

#endif
   
