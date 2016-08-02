/*
    NASA/TRMM, Code 910.1.
    This is the TRMM Office Radar Software Library.
    Copyright (C) 1996, 1997
            John H. Merritt
            Space Applications Corporation
            Vienna, Virginia

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include <trmm_rsl/rsl.h> 
/*********************************************************************/
/*                                                                   */
/*                      RSL_reverse_sweep_order                      */
/*                                                                   */
/*********************************************************************/
Volume *RSL_reverse_sweep_order(Volume *v)
{
/*
 *  Reverse the order of the sweeps within the volume.
 *  This routine modifies the argument and does not allocate memory.
 */

  int i, j;
  Sweep *s_tmp;

  if (v == NULL) return v;
  /* Yes, the follwing is integer divide by 2. */
  for (i=0, j=v->h.nsweeps-1; i<v->h.nsweeps/2; i++, j--) {
        s_tmp = v->sweep[i];
        v->sweep[i] = v->sweep[j];
        v->sweep[j] = s_tmp;
  }

  return v;
}
