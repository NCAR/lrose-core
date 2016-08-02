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
// Program sst2mdv reads a Sea Surface Temperature data file
// and writes to an MDV file
//
// Author: Curtis Caravone
// Date:   6/20/2003
//

#include "SSTData.hh"
#include "SST2Mdv.hh"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s SST_input_file MDV_output_file\n", argv[0]);
        return 2;
    }

    printf("Reading input file %s...", argv[1]);

    // Read input file
    FILE *f = NULL;
    f = fopen(argv[1], "r");
    if (f == NULL) {
        fprintf(stderr, "Error opening file %s\n", argv[1]);
        return 1;
    }
    SSTData data(f);
    fclose(f);

    printf("done\n");

    printf("Creating MDV object...");

    // Create output MDV object
    Mdvx mdvFile;

    // Write SST data to MDV object (give it today's date)
    sst2mdv(data, mdvFile, time(NULL), time(NULL));

    printf("done\n");

    printf("Writing output file %s...", argv[2]);

    // Write MDV file
    if (mdvFile.writeToPath(argv[2]) != 0) {
        printf("\nERROR: %s\n", mdvFile.getErrStr().c_str());
    } else {
        printf("done\n");
    }
}

