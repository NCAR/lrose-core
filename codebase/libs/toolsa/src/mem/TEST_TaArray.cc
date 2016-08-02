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
////////////////////////////////////////////////////////////////////
// TEST_TaArray.cc
//
// Test the TaArray class
//
////////////////////////////////////////////////////////////////////

#include <toolsa/TaArray.hh>
#include <toolsa/TaArray2D.hh>
#include <iostream>

//////////////////////
// test the compile
//

void TEST_TaArray()

{ 

  TaArray<double> array_;
  double *array = array_.alloc(100);
  double *ptr = array_.buf();
  if (array != ptr) {
    cerr << "ERROR - TEST_TaArray" << endl;
    cerr << "  ptr does not match original allocation" << endl;
  }

  TaArray2D<double> array2D_;
  double **array2D = array2D_.alloc(10, 100);
  double **ptr2D = array2D_.dat2D();
  if (array2D != ptr2D) {
    cerr << "ERROR - TEST_TaArray" << endl;
    cerr << "  ptr2D does not match original allocation" << endl;
  }

} 

