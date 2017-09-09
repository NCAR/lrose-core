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
 * SimpleGrid.hh: Class representing a simple data grid of 2D float data.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 2000
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef SimpleGrid_HH
#define SimpleGrid_HH

/*
 **************************** includes **********************************
 */

#include <cstdio>

/*
 ******************************* defines ********************************
 */


/*
 ******************************* structures *****************************
 */

/*
 ************************* global variables *****************************
 */

/*
 ***************************** function prototypes **********************
 */

/*
 ************************* class definitions ****************************
 */

template <class T>
class SimpleGrid
{
 public:

  //////////////////
  // Constructors //
  //////////////////

  SimpleGrid(const int nx,
	     const int ny) :
    _nx(nx),
    _ny(ny)
  {
    _grid = new T[_nx * _ny];
  }
  
  
  // Copy constructor

  SimpleGrid(const SimpleGrid& orig)
  {
    _nx = orig._nx;
    _ny = orig._ny;
    
    _grid = new T[_nx * _ny];

    memcpy(_grid, orig._grid, _nx * _ny * sizeof(T));
  }
  

  ////////////////
  // Destructor //
  ////////////////

  ~SimpleGrid(void)
  {
    delete [] _grid;
  }
  

  ///////////////////////////
  // Miscellaneous methods //
  ///////////////////////////

  // Reallocate space for the grid.

  void realloc(const int nx, const int ny)
  {
    if (nx != _nx || ny != _ny)
    {
      delete [] _grid;
      
      _grid = new T[nx * ny];
      
      _nx = nx;
      _ny = ny;
    }
  }
  
  
  ///////////////
  // Operators //
  ///////////////

  // Assignment

  SimpleGrid& operator=(const SimpleGrid& orig)
  {
    if (this != &orig)
    {
      _nx = orig._nx;
      _ny = orig._ny;
      
      delete [] _grid;
      
      _grid = new T[_nx * _ny];

      memcpy(_grid, orig._grid, _nx * _ny * sizeof(T));
    }
  }
  

  ///////////////
  // Accessors //
  ///////////////

  inline T *getGrid(void) const
  {
    return _grid;
  }
  
  inline const T get(const int& index) const
  {
    return _grid[index];
  }
  
  inline const T get(const int& x, const int& y) const
  {
    return _grid[x + (_nx * y)];
  }
  
  inline void set(const int& index, const T& value)
  {
    _grid[index] = value;
  }
  
  inline void set(const int& x, const int& y, const T& value)
  {
    _grid[x + (_nx * y)] = value;
  }
  
  inline void initializeGrid(T init_value)
  {
    for (int i = 0; i < _nx * _ny; ++i)
      _grid[i] = init_value;
  }
  
  inline const int getNx(void) const
  {
    return _nx;
  }
  
  inline const int getNy(void) const
  {
    return _ny;
  }
  

 private:

  int _nx;
  int _ny;
  
  T *_grid;
  
  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("SimpleGrid");
  }
  
};


#endif
