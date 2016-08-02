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
#include <string>
#include <fstream>
#include <strstream>
#include <iostream>
#include <stdio.h>

using namespace std;

#define MAX_MAP_ENTRIES 255

static void printUsage();
static void mapEntry( ofstream &os, float min, float max,
                      float red, float green, float blue );


int main( int argc, char **argv ) 
{
   int         i, numMapEntries = 0, numSpecs = 0;
   int         numShades1, numShades2;
   float       dataVal, dataVal1, dataVal2, dataFraction, dataStep;
   float       red, red1, red2, redFraction;
   float       green, green1, green2, greenFraction;
   float       blue, blue1, blue2, blueFraction;
   char        line[BUFSIZ];
   ifstream    colorSpec;
   ofstream    colorMap;
   istrstream  iss( line, BUFSIZ );

   //
   // Make sure the require command line args are there
   //
   if ( argc != 3 ) {
      printUsage();
      return( -1 );
   }

   //
   // Open the colorspec and colormap files
   //
   colorSpec.open( argv[1] );
   if ( !colorSpec ) {
      cerr << "ERROR: Cannot open the input colorspec file " << argv[1] << endl;
      return( -1 );
   }
   colorMap.open( argv[2] );
   if ( !colorMap ) {
      cerr << "ERROR: Cannot open the output colormap file " << argv[2] << endl;
      return( -1 );
   }

   //
   // Get the first spec
   //
   colorSpec.getline( line, BUFSIZ );
   if ( colorSpec.eof() ) {
      cerr << "ERROR: Empty colorspec file " << argv[1] << endl;
      return( -1 );
   }

   iss >> dataVal1;
   iss >> numShades1;
   iss >> red1;
   iss >> green1;
   iss >> blue1;
   cout << "val = " << dataVal1 << "   shades = " << numShades1
        << "   red = "   << red1 
        << "   green = " << green1 
        << "   blue = "  << blue1 
        << endl;
   numSpecs++;

   //
   // Interpolate for each entry in the specification
   //
   for( colorSpec.getline( line, BUFSIZ );  colorSpec && 
                                            numMapEntries < MAX_MAP_ENTRIES;
        colorSpec.getline( line, BUFSIZ ) ) {

      //
      // Get the next specification
      //
      iss.seekg( 0 );
      iss >> dataVal2;
      iss >> numShades2;
      iss >> red2;
      iss >> green2;
      iss >> blue2;
      cout << "val = " << dataVal2 << "   shades = " << numShades2
           << "   red = "   << red2
           << "   green = " << green2 
           << "   blue = "  << blue2
           << endl;

      numSpecs++;

      if ( numShades1 == 1 ) {
         //
         // We don't need to interpolate, 
         // just put out this data value as a colormap entry
         //
         mapEntry( colorMap, dataVal1, dataVal2, red1, green1, blue1 );
         numMapEntries += numShades1;
      }
      else {
         // 
         // Make sure we don't exceed max number of color map entries
         //
         if ( numMapEntries + numShades1 > MAX_MAP_ENTRIES ) {
            numShades1 = MAX_MAP_ENTRIES - numMapEntries;
         }

         //
         // Do the linear color interpolation
         //
         dataFraction  = (dataVal2-dataVal1)/numShades1;
         redFraction   = (red2-red1)/numShades1;
         greenFraction = (green2-green1)/numShades1;
         blueFraction  = (blue2-blue1)/numShades1;
         dataVal = dataVal1; dataStep = dataVal + dataFraction;
         red = red1; green = green1; blue = blue1;

         for( i=0; i < numShades1; i++ ) {
            mapEntry( colorMap, dataVal, dataStep, red, green, blue );
            numMapEntries++;

            dataVal  += dataFraction;
            dataStep += dataFraction;
            red      += redFraction;
            green    += greenFraction;
            blue     += blueFraction;
         }
      }

   //
   // Check for degenerate case
   //
   if ( numSpecs == 1 ) {
      cerr << "ERROR: Just one color spec? You don't need this tool." << endl;
      return( -1 );
   }

   // 
   // Move line2 spec into line of action
   //
   dataVal1   = dataVal2;
   numShades1 = numShades2;
   red1       =  red2;
   green1     = green2;
   blue1      = blue2;
   
   }

   //
   // That's it -- let's go home
   //
   colorSpec.close();
   colorMap.close();
}

void mapEntry( ofstream &os, float min, float max,
               float red, float green, float blue )
{
   char        line[BUFSIZ];

   sprintf( line, "%.2f\t%.2f\tRGBi:%.4f/%.4f/%.4f\n",
                   min, max, red, green, blue );
   os << line; 
}

void printUsage()
{
   cerr << "Usage: argv[0] colorSpecFile [colorMapFile]" << endl;
}
