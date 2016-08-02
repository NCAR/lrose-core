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
#include <rdi/misc.h>
#include <time.h>

int 
unix_time (time_t *time,
	   int *y, int *mon, int *d, int *h, int *m, int *s)
{
    int tm;
    static unsigned short mon_tbl[12] =
    {0, 31, 59, 90, 120, 151, 181, 212,
     243, 273, 304, 334};
    int dd, dm, dy;

    if (*time == 0) {           /* find unix time */
        if (*mon < 1 || *mon > 12)
            return (1);
        dd = 365 * (*y - 1970) + mon_tbl[*mon - 1] + *d;
        dm = (*y - 1968) >> 2;
        if (*mon <= 2 && *y % 4 == 0)
            dm -= 1;
        *time = (((dd + dm - 1) * 24 + *h) * 60 + *m) * 60 + *s;;
        return (0);
    }
    else {                      /* find year, month, ... */
        tm = *time;
        *s = tm % 60;
        tm = tm / 60;
        *m = tm % 60;
        tm = tm / 60;
        *h = tm % 24;
        tm = tm / 24;
        dm = (tm + 671) / 1461;
        dd = tm - dm;
        dy = 1970 + dd / 365;
        dd = dd % 365;
        for (dm = 0; dm < 12; dm++)
            if (dd < (int)mon_tbl[dm])
                break;
        dd = dd - mon_tbl[dm - 1] + 1;
        if (dm == 3 && dd == 1 && dy % 4 == 0 && (tm + 671) % 1461 != 0) {
            dm = 2;
            dd = 29;
        }
        *y = dy;
        *mon = dm;
        *d = dd;
        return (0);
    }

}
