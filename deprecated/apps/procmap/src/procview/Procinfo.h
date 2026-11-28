/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
//////////////////////////////////////////////////////////
// Procinfo.h
//
// Procmap info class - Singleton
//
//////////////////////////////////////////////////////////

#ifndef PROCINFO_H
#define PROCINFO_H

#include <Xm/Xm.h>
#include <Xm/Text.h>
#include <toolsa/pmu.h>

#define TMPBUFLEN 256
#define TEXTBUFLEN 4096

class Args;

class Procinfo {
  
public:
    
  static Procinfo *Inst ();
  static void queryMapper();
  void registerTextWidget(Widget text);
  void unregisterTextWidget();
  
protected:
  
  Procinfo ();

private:

  static Procinfo *_instance = NULL;

  PROCMAP_info_t *_procs;
  int _nProcs;
  long _upTime;
  Widget _textWidget;
  char _tmpBuf[TMPBUFLEN];
  char _textBuf[TEXTBUFLEN];

  void initText();
  void appendText();
  void putText2Widget();

  void getProcInfo();
  void Procinfo::displayProcs();
  void Procinfo::printData(PROCMAP_info_t *dinfo, time_t now);
  void Procinfo::printHeader();
  char *Procinfo::pidString(int pid);
  char *Procinfo::secsStr(time_t now, si32 refTime);
  char *Procinfo::stripHost(char *hostname);
  char *Procinfo::tdiffStr(long tdiff);

  static int Procinfo::sortInfo(const void *p1, const void *p2);

};

#endif









