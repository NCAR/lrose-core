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
/* 	$Id: MedianFilter.hh,v 1.3 2016/03/07 01:23:00 dixon Exp $	 */

# ifndef MEDIANFILTER_HH
# define MEDIANFILTER_HH

#include <dataport/port_types.h>

// c---------------------------------------------------------------------------

struct MFentry {
    struct MFentry *prev;
    struct MFentry *next;
    Xfloat value;
    si32 ivalue;
    si32 index;
} ;

// c---------------------------------------------------------------------------

struct MFentryInt {
    struct MFentry *prev;
    struct MFentry *next;
    si32 value;
    si32 index;
} ;

// typedef struct MFentry MFentry;

// c---------------------------------------------------------------------------

class MedianFilterList {	// First order median filter list

private:

    MFentry *smallest;		// points to beginning of sorted list of values
    MFentry *median;
    int medianLoopCount;	// gets you from "smallest" to the median;
  
    MFentry **entries;		// used to find entry in sorted list
    // corresponding to the current gate being filtered
    int sizeofList;
    int ndxNext;		// entries ndx to next entry in filter list
    int counter;

    inline void incNdxNext()
    { if(++ndxNext >= sizeofList) { ndxNext -= sizeofList; } }

    inline void extractEntry(MFentry *here)
    { here->prev->next = here->next; here->next->prev = here->prev; }

    inline void insertEntry(MFentry *here, MFentry *there)
    {
	here->next = there;
	here->prev = there->prev;
	there->prev->next = here;
	there->prev = here;
    }

public:

    MedianFilterList(int size)
    {
	MFentry *next, *prev;
	sizeofList = size < 1 ? 1 : size;
	medianLoopCount = size/2 -1;
	counter = 0;

	entries = new MFentry * [size];
	ndxNext = 0;
	int ii;

	for(ii=0; ii < sizeofList; ii++) {
	    next = new MFentry;

	    next->value = 0;
	    next->index = ii;

	    if(ii) {
		prev->next = next;
		next->prev = prev;
	    }
	    else {
		smallest = next;
	    }
	    prev = next;
	    *(entries + ii) = next; // entries always points to the same struct
	}
	smallest->prev = next;
    }

    ~MedianFilterList();

    Xfloat replaceNext(Xfloat value);
    inline Xfloat returnMedian() { return median->value; }
    inline int returnSize() { return sizeofList; }
    inline int returnCounter() { return counter; }
    void reset(Xfloat value);

    void insertNext(Xfloat value)
    {
	counter++;
	MFentry *newest = *(entries + ndxNext);
	incNdxNext();
	
	newest->value = value;
    }
};

# endif  // MEDIANFILTER_HH





# ifdef notyet
template <class Type>
class MedianFilterList {	// First order median filter list

private:

    Type *smallest;		// points to beginning of sorted list of values
    Type *median;
    int medianLoopCount;	// gets you from "smallest" to the median;
  
    Type **entries;		// used to find entry in sorted list
    // corresponding to the current gate being filtered
    int sizeofList;
    int ndxNext;		// entries ndx to next entry in filter list

    inline void incNdxNext()
    { if(++ndxNext >= sizeofList) { ndxNext -= sizeofList; } }

    inline void extractEntry(Type *here)
    { here->prev->next = here->next; here->next->prev = here->prev; }

    inline void insertEntry(Type *here, Type *there)
    {
	here->next = there;
	here->prev = there->prev;
	there->prev->next = here;
	there->prev = here;
    }

};
# endif















