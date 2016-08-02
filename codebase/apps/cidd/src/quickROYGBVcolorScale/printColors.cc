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
// Small program to write the ROYGBV colorscale values (hex encoded)
// into the file colors.dat so that the program genScale can read them
// to write a colorscale for CIDD. Niles Oien.
//
#include <cstdio>

int main(int argc, char *argv[]){

  int r,g,b;

  FILE *fp = fopen("colors.dat","w");

  //
  // For ROYGBV - 5 steps -
  //

  //
  // Step one : red and green at 0, blue goes 127 to 255 delta 16
  //
  r = 0; g = 0;
  for (b = 127; b <= 255; b=b+16){
    fprintf(fp, "%02x%02x%02x\n", r,g,b); 
  }


  //
  // Step two : blue holds at 255, red holds at 0, green goes 1 to 255 delta 16
  //
  b=255; r = 0;
  for (g = 15; g <= 255; g=g+16){
    fprintf(fp, "%02x%02x%02x\n", r,g,b); 
  }

  //
  // Step three : Green 255, red goes from 1 to 255, blues goes from 255 to 0
  //
  g = 255;
  r = 15;
  for (b = 240; b >= 0; b=b-16){
    fprintf(fp, "%02x%02x%02x\n", r,g,b); 
    r = r + 16;
  }

  //
  // Step four : red 255 blue 0 green 255 to 0
  //
  b=0; r = 255;
  for (g = 240; g >=0; g=g-16){
    fprintf(fp, "%02x%02x%02x\n", r,g,b); 
  }

  //
  // Step 5 : b=0 g=0 red 255 -> 127
  //
  b=0; g=0;
  for (r = 240; r >= 127; r=r-16){
    fprintf(fp, "%02x%02x%02x\n", r,g,b); 
  }


  //
  // Colors are now in file colors.dat
  //


  fclose(fp);


}
