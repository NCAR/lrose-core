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
////////////////////////////////////////////////////////////////////////////////
//
//  Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  January 1998
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _FUZZY_FCN_INC_
#define _FUZZY_FCN_INC_

#include <string>
#include <vector>
#include <cassert>

using namespace std;

template <class T>
class FuzzyFcn
{
public:
   FuzzyFcn( int fcnType, 
             T bad, T missing,
             T x1, T y1,
             T x2, T y2,
             T x3 = (T) 0.0, T y3 = (T) 0.0,
             T x4 = (T) 0.0, T y4 = (T) 0.0,
             float shrink = 1.0, float scale = 1.0);

   FuzzyFcn( int fcnType,
             T bad, T missing,
             const vector<T> &x, const vector<T> &y,
             float shrink = 1.0, float scale = 1.0);

  ~FuzzyFcn(){};

  enum fcnType { DISTANCE_RAMP, LINEAR_BELL, RAMP, RESCALE, STEP, FOUR_POINT_RAMP };

   //
   // Apply the function to a value...return fuzzy value.
   //
   T           apply( T value );

   T           get_maxdist();
   float       get_km_shrink();

   void        setValueDefs(T bad, T missing) { badValueDef = bad,
                                                missingValueDef = missing; }

private:

   vector< T > x;
   vector< T > y;

   T               badValueDef;
   T               missingValueDef;

   int             type;
   float           scale;
   float           shrink;

   void            createRamp( T x1, T y1, T x2, T y2 );

   void            createBell( T x1, T y1, T x2, T y2,
                               T x3, T y3, T x4, T y4 );

   void            createStep( const vector<T> &xx, const vector<T> &yy );

   void            createStep( T x1, T y1, T x2, T y2,
                               T x3, T y3, T x4, T y4 );

   void            createFourPointRamp( T x1, T y1, T x2, T y2,
					T x3, T y3, T x4, T y4 );

   // Default constructor not to be used.
   FuzzyFcn();
};

// 
// The Implementation.
// 

template <class T>
FuzzyFcn<T>::FuzzyFcn( int fType,
                       T bad, T missing,
                       T x1, T y1, 
                       T x2, T y2, 
                       T x3, T y3, 
                       T x4, T y4 ,
                       float fShrink, float fScale )
{
   badValueDef     = bad;
   missingValueDef = missing;

   type = fType;
   switch( type ) {

     case RESCALE:
          scale = fScale;
          break;

     case STEP:
          createStep( x1, y1, x2, y2, x3, y3, x4, y4 );
          break;

     case RAMP:
          createRamp( x1, y1, x2, y2 );
          break;

     case DISTANCE_RAMP:
          shrink = fShrink;
          createRamp( x1, y1, x2, y2 );
          break;

     case LINEAR_BELL:
          createBell( x1, y1, x2, y2, x3, y3, x4, y4 );
          break;

     case FOUR_POINT_RAMP :
          createFourPointRamp( x1, y1, x2, y2, x3, y3, x4, y4 );
          break;

   }
}

template <class T>
FuzzyFcn<T>::FuzzyFcn( int fType,
                       T bad, T missing,
                       const vector<T> &x, const vector<T> &y,
                       float fShrink, float fScale )
{
   badValueDef     = bad;
   missingValueDef = missing;

   type = fType;
   switch( type ) {

     case RESCALE:
          scale = fScale;
          break;

     case RAMP:
          assert( x.size() == 2  &&  y.size() == 2 );
          createRamp( x[0], y[0], x[1], y[1] );
          break;

     case DISTANCE_RAMP:
          assert( x.size() == 2  &&  y.size() == 2 );
          shrink = fShrink;
          createRamp( x[0], y[0], x[1], y[1] );
          break;

     case LINEAR_BELL:
          assert( x.size() == 4  &&  y.size() == 4 );
          createBell( x[0], y[0], x[1], y[1], x[2], y[2], x[3], x[3] );
          break;

     case STEP:
          assert( x.size() == y.size() || x.size()+1 == y.size() );
          createStep( x, y );
          break;

     case FOUR_POINT_RAMP :
          assert( x.size() == 4  &&  y.size() == 4 );
          createFourPointRamp( x[0], y[0], x[1], y[1], x[2], y[2], x[3], x[3] );
          break;

   }
}

template <class T>
void FuzzyFcn<T>::createRamp( T x1, T y1, T x2, T y2 )
{
   if ( x1 < x2 ) {
      x.push_back( x1 );
      x.push_back( x2 );
      y.push_back( y1 );
      y.push_back( y2 );
   }
   else {
      x.push_back( x2 );
      x.push_back( x1 );
      y.push_back( y2 );
      y.push_back( y1 );
   }
}

