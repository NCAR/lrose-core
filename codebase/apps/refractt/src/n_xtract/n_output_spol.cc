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
// SUMMARY:      Output refractivity variables to a DORADE sweepfile 
// USAGE:        write_data_spol(char *inputFile, char *outputFile)      
//
// AUTHOR:       Joe VanAndel and Frederic Fabry
// ORG:          National Center for Atmospheric Research
// E-MAIL:       vanandel@ucar.edu
//
// ORIG-DATE:     5-Oct-99 at 09:41:01
// LAST-MOD:     8-April-2003 by Joe VanAndel
//
// DESCRIPTION:  read the specified input file and copy to an output file,
// adding refractivity (N) , delta refractivity (DELTA_N),
// SIGMA_N and SIGMA_DN variables 
//
// Output data  data is in the arrays:
// n_array_polar[NOutNumAzim*NOutNumRanges]
// dn_array_polar[NOutNumAzim*NOutNumRanges]
//
//
// for each input ray, we determine the matching Refractivity data
// (which has lower angular and azimuthal resolution)
// if "REFRACT_POSTPROC" is set, invoke the specified program/script to
// postprocess the output file

// DESCRIP-END.
//
#if __GNUC__ >= 3
#include <iostream>
using namespace std;
#else
#include <iostream.h>
#endif

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <sys/param.h>

#include <refractt/dd_sweepfiles.hh>
#include <refractt/dorade_includes.h>

#ifndef N_XTRACT_H
#include        <refractt/n_xtract.h>    /* Include & defines for this program */
#endif

#include <refractt/dd_utils.hh>
#define N_BAD_DATA -32768

#ifdef DUMP_NC
#include "n_util.h"
extern int Dump_nc;
char name_buf[300];
#endif

#include <refractt/DataInput.hh>  // SPOL sweepfile utility

#define ADD_TO_BUF(ptr, bufSize, objectPtr, objectSize) \
     memcpy(ptr, (char *)objectPtr, objectSize); \
     ptr += objectSize; bufSize += objectSize;

     
const unsigned int SZ_DD_BUF = (256*1024);

//char *newNames[] = {"DBZ", "VR","SW","NCP","NIQ","AIQ","DM", "N", "DELTA_N",
//                    "SIGMA_N", "SIGMA_DN"};
char *newNames[] = {"DBZ", "VR","VE", "VEL", "SW","NCP",
		    "NIQ", "NIQ_V", "AIQ", "AIQ_V", "DM", "N", "DELTA_N",
                    "SIGMA_N", "SIGMA_DN"};
int numNewNames = sizeof(newNames) / sizeof(char*);

//char *fieldNames[] = {"DBZ", "VR","SW","NCP","NIQ","AIQ","DM"};
char *fieldNames[] = {"DBZ", "VEL","SW","NCP","NIQ_V","AIQ_V","DM"};

int numFieldNames = sizeof(fieldNames) / sizeof(char*);

const int N_SCALE = 10;
const int N_OFFSET = 0;
const int DELTA_N_SCALE = 10;
const int DELTA_N_OFFSET = 0;

const int SIGMA_N_SCALE = 10;
const int SIGMA_N_OFFSET = 0;
const int SIGMA_DN_SCALE = 10;
const int SIGMA_DN_OFFSET = 0;


