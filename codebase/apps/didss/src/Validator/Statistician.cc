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
#include <toolsa/file_io.h>
#include <toolsa/umisc.h>

#include "Params.hh"
#include "Statistician.hh"
using namespace std;

Statistician::Statistician(const string &Name){
  //
  // Set everything to 0 and make a local copy
  // of the name.
  //
  total_num_non = 0.0;
  total_num_fail = 0.0; 
  total_num_false = 0.0;
  total_num_success = 0.0;

  subtotal_num_non = 0.0;
  subtotal_num_fail = 0.0; 
  subtotal_num_false = 0.0;
  subtotal_num_success = 0.0;

  Num = 0; SubNum=0;

  _Name = Name;

}

/////////////////////////////////////////////////

void Statistician::Init(time_t Start, Params *P){
  //
  // Make a local copy of the start time and
  // open the output file.
  //
  char FileName[MAX_PATH_LEN], TimeStr[16];
  date_time_t BeginTime;

  BeginTime.unix_time = Start;
  uconvert_from_utime(&BeginTime);  
  sprintf(TimeStr,"%4d%02d%02d_%02d%02d%02d",
	  BeginTime.year, BeginTime.month, BeginTime.day,
	  BeginTime.hour, BeginTime.min, BeginTime.sec);


  fprintf(stderr,"Statistician started with %s\n",utimstr(Start));

  // Create the statistics directory

  if (ta_makedir_recurse(P->StatsDir) != 0)
  {
    fprintf(stderr, "Failed to create stats dir: %s\n", P->StatsDir);
    exit(-1);
  }
  
  if(P->DefineName)
    _Name = P->UserDefinedName;

  sprintf(FileName,"%s/Valid_%s_%s.dat",P->StatsDir,
	  TimeStr,_Name.c_str());
  sfp = fopen(FileName,"wt");
  if (sfp == NULL){
    fprintf(stderr,"Failed to create stats file %s\n",FileName);
    exit(-1);
  }

  sprintf(FileName,"%s/Valid_%s_%s.final",P->StatsDir,
	  TimeStr,_Name.c_str());
  fsfp = fopen(FileName,"wt");
  if (fsfp == NULL){
    fprintf(stderr,"Failed to create final stats file %s\n",FileName);
    exit(-1);
  }

  LastOutputTime = Start;
  StartTime = Start;
  _OutputInterval = P->OutputInterval;

}

////////////////////////////////////////////////
void Statistician::Accumulate(unsigned long num_non, 
			      unsigned long num_fail, 
			      unsigned long num_false, 
			      unsigned long num_success,
			      time_t TruthTime,
			      Params *P){

  if ((TruthTime - LastOutputTime) > P->OutputInterval){


    date_time_t T;
    T.unix_time = LastOutputTime + P->OutputInterval;
    uconvert_from_utime(&T);

    if (SubNum > 0){ // Only print out if we got something.
      fprintf(sfp,"%d\t%02d\t%02d\t",T.year,T.month,T.day);
      fprintf(sfp,"%02d\t%02d\t%02d\t%ld\t",
	      T.hour,T.min,T.sec,
	      LastOutputTime + P->OutputInterval - StartTime );

      fprintf(sfp,"%ld\t",SubNum);

      fprintf(sfp,"%g\t%g\t%g\t%g\t",
	      subtotal_num_non,subtotal_num_fail,
	      subtotal_num_false,subtotal_num_success);

      double pod,far,csi,hssn,hssd,hss;
      double pod_no,bias;
      double num_fcasts, num_truths;

      num_truths = subtotal_num_success + subtotal_num_fail;
      num_fcasts = subtotal_num_success + subtotal_num_false;
      bias = num_fcasts / num_truths;

      pod_no = double(subtotal_num_non) / 
	double(subtotal_num_non + subtotal_num_false);
    
      pod = subtotal_num_success / (subtotal_num_success + subtotal_num_fail);
      far = subtotal_num_false / (subtotal_num_false + subtotal_num_success);
      csi = subtotal_num_success / (subtotal_num_false + 
				    subtotal_num_fail + subtotal_num_success);

      hssn=2.0*(subtotal_num_success*subtotal_num_non-subtotal_num_fail*subtotal_num_false);

      hssd= subtotal_num_fail * subtotal_num_fail;
      hssd= hssd + subtotal_num_false * subtotal_num_false;
      hssd= hssd + 2.0*subtotal_num_success*subtotal_num_non;
      hssd= hssd + 
	(subtotal_num_fail + subtotal_num_false)*(subtotal_num_success + subtotal_num_non);

      hss = hssn / hssd;


      fprintf(sfp,"%g\t%g\t%g\t%g\t",
	      NoNaN(pod),NoNaN(far),NoNaN(csi),NoNaN(hss));

      fprintf(sfp,"%g\t%g\n",NoNaN(pod_no),NoNaN(bias));

      fflush(sfp);
    } // End of only print out if we got something.

    do { // skip empty intervals.
      LastOutputTime = LastOutputTime + P->OutputInterval;
    }while ((TruthTime - LastOutputTime) > P->OutputInterval);

    subtotal_num_non = 0.0;
    subtotal_num_fail = 0.0; 
    subtotal_num_false = 0.0;
    subtotal_num_success = 0.0;
    SubNum=0;

  }


  total_num_non += num_non;
  total_num_fail += num_fail; 
  total_num_false += num_false;
  total_num_success += num_success;

  subtotal_num_non += num_non;
  subtotal_num_fail += num_fail; 
  subtotal_num_false += num_false;
  subtotal_num_success += num_success;
  SubNum++; Num++;


}