template <class T>
void FuzzyFcn<T>::createBell( T x1, T y1, T x2, T y2,
                              T x3, T y3, T x4, T y4 )
{
   x.push_back( x1 );
   x.push_back( x2 );               
   x.push_back( x3 );
   x.push_back( x4 );

   y.push_back( y1 );
   y.push_back( y2 );
   y.push_back( y3 );
   y.push_back( y4 );
}

template <class T>
void FuzzyFcn<T>::createStep( const vector<T> &xx, const vector<T> &yy )
{
   //
   // We should probably do something here like make sure
   // that the x values are in increasing order
   //
   x = xx;
   y = yy;
}


template <class T>
void FuzzyFcn<T>::createStep( T x1, T y1, T x2, T y2,
			      T x3, T y3, T x4, T y4 )
{
  //
  // The x values have to be monotonically increasing.
  // If that is not the case then the steps end on that last
  // point for which x was increasing.
  //
  x.push_back( x1 );
  y.push_back( y1 );

  if (x1 < x2){
    x.push_back( x2 );
    y.push_back( y2 );
  } else {
    return;
  }

  if (x2 < x3){
    x.push_back( x3 );
    y.push_back( y3 );
  } else {
    return;
  }

  if (x3 < x4){
    x.push_back( x4 );
    y.push_back( y4 );
  }

  return;

}


template <class T>
void FuzzyFcn<T>::createFourPointRamp( T x1, T y1, T x2, T y2,
				       T x3, T y3, T x4, T y4 )
{
   x.push_back( x1 );
   x.push_back( x2 );               
   x.push_back( x3 );
   x.push_back( x4 );

   y.push_back( y1 );
   y.push_back( y2 );
   y.push_back( y3 );
   y.push_back( y4 );
}




template <class T>
T FuzzyFcn<T>::apply( T value )
{
   T answer;
   size_t i, numJumps;
   bool   answerFound;

   if ( value == badValueDef || value == missingValueDef )
      return value;
    
   switch( type ) {

      case DISTANCE_RAMP:
       //
       // Assumes input is closest distance to value exceeding
       // threshold..just a ramp function...fallthrough
       //

      case RAMP:
           if ( value < x[0] )
              answer = y[0];
           else if ( value > x[1] )
              answer = y[1];
           else
              answer = ((value-x[0])/(x[1]-x[0]))*(y[1]-y[0]) + y[0];
           break;

      case LINEAR_BELL:
           if ( value <= x[0] )
              answer = y[0];
           else if ( value > x[0] && value <= x[1] )
              answer = ((value-x[0])/(x[1]-x[0]))*(y[1]-y[0]) + y[0];
           else if ( value > x[1] && value <= x[2] )
              answer = y[2]; // Arguably should be ((value-x[1])/(x[2]-x[1]))*(y[2]-y[1]) + y[1] - allow for sloped top linear bell.
                             // but instead that is implemented in FOUR_POINT_RAMP for backwards compatibility. Niles.
           else if ( value > x[2] && value <= x[3] )
              answer = ((value-x[2])/(x[3]-x[2]))*(y[3]-y[2]) + y[2];
           else
              answer = y[3];
           break;

      case RESCALE:
           answer = (T) (value * scale);
           break;

      case STEP:
	if (x.size() == 0) return badValueDef;
           answerFound = false;
           numJumps = y.size() - 1;
           for( i=0;  !answerFound && i < numJumps; i++ ) {
              if ( value <= x[i] ) {
                 answer = y[i];
                 answerFound = true;
              }
           }
           if ( !answerFound ) {
              answer = y[i];
           }
           break;

      case FOUR_POINT_RAMP:
           if ( value <= x[0] )
              answer = y[0];
           else if ( value > x[0] && value <= x[1] )
              answer = ((value-x[0])/(x[1]-x[0]))*(y[1]-y[0]) + y[0];
           else if ( value > x[1] && value <= x[2] )
 	      answer = ((value-x[1])/(x[2]-x[1]))*(y[2]-y[1]) + y[1];
           else if ( value > x[2] && value <= x[3] )
              answer = ((value-x[2])/(x[3]-x[2]))*(y[3]-y[2]) + y[2];
           else
              answer = y[3];
           break;


      default:
           answer = value;
   }
   return answer;
}

template <class T>
T FuzzyFcn<T>::get_maxdist()
{
   if ( type == DISTANCE_RAMP )
      return x[1];
   else
      return -1.0;
}

template <class T>
float FuzzyFcn<T>::get_km_shrink()
{
   if ( type == DISTANCE_RAMP )
      return shrink;
   else
      return -1.0;
}

#endif
