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

/************************************************************************

Module:	usort_stl.cc

Author:	C S Morse

Date:	Tue Sep 30 17:24:24 2003

Description: sorting routines utilizing C++ STL classes.

************************************************************************/



/* System include files / Local include files */

#include <vector>

/* Constant definitions / Macro definitions / Type definitions */


/* External global variables / Non-static global variables / Static globals */


/* External functions / Internal global functions / Internal static functions */

using namespace std;

static void sift_down( vector<double>& array, int root, int bottom );
static void sift_down( vector<float>& array, int root, int bottom );
static void sift_down( vector<int>& array, int root, int bottom );

static void idx_sift_down( vector<int>& index, vector<double>& array, 
			   int root, int bottom );
static void idx_sift_down( vector<int>& index, vector<float>& array, 
			   int root, int bottom );
static void idx_sift_down( vector<int>& index, vector<int>& array, 
			   int root, int bottom );


/************************************************************************

Function Name: 	usort

Description:	sorts a vector of doubles in place using a heap sort algorithm

Returns:	none

Globals:	none

Notes:	Adapted (and debugged!) from source found at
        http://linux.wku.edu/~lamonml/algor/sort/heap.html
        
************************************************************************/

void 
usort( vector<double>& array )
{
  double temp;
  int i;

  int size = static_cast<int>(array.size());
  
  for ( i=(size/2); i>=0; --i )
    sift_down(array, i, size-1);

  for ( i=size-1; i>=1; --i )
    {
      temp = array[0];
      array[0] = array[i];
      array[i] = temp;
      sift_down(array, 0, i-1);
    }
}

void 
usort( vector<float>& array )
{
  float temp;
  int i;

  int size = static_cast<int>(array.size());

  for ( i=(size/2); i>=0; --i )
    sift_down(array, i, size-1);

  for ( i=size-1; i>=1; --i )
    {
      temp = array[0];
      array[0] = array[i];
      array[i] = temp;
      sift_down(array, 0, i-1);
    }
}

void 
usort( vector<int>& array )
{
  int temp;
  int i;

  int size = static_cast<int>(array.size());

  for ( i=(size/2); i>=0; --i )
    sift_down(array, i, size-1);

  for ( i=size-1; i>=1; --i )
    {
      temp = array[0];
      array[0] = array[i];
      array[i] = temp;
      sift_down(array, 0, i-1);
    }
}

/************************************************************************

Function Name: 	sift_down

Description:	utility function for heap sort

Returns:	none

Globals:	none

Notes:

************************************************************************/

static void 
sift_down( vector<double>& array, int root, int bottom )
{
  double temp;
  int done, max_child;

  done = 0;
  while ((root<<1 <= bottom) && (!done))
    {
      if (root<<1 == bottom)
	max_child = bottom;
      else if (array[root<<1] > array[(root<<1) + 1])
	max_child = root<<1;
      else
	max_child = (root<<1) + 1;

      if (array[root] < array[max_child])
	{
	  temp = array[root];
	  array[root] = array[max_child];
	  array[max_child] = temp;
	  root = max_child;
	}
      else
	done = 1;
    }
}

static void 
sift_down( vector<float>& array, int root, int bottom )
{
  float temp;
  int done, max_child;

  done = 0;
  while (((root<<1) <= bottom) && (!done))
    {
      if (root<<1 == bottom)
	max_child = root<<1;
      else if (array[root<<1] > array[(root<<1) + 1])
	max_child = root<<1;
      else
	max_child = (root<<1) + 1;

      if (array[root] < array[max_child])
	{
	  temp = array[root];
	  array[root] = array[max_child];
	  array[max_child] = temp;
	  root = max_child;
	}
      else
	done = 1;
    }
}

static void 
sift_down( vector<int>& array, int root, int bottom )
{
  int temp;
  int done, max_child;

  done = 0;
  while (((root<<1) <= bottom) && (!done))
    {
      if (root<<1 == bottom)
	max_child = root<<1;
      else if (array[root<<1] > array[(root<<1) + 1])
	max_child = root<<1;
      else
	max_child = (root<<1) + 1;

      if (array[root] < array[max_child])
	{
	  temp = array[root];
	  array[root] = array[max_child];
	  array[max_child] = temp;
	  root = max_child;
	}
      else
	done = 1;
    }
}