int write_data_spol(char *inputFile, char *outputDir)
{

        
    int ActualRangeBins;
    bool inSweep=false;
    int swpbeams = 0;
    char	tmp_str[80] ;
    int		 nr, ng;
    int 	numRays,numGates; 
    short *origSS, *origDSS,*orig_Sigma_N, *orig_Sigma_dN;
    
 
    
    char *dd_buf = new char [SZ_DD_BUF];
    char *dph = dd_buf;
    int hbu = 0;

    
    int bytesUsed = 0;
    int actualUsed = 0;

    int dataSize;
    int iaz;
    register short *ss;
    register short *dss;
    register short *sigma_N;
    register short *sigma_dN;
    
#ifdef DUMP_NC
    if (Dump_nc) {
	char *tp = strrchr(RealFileName, '/');
	/* get portion filename that follows 'xyz/swp.' */
	tp = strchr(tp, '.');
	
	sprintf(name_buf, "%s/%s.nc", OutputDir, tp+1);
	dump_T_info(target, "phase maps", name_buf);
    }
#endif    
    
    
     
    DFileInput dfi(inputFile);
    dd_mapper outMapr;
    dd_mapper *imap = dfi.get_beam();
    dd_sweepfile dd_sweep(imap->rays_in_sweep());
#ifdef USE_COMPRESSION
    dd_sweep.set_typeof_compression(HRD_COMPRESSION);
#else
    dd_sweep.set_typeof_compression(NO_COMPRESSION);
#endif
    
    if (!dfi.OK) {
	sprintf( tmp_str, "Cannot read sweep file" ) ;
	error_out( tmp_str, FALSE ) ;
	return( FALSE ) ;
    }
    for (;;) {
        if (inSweep && (!imap || imap->new_sweep())) {
            inSweep = false;
            break;
        }
        bytesUsed = 0;
        
        if (!imap) {
            sprintf( tmp_str, "Can not access sweepfile" ) ;
            error_out( tmp_str, FALSE ) ;
            return( FALSE ) ;
        }
        // first ray processing
        if (!inSweep)
        {
            inSweep = true;
            numRays = imap->rays_in_sweep();
            numGates = imap->number_of_cells();
            // copy the original sweep's DORADE structures.  Side effect is
            // to allocate space for each variables data,  needed by
            // dd_mapper::copy_data()
            outMapr.ray_constructor(imap, dd_buf,newNames,numNewNames);

            // locate the pointer to the raw data field for 'N'
            // in the output sweepfile, and fill it in.
            origSS = (short *) outMapr.raw_data_ptr( outMapr.field_index_num( "N" ) );
        
            // and for "DELTA_N" field
            origDSS = (short *) outMapr.raw_data_ptr(
                outMapr.field_index_num( "DELTA_N" ) );

            orig_Sigma_N = (short *) outMapr.raw_data_ptr(
                outMapr.field_index_num( "SIGMA_N" ) );

            orig_Sigma_dN = (short *) outMapr.raw_data_ptr(
                outMapr.field_index_num( "SIGMA_DN" ) );

            if (dd_sweep.begin_sweepfile(&outMapr, outputDir, "", 0)) {
                return( FALSE ) ;
            }
            
        }
        // end if first ray processing 



        ss = origSS;    // restore values when starting new ray
        dss = origDSS;
        sigma_N = orig_Sigma_N;
        sigma_dN = orig_Sigma_dN;
        ActualRangeBins = NOutNumRanges;
        int realMaxGate = (numGates < NumRangeBins) ? numGates: NumRangeBins;



        // output the actual data 
        for (int g = 0; g < realMaxGate; ++g,++ss,++dss,++sigma_N,++sigma_dN){
            iaz = (int)floor(imap->azimuth()+.5);
            nr = iaz * NOutNumAzim / NumAzim ;
            // assume 150 meter gates for now
            ng = (g * NOutNumRanges / NumRangeBins);

            float orig = n_array_polar[nr*ActualRangeBins +ng];
            *ss =  (orig == INVALID) ? N_BAD_DATA :
                        (short)DD_SCALE2(orig, N_SCALE, N_OFFSET);

            float dorig = dn_array_polar[nr*ActualRangeBins +ng];
            *dss =  (dorig == INVALID) ? N_BAD_DATA :
                        (short)DD_SCALE2(dorig, DELTA_N_SCALE, DELTA_N_OFFSET);

            float sigman_orig = n_er_array_polar[nr*ActualRangeBins +ng];  
            *sigma_N =  (sigman_orig == INVALID) ? N_BAD_DATA :
                        (short)DD_SCALE2(sigman_orig, SIGMA_N_SCALE, SIGMA_N_OFFSET);
            float sigma_dn_orig = dn_er_array_polar[nr*ActualRangeBins +ng];  
            *sigma_dN =  (sigma_dn_orig == INVALID) ? N_BAD_DATA :
                        (short)DD_SCALE2(sigma_dn_orig, SIGMA_DN_SCALE,
                                         SIGMA_DN_OFFSET);
        }
        for (int g = NumRangeBins; g < numGates; ++g,++ss,++dss,
                 ++sigma_N,++sigma_dN)
        {
            *ss =   N_BAD_DATA;
            *dss =   N_BAD_DATA;
            *sigma_N = N_BAD_DATA;
            *sigma_dN = N_BAD_DATA;
        }


        // copy original data with newly generated data for this beam
        outMapr.copy_data(imap, fieldNames, numFieldNames);


        dd_sweep.add_to_sweepfile();

        swpbeams++;
        imap = dfi.get_beam();  // check for next time
    }// done with all rays
    char *fname = dd_sweep.end_sweepfile(0);
    if (fname) {
	    if( DebugLevel != QUIET )
                cerr << "write_data_spol: wrote output to " << fname << endl;
	    // if we wrote the sweepfile, post process if necessary
	    char *postProc = getenv("REFRACT_POSTPROC");
	    if (postProc) {
		char cmdBuf[MAXPATHLEN];
		sprintf(cmdBuf, "%s %s", postProc, fname);
		if (DebugLevel != QUIET)
		    fprintf(stderr, "n_xtract: post proc cmd='%s'\n", cmdBuf);
		system(cmdBuf);
	    }
    } else {
            cerr << "write_data_spol: ERROR writing output" << endl;
    }
    delete [] dd_buf;

    return(0);

}


