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
#ifndef __INPOLYGON_HH__
#define __INPOLYGON_HH__

#ifdef __cplusplus


// make sure we're using GCC
#ifndef __GNUC__
#error __GNUC__ is not defined!
#endif    // __GNUC__

#include <iterator>

#include <math.h>
#include <assert.h>

#if __GNUC__ >= 3
#include <limits>
#else    // __GNUC__ < 3
#include <values.h>
#endif    // GCC version check



//==============================================================================
//    InPolygon function (for points)
//==============================================================================


/**
	\brief Tests if a point is inside a polygon

	The polygon is defined by iterators, assumed to point to a subsequence of
	the same length, giving the latitude and longitude of the corners of the
	polygon, <i>in order</i> (there is no convexity assumption)--the sides of
	the polygon are the line segments connecting each point to the following
	point, ending with a line segment connecting the last point to the first.
	As usual, endLatitudes and endLongitudes do not point to the last elements
	in the sequence, but rather one past the end.

	The point is determined to be inside the polygon if it lies to the right of
	(or exactly on) an odd number of line segments, and outside the polygon
	otherwise. Hence, the polygon is left-closed/right-open, where "leftness"
	and "rightness" is determined by the <i>longitude</i> being smaller or
	larger, respectively.

	\param latitude         Latitude of the point to test
	\param longitude        Longitude of the point to test
	\param beginLatitudes   Iterator to the first polygon latitude
	\param beginLongitudes  Iterator to the first polygon longitude
	\param endLatitudes     Iterator to one-past-the-last polygon latitude
	\param endLongitudes    Iterator to one-past-the-last polygon longitude

	\result true if the point is inside the poolygon, false otherwise
*/
template<
	typename t_LatitudeType,
	typename t_LongitudeType,
	typename t_LatitudeIterator,
	typename t_LongitudeIterator
>
bool InPolygon(
	t_LatitudeType             latitude,
	t_LongitudeType            longitude,
	const t_LatitudeIterator&  beginLatitudes,
	const t_LongitudeIterator& beginLongitudes,
	const t_LatitudeIterator&  endLatitudes,
	const t_LongitudeIterator& endLongitudes
)
{
	// the types to which the iterators point
	typedef typename std::iterator_traits< t_LatitudeIterator  >::value_type LatitudeIteratorType;
	typedef typename std::iterator_traits< t_LongitudeIterator >::value_type LongitudeIteratorType;

	bool inside = false;

	t_LatitudeIterator  latitudeIterator(  beginLatitudes  );
	t_LongitudeIterator longitudeIterator( beginLongitudes );
	for ( bool done = false; ! done; ) {

		t_LatitudeIterator  nextLatitudeIterator(  latitudeIterator  );
		t_LongitudeIterator nextLongitudeIterator( longitudeIterator );
		++nextLatitudeIterator;
		++nextLongitudeIterator;

		//  if the next position is past the end, take the first one
		if ( nextLatitudeIterator == endLatitudes ) {

			assert( nextLongitudeIterator == endLongitudes );

			nextLatitudeIterator  = beginLatitudes;
			nextLongitudeIterator = beginLongitudes;

			done = true;
		}
		assert( nextLongitudeIterator != endLongitudes );

		LatitudeIteratorType  lowLatitude   = *latitudeIterator;
		LatitudeIteratorType  lowLongitude  = *longitudeIterator;
		LongitudeIteratorType highLatitude  = *nextLatitudeIterator;
		LongitudeIteratorType highLongitude = *nextLongitudeIterator;

		// make sure lowXXX has the lower latitude
		if ( lowLatitude > highLatitude ) {

		  /*std::swap( lowLatitude,  highLatitude  );
		    std::swap( lowLongitude, highLongitude );*/

		  {
		    LatitudeIteratorType temp( lowLatitude );
		    lowLatitude = highLatitude;
		    highLatitude = temp;
		  }
		  {
		    LongitudeIteratorType temp( lowLongitude );
		    lowLongitude = highLongitude;
		    highLongitude = temp;
		  }

		}

		// if our point is inside the latitude range of this segment ....
		if ( ( latitude >= lowLatitude ) && ( latitude < highLatitude ) ) {

			/*
				.... and the corresponding point on the line is less than (or
				equal to) our point ....
			*/
			if (
				( longitude - lowLongitude ) * ( highLatitude  - lowLatitude  ) >=
				( latitude  - lowLatitude  ) * ( highLongitude - lowLongitude )
			)
			{
				// .... then invert the flag
				inside = ( ! inside );
			}
		}

		latitudeIterator  = nextLatitudeIterator;
		longitudeIterator = nextLongitudeIterator;
	}

	return inside;
}



#endif    /* __cplusplus */

#endif    /* __MATLABISH_HH__ */
