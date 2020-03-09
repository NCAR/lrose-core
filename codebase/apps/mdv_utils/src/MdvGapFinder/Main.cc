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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/04 02:22:11 $
//   $Id: Main.cc,v 1.4 2016/03/04 02:22:11 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Main.cc: MergeFcsts main routine
 *
 * RAP, NCAR, Boulder CO
 *
 *********************************************************************/

#include <cstdio>
#include "Params.hh"
#include <niwot_util/CmdArgs.hh>
#include <niwot_util/UrlWatcher.hh>
#include <toolsa/DateTime.hh>
using namespace std;

/*********************************************************************
 * main()
 */

int main(int argc, char **argv)
{
  CmdArgs C(argc, argv);

  Params P;
  char *params_path = (char *) "unknown";
  tdrp_override_t override;
  TDRP_init_override(&override);
  if (P.loadFromArgs(argc, argv, override.list, &params_path))
  {
    cerr << "Problem with TDRP parameters in file: " << params_path << endl;
    exit(-1);
  }
  TDRP_free_override(&override);

  if (!C.is_archive())
  {
    printf("ERR need archivemode (-interval yyyymmddhhmmss yyyymmddhhmmss)\n");
    exit(-1);
  }

  UrlWatcher U(C, P.url);

  FILE *fp = NULL;
  if (strlen(P.output_file_path) > 0)
  {
    string fname = P.output_file_path;
    fname += "/";
    fname += C.get_start_str_plain();
    fname += "_";
    fname += C.get_end_str_plain();
    fname += "_gaps.txt";
    fp = fopen(fname.c_str(), "w");
  }

  vector<time_t> t;
  U.getArchiveList(t);
  if (t.size() == 0)
  {
    printf("NO DATA found in range specified\n");
    exit(-1);
  }
  vector<time_t>::iterator i;
  time_t t0;
  i = t.begin();
  t0 = *i;
  int delta, max_allowed_delta;
  int mindelta, maxdelta, ndelta=0, avedelta=0;
  max_allowed_delta = (int)(P.max_gap_minutes*60.0);
  bool first = true;
  for (i++; i!= t.end(); i++)
  {
    delta = *i - t0;
    if (delta > max_allowed_delta)
    {
      if (P.output_to_std)
	printf("Gap from %s  to %s\n", DateTime::strn(t0).c_str(),
	       DateTime::strn(*i).c_str());
      if (fp != NULL)
	fprintf(fp, "Gap from %s  to %s\n", DateTime::strn(t0).c_str(),
	       DateTime::strn(*i).c_str());
    }
    if(first)
    {
      first = false;
      mindelta = maxdelta = delta;
    }
    else
    {
      if (delta < mindelta)
	mindelta = delta;
      if (delta > maxdelta)
	maxdelta = delta;
    }

    avedelta += delta;
    ndelta += 1;
    t0 = *i;
  }

  if (ndelta > 0)
  {
    double d_avedelta = (double)avedelta/(double)ndelta;
    if (P.output_to_std)
      printf("Num:%d  Delta seconds min:%d  max:%d  ave:%.2f\n", 
	     ndelta, mindelta, maxdelta, d_avedelta);
    if (fp != NULL)
    {
      fprintf(fp, "Num:%d  Delta seconds min:%d  max:%d  ave:%.2f\n", 
	      ndelta, mindelta, maxdelta, d_avedelta);
      fclose(fp);
    }      
  }
  else
  {
    if (P.output_to_std)
      printf("No MDV data in range\n");
    if (fp != NULL)
    {
      fprintf(fp, " No MDV data in range\n");
      fclose(fp);
    }      
  }

}
