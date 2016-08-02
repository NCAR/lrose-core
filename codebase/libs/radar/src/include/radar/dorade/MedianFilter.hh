/* 	$Id: MedianFilter.hh,v 1.1 2008/01/24 22:22:32 rehak Exp $	 */

# ifndef MEDIANFILTER_HH
# define MEDIANFILTER_HH


// c---------------------------------------------------------------------------

struct MFentry {
    struct MFentry *prev;
    struct MFentry *next;
    Xfloat value;
    int ivalue;
    int index;
} ;

// c---------------------------------------------------------------------------

struct MFentryInt {
    struct MFentry *prev;
    struct MFentry *next;
    int value;
    int index;
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















