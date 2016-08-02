///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//         This example code is from the book:
//
//           Object-Oriented Programming with C++ and OSF/Motif, 2nd Edition
//         by
//           Douglas Young
//           Prentice Hall, 1995
//           ISBN 0-13-20925507
//
//         Copyright 1995 by Prentice Hall
//         All Rights Reserved
//
//  Permission to use, copy, modify, and distribute this software for 
//  any purpose except publication and without fee is hereby granted, provided 
//  that the above copyright notice appear in all copies of the software.
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////
// SimpleList.h: A Simple List Template
///////////////////////////////////////////////////////

#ifndef SIMPLE_LIST_H
#define SIMPLE_LIST_H

#include <stdlib.h>

template <class T>
class SimpleList {
 public:

   SimpleList()
   {
     _size = 0;   // Initialize the array
     _data = NULL;
   }
  
   ~SimpleList()
   {
     free((char *)_data); 
   }
  
   void add(T t)
   {
     // Allocate the array to be one larger than its current size
  
     if (_data == NULL)
       _data = (T *)malloc(sizeof(T) * (_size + 1));
     else
       _data = (T *)realloc((char *)_data, sizeof(T) * (_size + 1));
     
     // Add the new element to the end of the list
  
     _data[_size++] = t;
   }
  
   void remove(T t)
   {
     // First, find the item on the list.
  
     for (int i = 0; i < _size; i++)
       if (memcmp(&_data[i], &t, sizeof(t)) == 0)
       {     
	 // Once found, decrease the size of the list,
	 // and move all remaining items in the list up one space.
  
	 _size--;
  	    
	 for (int j = i; j < _size; j++)
	   _data[j] = _data[j+1];
  
	 // Reduce the size of the list.
  		
	 _data = (T *)realloc((char *)_data, sizeof(T) * _size );
  
	 return;
       }
   }

   // Check for the presence of a node in the list.  If the node
   // exists in the list, returns the index to the node.  Otherwise,
   // returns -1.

   int is_present(T t)
   {
     for (int i = 0; i < _size; i++)
     {
       if (memcmp(&_data[i], &t, sizeof(t)) == 0)
	 return(i);
     } /* endfor - i */
     
     return(-1);
   }
   
   T operator[](int i) {return _data[i];}
   int size() { return _size; };
  
 private:
   T *_data;
   int _size;
};

#endif
