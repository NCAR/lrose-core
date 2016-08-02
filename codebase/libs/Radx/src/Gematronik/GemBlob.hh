/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 1999
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1999/03/14 14:18:54
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/////////////////////////////////////////////////////////////
// GemBlob.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2014
//
///////////////////////////////////////////////////////////////

#ifndef GemBlob_HH
#define GemBlob_HH

#include <string>
#include <vector>
#include <iostream>
using namespace std;

////////////////////////
// This class

class GemBlob {
  
public:
  
  // constructor
  
  GemBlob(int id,
       bool debug);
  
  // destructor
  
  ~GemBlob();

  // load data
  // returns 0 on success, -1 on failure
  
  int loadData(int size,
               const string &compression,
               const char *data);
  
  // clear data
  
  void clearData();
  
  // get methods

  int getId() const { return _id; }
  int getSize() const { return _size; }
  const char *getData() const { return _data; }
  
protected:
  
private:

  int _id;
  int _size;
  char *_data;
  bool _debug;

};

#endif

