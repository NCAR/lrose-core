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
/////////////////////////////////////////////////////////////
// DataSet.h
//
// DataSet object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// February 1998
//
///////////////////////////////////////////////////////////////

#ifndef DataSet_H
#define DataSet_H

#include "Params.hh"

#define DATASET_LABEL_MAX 64

typedef struct {
  double time;
  double dur;
} time_dur_t;

class DataSet {
  
public:

  // constructor

  DataSet (const char *prog_name,
           const Params &params);

  // destructor
  
  ~DataSet();

  // reset to start of data set

  void Reset();

  // reset to given point in data

  void Reset(int pos);

  // get size of data set

  int Size();

  // get specified entry

  int Get(int pos, double *time_p, double *dur_p);

  // get next entry

  int Next(double *time_p, double *dur_p);

  // get next active period

  int GetActivePeriod(int start_pos, int *end_pos_p);

  // data members

  int OK;

protected:
  
private:

  char *_progName;
  const Params &_params;
  
  char _timeLabel[DATASET_LABEL_MAX];
  char _durLabel[DATASET_LABEL_MAX];
  char _condLabel[DATASET_LABEL_MAX];

  int _timePos;
  int _durPos;
  int _condPos;
  
  int _nData;
  int _dataPos;

  time_dur_t *_inputData;

  double _timeMin, _timeMax;
  double _durMin, _durMax;

  int _readLabels();
  int _readData();

};

#endif
