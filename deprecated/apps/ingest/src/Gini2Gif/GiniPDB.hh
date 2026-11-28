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

//
// A typedef and some routines for dealing with the GINI Product
// Definition Block (PDB) in GINI satellite files. Niles Oien.
//

#include <cstdio>
using namespace std;

const int GiniMercatorProj = 1;
const int GiniLambertProj = 3;
const int GiniPolarStereoProj = 5;


typedef struct {
  
  char Source[16];
  char Creator[16];
  char Sector[32];
  char Channel[32];
  int NumRecords;
  int SizeRecords;
  int Year, Month, Day;
  int Hour, Min, Sec, HSec;
  int Projection;
  int Nx, Ny;
  double Lat1, Lon1;
  double Lov;
  double Dx, Dy;
  int SouthPole;
  int ScanLeftToRight;
  int ScanTopToBottom;
  int ScanInXfirst;
  double Latin;
  int ImageRes;
  int Compression;
  int CreatorVersion;
  int PDBSize;
  int NavIncluded;
  int CalIncluded;

} GiniPDB_t;


void DecodeGiniPDB(unsigned char pdb[512], GiniPDB_t *GiniPDB);

void PrintGiniPDB(GiniPDB_t GiniPDB, FILE *out);



