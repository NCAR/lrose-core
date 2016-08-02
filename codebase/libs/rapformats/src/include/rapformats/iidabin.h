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


////////////////////////////////////////////////////////////////////////


#ifndef __IIDA_BIN_H__
#define __IIDA_BIN_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <cmath>
using namespace std;


////////////////////////////////////////////////////////////////////////


static const int iida_nx = 151;
static const int iida_ny = 113;
static const int iida_nz =  37;


////////////////////////////////////////////////////////////////////////


class IidaBinaryFile {

   private:

      int fd;

      double threeD_data(int data_start, int x, int y, int z);

      double twoD_data(int data_start, int x, int y);

      IidaBinaryFile(const IidaBinaryFile &);
      IidaBinaryFile & operator=(const IidaBinaryFile &);

      void data_minmax_3D(int offset_bytes, double &datamin, double &datamax);

   public:

      IidaBinaryFile();
     ~IidaBinaryFile();

      int valid_time;

      int open(const char *filename);
      void close();

      void ice_minmax          (double &datamin, double &datamax);
      void ice_type_minmax     (double &datamin, double &datamax);
      void sld_minmax          (double &datamin, double &datamax);
      void height_m_minmax     (double &datamin, double &datamax);
      void temperature_minmax  (double &datamin, double &datamax);
      void rh_minmax           (double &datamin, double &datamax);
      void pirep_minmax        (double &datamin, double &datamax);

      double ice             (int x, int y, int z);
      double ice_type        (int x, int y, int z);
      double sld             (int x, int y, int z);
      double height_m        (int x, int y, int z);
      double temperature     (int x, int y, int z);
      double rh              (int x, int y, int z);
      double pirep           (int x, int y, int z);
      double new_ice         (int x, int y, int z);
      double slw             (int x, int y, int z);
      double vv              (int x, int y, int z);
      double inten           (int x, int y, int z);

      double cloudcount      (int x, int y);
      double cloudtoptemp    (int x, int y);
      double cloudtopheight  (int x, int y);
      double cloudbaseheight (int x, int y);
      double anyprcp         (int x, int y);
      double zprcp           (int x, int y);
      //////////////////////////////////////////////////////////////////
      /// new functions for pre 2002 data


      double ice_p02             (int x, int y, int z);
      double ice_type_p02        (int x, int y, int z);
      double sld_p02             (int x, int y, int z);
      double height_m_p02        (int x, int y, int z);
      double temperature_p02     (int x, int y, int z);
      double rh_p02              (int x, int y, int z);
      double pirep_p02           (int x, int y, int z);

      double cloudcount_p02      (int x, int y);
      double cloudtoptemp_p02    (int x, int y);
      double cloudtopheight_p02  (int x, int y);
      double cloudbaseheight_p02 (int x, int y);
      double anyprcp_p02         (int x, int y);
      double zprcp_p02           (int x, int y);


};


////////////////////////////////////////////////////////////////////////


#endif   //  _IIDA_BIN_H__


////////////////////////////////////////////////////////////////////////


