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
// ##########################################################################
//
// i2readvar_cart3d.cc:  function to read bi format file nto 1D short array.
//
// ##########################################################################
//
//      Author: Jian Zhang (CIMMS/NSSL), Wenwu Xia (CIMMS/NSSL)
//      July 20, 2000
//
//      Modification History:
//      Wenwu Xia (CIMMS/NSSL)
//      Febrary, 10, 2001
//      Modified to read gzipped binary file.
//
// ##########################################################################




#include <fstream>
#include <iostream>
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <zlib.h>

#include "func_prototype.h"
#include "gage_config_pars.h"

using namespace std;

//#######################################################################
void i2readvar_cart3d(const string& vfname,
                     int &map_scale,
                     float &nw_lon,  float &nw_lat,
                     int &xy_scale, int &dxy_scale, int &z_scale,
                     char *varname, char *varunit,
                     int &nradars,  char radarnam[][5],
                     short int *i2var,int &var_scale,
                     int &imissing, inputpars &mp,  int &istatus, int &i_bb_mode)
{
//#######################################################################
//
//     Misc. local variables:
//
//#######################################################################
//

      int temp;
      int nx1, ny1, nz1;
//
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//
//     Beginning of exeCUTAble code...
//
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//
      istatus = -1;
      char projection[5];
      int proj=4;
    
      char open_mode[3];

      gzFile   fp_gzip;

      sprintf(open_mode,"%s","rb");

      open_mode[2] = '\0';

      if ( (fp_gzip = gzopen(vfname.c_str(),open_mode) ) == (gzFile) NULL )
      {
         cout<<"++ERROR open "<<vfname<<endl;
         exit(0);
      }

      for (int i=0; i<6; i++) {gzread( fp_gzip, &temp,sizeof(int)) ;}

      gzread( fp_gzip,&nx1,sizeof(int)) ;
      gzread( fp_gzip, &ny1,sizeof(int)) ;
      gzread( fp_gzip, &nz1,sizeof(int)) ;

      if ( (nx1!=mp.nx) || (ny1!= mp.ny) )  {
        cout<<endl<<"++ERROR++ inconsistent dimensions."<<endl;
        cout<<"input were:"<<mp.nx<<"  "<<mp.ny<<endl;
        cout<<"read were:"<<nx1<<"  "<<ny1<<endl;
        cout<<"Aborting from i2readvar_cart3d."<<endl<<endl;
        exit(0);
      }


      gzread(fp_gzip,&projection,4*sizeof(char));

      projection[4] = '\0';
      if (strncmp(projection, "    ", 4)==0 ) proj=0; 
      if (strncmp(projection, "PS  ", 4)==0 ) proj=1;
      if (strncmp(projection, "LAMB", 4)==0 ) proj=2; 
      if (strncmp(projection, "MERC", 4)==0 ) proj=3; 
      if (strncmp(projection, "LL  ", 4)==0 ) proj=4; 

      if(proj!=mp.mapproj) {
        cout<<endl<<"++ERROR++ inconsistent map projections."<<endl;
        cout<<"input were:"<<mp.mapproj<<endl;
        cout<<"read were:"<<proj<<endl;
        cout<<"Aborting from i2readvar_cart3d."<<endl<<endl;
        exit(0);
      }

      gzread(fp_gzip,&map_scale,sizeof(int));
      for(int i = 0; i < 3; i++) {gzread(fp_gzip,&temp,sizeof(int));}
      int nw_lon1, nw_lat1;
      gzread(fp_gzip,&nw_lon1,sizeof(int));
      nw_lon = (float)nw_lon1;
      gzread(fp_gzip,&nw_lat1,sizeof(int));
      nw_lat = (float)nw_lat1;

      gzread(fp_gzip,&xy_scale,sizeof(int));

      nw_lon = nw_lon/float(map_scale);
      nw_lat = nw_lat/float(map_scale);
    
      gzread(fp_gzip,&temp,sizeof(int));
      float dx1 = (float)(temp);
      gzread(fp_gzip,&temp,sizeof(int));
      float dy1 = (float)(temp);

      gzread(fp_gzip,&dxy_scale,sizeof(int));
      dx1 = dx1/float(dxy_scale);
      dy1 = dy1/float(dxy_scale);


      if ( (fabs(dx1-mp.dx)>1.0e-6) || (fabs(dy1- mp.dy)>1.0e-6) )  {
        cout<<endl<<"++ERROR++ inconsistent dimensions."<<endl;
        cout<<"input were:"<<mp.dx<<"  "<<mp.dy<<endl;
        cout<<"read were:"<<dx1<<"  "<<dy1<<endl;
        cout<<"Aborting from i2readvar_cart3d."<<endl<<endl;
        exit(0);
      }

      float zhgt[nz1];
      
      for(int k=0; k<nz1; k++){
        gzread(fp_gzip,&temp,sizeof(int));
          zhgt[k] = (float)temp;
      }

      gzread(fp_gzip,&temp,sizeof(int)); //z_scale

      gzread(fp_gzip,&i_bb_mode,sizeof(int));

      for(int j = 0; j < 9; j++){
        gzread(fp_gzip,&temp,sizeof(int)); //for future use
      }
      gzread(fp_gzip,varname,20*sizeof(char));
      varname[19]='\0';

      gzread(fp_gzip,varunit,6*sizeof(char));
      varunit[5]='\0';

      gzread(fp_gzip,&var_scale,sizeof(int));

      gzread(fp_gzip,&imissing,sizeof(int));

      gzread(fp_gzip,&nradars,sizeof(int));

      for(int i=0;i<nradars;i++){
        gzread(fp_gzip,radarnam[i],4*sizeof(char));
        radarnam[i][4]='\0';
      }


      gzread(fp_gzip,i2var,nx1*ny1*nz1*sizeof(short int));

        
      gzclose( fp_gzip );


      z_scale = 1;
      istatus = 0;
}