/************************************************************************

Function Name: 	usort_index

Description:	indexes an input vector so that the indexed vector is 
                sorted in ascending order

Returns:	none

Globals:	none

Notes:	Adapted from usort.

************************************************************************/

void 
usort_index( vector<double>& array, int size, vector<int>& index )
{
  int temp;
  int i;

  /* initialize the index array */
  for ( i=0; i<size; ++i )
    index[i] = i;
  
  for ( i=(size/2); i>=0; --i )
    idx_sift_down(index, array, i, size-1);

  for ( i=size-1; i>=1; --i )
    {
      temp = index[0];
      index[0] = index[i];
      index[i] = temp;
      idx_sift_down(index, array, 0, i-1);
    }
}

void 
usort_index( vector<float>& array, int size, vector<int>& index )
{
  int temp;
  int i;

  /* initialize the index array */
  for ( i=0; i<size; ++i )
    index[i] = i;
  
  for ( i=(size/2); i>=0; --i )
    idx_sift_down(index, array, i, size-1);

  for ( i=size-1; i>=1; --i )
    {
      temp = index[0];
      index[0] = index[i];
      index[i] = temp;
      idx_sift_down(index, array, 0, i-1);
    }
}

void 
usort_index( vector<int>& array, int size, vector<int>& index )
{
  int temp;
  int i;

  /* initialize the index array */
  for ( i=0; i<size; ++i )
    index[i] = i;
  
  for ( i=(size/2); i>=0; --i )
    idx_sift_down(index, array, i, size-1);

  for ( i=size-1; i>=1; --i )
    {
      temp = index[0];
      index[0] = index[i];
      index[i] = temp;
      idx_sift_down(index, array, 0, i-1);
    }
}

/************************************************************************

Function Name: 	idx_sift_down

Description:	utility function for heap sort of indexes

Returns:	none

Globals:	none

Notes:

************************************************************************/

static void 
idx_sift_down( vector<int>& index, vector<double>& array, int root, int bottom )
{
  int done, max_child, temp;

  done = 0;
  while (((root<<1) <= bottom) && (!done))
    {
      if (root<<1 == bottom)
	max_child = root<<1;
      else if (array[index[root<<1]] > array[index[(root<<1) + 1]])
	max_child = root<<1;
      else
	max_child = (root<<1) + 1;

      if (array[index[root]] < array[index[max_child]])
	{
	  temp = index[root];
	  index[root] = index[max_child];
	  index[max_child] = temp;
	  root = max_child;
	}
      else
	done = 1;
    }
}

static void 
idx_sift_down( vector<int>& index, vector<float>& array, int root, int bottom )
{
  int done, max_child, temp;

  done = 0;
  while (((root<<1) <= bottom) && (!done))
    {
      if (root<<1 == bottom)
	max_child = root<<1;
      else if (array[index[root<<1]] > array[index[(root<<1) + 1]])
	max_child = root<<1;
      else
	max_child = (root<<1) + 1;

      if (array[index[root]] < array[index[max_child]])
	{
	  temp = index[root];
	  index[root] = index[max_child];
	  index[max_child] = temp;
	  root = max_child;
	}
      else
	done = 1;
    }
}

static void 
idx_sift_down( vector<int>& index, vector<int>& array, int root, int bottom )
{
  int done, max_child, temp;

  done = 0;
  while (((root<<1) <= bottom) && (!done))
    {
      if (root<<1 == bottom)
	max_child = root<<1;
      else if (array[index[root<<1]] > array[index[(root<<1) + 1]])
	max_child = root<<1;
      else
	max_child = (root<<1) + 1;

      if (array[index[root]] < array[index[max_child]])
	{
	  temp = index[root];
	  index[root] = index[max_child];
	  index[max_child] = temp;
	  root = max_child;
	}
      else
	done = 1;
    }
}