////////////////////////////////////////////////

Statistician::~Statistician(){


  date_time_t T;
  T.unix_time = LastOutputTime + _OutputInterval;
  uconvert_from_utime(&T);

  if (SubNum > 0){ // Only print out if we got something.
    fprintf(sfp,"%d\t%02d\t%02d\t",T.year,T.month,T.day);
    fprintf(sfp,"%02d\t%02d\t%02d\t%ld\t",
	    T.hour,T.min,T.sec,
	    LastOutputTime + _OutputInterval - StartTime );

    fprintf(sfp,"%ld\t",SubNum);

    fprintf(sfp,"%g\t%g\t%g\t%g\t",
	    subtotal_num_non,subtotal_num_fail,
	    subtotal_num_false,subtotal_num_success);

    double pod,far,csi,hssn,hssd,hss;
    double pod_no,bias;
    double num_fcasts, num_truths;

    num_truths = subtotal_num_success + subtotal_num_fail;
    num_fcasts = subtotal_num_success + subtotal_num_false;
    bias = num_fcasts / num_truths;

    pod_no = double(subtotal_num_non) / 
      double(subtotal_num_non + subtotal_num_false);
    
    pod = subtotal_num_success / (subtotal_num_success + subtotal_num_fail);
    far = subtotal_num_false / (subtotal_num_false + subtotal_num_success);
    csi = subtotal_num_success / (subtotal_num_false + 
				  subtotal_num_fail + subtotal_num_success);

    hssn=2.0*(subtotal_num_success*subtotal_num_non-subtotal_num_fail*subtotal_num_false);

    hssd= subtotal_num_fail * subtotal_num_fail;
    hssd= hssd + subtotal_num_false * subtotal_num_false;
    hssd= hssd + 2.0*subtotal_num_success*subtotal_num_non;
    hssd= hssd + 
      (subtotal_num_fail + subtotal_num_false)*(subtotal_num_success + subtotal_num_non);

    hss = hssn / hssd;


    fprintf(sfp,"%g\t%g\t%g\t%g\t",
	    NoNaN(pod),NoNaN(far),NoNaN(csi),NoNaN(hss));

    fprintf(sfp,"%g\t%g\n",NoNaN(pod_no),NoNaN(bias));

    fflush(sfp);
  } // End of only print out if we got something.


  /////////
  fclose(sfp);

  fprintf(fsfp,"\nFinal statistics for %s :\n\n",_Name.c_str());

  fprintf(fsfp,"%s Total non-events : %g\n",_Name.c_str(),total_num_non);
  fprintf(fsfp,"%s Total failures : %g\n",_Name.c_str(),total_num_fail);
  fprintf(fsfp,"%s Total false alarms : %g\n",_Name.c_str(),total_num_false);
  fprintf(fsfp,"%s Total successes : %g\n\n",_Name.c_str(),total_num_success);
  fprintf(fsfp,"%s Total number of forecast files : %ld\n",_Name.c_str(),Num);

  double pod,far,csi,hssn,hssd,hss;

  pod = total_num_success / (total_num_success + total_num_fail);
  far = total_num_false / (total_num_false + total_num_success);
  csi = total_num_success / (total_num_false + 
			     total_num_fail + total_num_success);

  hssn=2.0*(total_num_success*total_num_non - total_num_fail*total_num_false);

  hssd= total_num_fail * total_num_fail;
  hssd= hssd + total_num_false * total_num_false;
  hssd= hssd + 2.0*total_num_success*total_num_non;
  hssd= hssd + 
    (total_num_fail + total_num_false)*(total_num_success + total_num_non);

  hss = hssn / hssd;

  double pod_no,bias;
  double num_fcasts, num_truths;

  num_truths = total_num_success + total_num_fail;
  num_fcasts = total_num_success + total_num_false;
  bias = num_fcasts / num_truths;

  pod_no = double(total_num_non) / 
    double(total_num_non + total_num_false);

  fprintf(fsfp,"%s POD : %g\n",_Name.c_str(),NoNaN(pod));
  fprintf(fsfp,"%s FAR : %g\n",_Name.c_str(),NoNaN(far));
  fprintf(fsfp,"%s CSI : %g\n",_Name.c_str(),NoNaN(csi));
  fprintf(fsfp,"%s HSS : %g\n",_Name.c_str(),NoNaN(hss));
  fprintf(fsfp,"%s POD_NO : %g\n",_Name.c_str(),NoNaN(pod_no));
  fprintf(fsfp,"%s BIAS : %g\n",_Name.c_str(),NoNaN(bias));
  fprintf(fsfp,"%s TRUTHS : %g\n",_Name.c_str(),NoNaN(num_truths));
  fprintf(fsfp,"%s FORECASTS : %g\n",_Name.c_str(),NoNaN(num_fcasts));

  fclose(fsfp);

}

/////////////////////////////////////////////////

double Statistician::NoNaN(double val){

  if (std::isnan(val)){
    return -1000.0;
  }
  return val;
}
































