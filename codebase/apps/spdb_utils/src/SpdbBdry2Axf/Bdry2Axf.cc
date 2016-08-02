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
/////////////////////////////////////////////////////////////
//
// Bdry2Axf.cc - class to convert SPDB boundaries from
// colide into meta data for the Olympics 2000 project.
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2000
//
/////////////////////////////////////////////////////////////


#include "Bdry2Axf.hh"
using namespace std;


// Constructor - makes local copy of parameters, opens output file.
Bdry2Axf::Bdry2Axf(Params *P, time_t ThisTime, int NumAtThisTime,
		   int *LeadTimes){

  _params = *P;
  _Time = ThisTime;
  _Num = NumAtThisTime; // Number to add.
  num = 0; // Number done so far.

  date_time_t T;
  T.unix_time =  ThisTime;
  uconvert_from_utime( &T );

  sprintf(TimeStr,"%4d%02d%02d%02d%02d%02d",
          T.year, T.month, T.day, T.hour, T.min, T.sec);

  sprintf(OutFile,"%s/ANC_BDRY_%s.axf",
          P->OutDir,TimeStr);     

  ofp = fopen(OutFile,"wt");
  if (ofp == NULL){
    cerr << "Failed to create " << OutFile << endl;
    exit(-1);
  }

  //
  // Put a header on the file.
  //
  fprintf(ofp,"[DESCRIPTION]\nsystem[30]=\"anc\"\n");
  fprintf(ofp,"product[30]=\"ANC boundary\"\n");
  fprintf(ofp,"radar[30]=\"Kurnell\"\n");
  fprintf(ofp,"aifstime[30]=\"%s\"\n",TimeStr);
  fprintf(ofp,"[$]\n\n");   



  fprintf(ofp,"[LINE]\nline[30]\n");
 
  for (int l=0; l < _Num; l++){
    fprintf(ofp,"\"GUST_FRONT_%d_%02d\"\n",
	    l,LeadTimes[l]/60);
  }
  fprintf(ofp,"[$]\n\n");



}

//////////////////////////////////////////////////////
//
// Add a chunk to the output file.
//
void Bdry2Axf::Add(BDRY_product_t bdryProduct, int lineType,
		   int LeadTime){


 int extrap;
  if (lineType != BDRY_LINE_TYPE_EXTRAPOLATED){
    extrap = 0;
  } else {
    extrap = 1;
  }


  fprintf(ofp,"[GUST_FRONT_%d_%02d]\n",
	  num, LeadTime / 60);

  fprintf(ofp,"lat, lon, u, v, extrap, num\n");         

  for (int k = 0; k < bdryProduct.num_polylines; k++)
    {

      BDRY_point_t *points = bdryProduct.polylines[k].points;

      for (int q=0; q < bdryProduct.polylines[k].num_pts; q++){

        fprintf(ofp,"%g,\t%g,\t%g,\t%g,\t%d,\t%d\n",
                points[q].lat, points[q].lon,
                points[q].u_comp, points[q].v_comp,
                extrap, num);

      }
    }

  fprintf(ofp,"[$]\n\n");     
  num ++;

}

//////////////////////////////////////////////////////
//
// Close the output file, do a shuffle of latest data files.
void Bdry2Axf::Close(){

  fclose(ofp);
  //
  // Do a sequence of file moves to get ready for the latest output.
  //
  char command[2*MAX_PATH_LEN+10];
  for(int k=_params.NumLatestFiles; k > 1  ;k--){
    char FNameA[MAX_PATH_LEN], FNameB[MAX_PATH_LEN];

    sprintf(FNameA,"%s_%d.axf",_params.LatestFileName,k-1);
    sprintf(FNameB,"%s_%d.axf",_params.LatestFileName,k);
    sprintf(command,"\\mv  %s %s",FNameA, FNameB);

    system(command);

  }                

  //
  // Copy the file into the latest output file name
  // specified. This is done with calls to system which
  // is not ideal.
  //
  char TempFileName[MAX_PATH_LEN];

  sprintf(TempFileName,"%s.tmp",_params.LatestFileName);
  //
  // Copy the file across to a temporary file and then
  // move that file into place as the latest file (a move
  // being faster than a copy, this may avoid file lock issues).
  //
  // This is done with system calls, which is not great, but not
  // (I think) awful either.
  //
  sprintf(command,"\\cp %s %s", OutFile, TempFileName);  
  if (system(command)){
    cerr << "Command " << command << " failed." << endl;
    exit(-1);
  }

  char Target[MAX_PATH_LEN];
  sprintf(Target,"%s_%d.axf",_params.LatestFileName,1);

 sprintf(command,"\\mv %s %s", TempFileName, Target);
  if (system(command)){
    cerr << "Command " << command << " failed." << endl;
    exit(-1);
  }   
}


//////////////////////////////////////////////////////
//
// Destructor - non-event.
Bdry2Axf::~Bdry2Axf(){

}




