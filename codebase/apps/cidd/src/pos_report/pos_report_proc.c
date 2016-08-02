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
/*************************************************************************
 * POS_REPORT_PROC.C - Notify and event callback functions
 */

#include "pos_report.h"


/*************************************************************************
* Notify callback function for `origin_lst'.
 */
int
origin_select_proc(item, string, client_data, op, event, row)
        Panel_item      item;
        char            *string;
        Xv_opaque       client_data;
        Panel_list_op   op;
        Event           *event;
        int             row;
{
	char label_string[128];
	 
        switch(op) {
        case PANEL_LIST_OP_DESELECT:
        break;

        case PANEL_LIST_OP_SELECT:
            gd.selected_origin = row;
	    sprintf(label_string,"From %s:",gd.origin[gd.selected_origin].label);
            xv_set(gd.gui_objects->label_msg,PANEL_LABEL_STRING,label_string,NULL);
            report_relative_position();
        break;

        case PANEL_LIST_OP_VALIDATE:
        break;

        case PANEL_LIST_OP_DELETE:
        break;
        }

        return XV_OK;
}

/*************************************************************************
 * REPORT_RELATIVE_POSITION: Calculate the relative position and
 *     report it by setting panel messages
 *
 */
 
report_relative_position(void)
{
    double lat,lon;
    double x_km,y_km;
    double radius,theta;
    double new_angle;
    char  xy_string[128];
    char  rad_string[128];

    lat = gd.coord_expt->pointer_lat;
    lon = gd.coord_expt->pointer_lon;

    PJGLatLon2DxDy(gd.origin[gd.selected_origin].lat,
		   gd.origin[gd.selected_origin].lon,
		   lat,lon,
		   &x_km,&y_km);

    PJGLatLon2RTheta(gd.origin[gd.selected_origin].lat,
                         gd.origin[gd.selected_origin].lon,
                         lat,lon,
                         &radius,&theta);

    new_angle = theta + gd.origin[gd.selected_origin].angle;
    if(new_angle < 0.0) new_angle += 360.0;
    new_angle += 0.5;	/* So display rounds to nearest integer */

     
    sprintf(xy_string,"%.1fE  %.1fN km",x_km,y_km);
    xv_set(gd.gui_objects->km_msg,PANEL_LABEL_STRING,xy_string,NULL);

    if(gd.origin[gd.selected_origin].angle != 0.0) { /* Is an FAA origin - Use NM */
        sprintf(rad_string,"%.0f deg %.2f nm",new_angle,(radius * NM_KM));
    } else {	/* Is a Meterlogical origin - USE KM */
        sprintf(rad_string,"%.0f deg %.2f km",new_angle,radius);
    }
    xv_set(gd.gui_objects->radial_msg,PANEL_LABEL_STRING,rad_string,NULL);

    return 0;
}

/* SCCS info
 *   %W% %D% %T%
 *   %F% %E% %U%
 *
 * RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/07 18:28:26 $
 *   $Id: pos_report_proc.c,v 1.3 2016/03/07 18:28:26 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 *
 *   $Log: pos_report_proc.c,v $
 *   Revision 1.3  2016/03/07 18:28:26  dixon
 *   Changing copyright/license to BSD
 *
 *   Revision 1.2  1995/09/22 19:22:00  dixon
 *   Changed to rap_make includes
 *
 * Revision 1.1  1995/07/11  20:06:01  fhage
 * New placement in RPA's CVS Tree
 *
 * Revision 1.3  1994/01/27  19:22:54  fhage
 * dded switch for units; Use NM for non-true north origins (FAA ones)
 *
 * Revision 1.2  94/01/27  17:25:21  fhage
 * Removed redundant RCS_Id string defn, fixed name clash for variable.
 * 
 * Revision 1.1.1.1  1994/01/27  16:39:20  fhage
 * INitial Checkin
 *
 */

#ifndef LINT
static char RCS_id[] = "$Id: pos_report_proc.c,v 1.3 2016/03/07 18:28:26 dixon Exp $";
static char SCCS_id[] = "%W% %D% %T%";
#endif /* not LINT */
