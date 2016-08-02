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

#include <cstdio>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <Spdb/Spdb.hh>
#include <cmath>

int main(int argc, char *argv[]){

  if (
      (argc < 2) ||
      (!(strcmp(argv[1],"-h")))
      ){
    cerr << "USAGE : Hash Input_arg where Input_arg is either" << endl;
    cerr << "        a four character station ID string or a" << endl;
    cerr << "        number resulting from hashing such a string. If" << endl;
    cerr << "        you give it a number, Hash will print the four" << endl;
    cerr << "        character ID string for that number. If you give" << endl;
    cerr << "        Hash a four character string then Hash will print" << endl;
    cerr << "        the number corresponding to that. Niles Oien April 2005." << endl;
    exit(0);
  }
  //
  // See if we can get a non-zero int out of the
  // argument we got.
  //
  long hashInt = (long)rint(atof(argv[1]));
  if (hashInt != 0){
    //
    // We have a number, print the string.
    //
    cerr << Spdb::dehashInt32To4Chars(hashInt) << endl;
    //
  } else {
    //
    // We have a string, print the number.
    //
    cerr << Spdb::hash4CharsToInt32(argv[1]) << endl;
    //
  }

  return 0;

}

