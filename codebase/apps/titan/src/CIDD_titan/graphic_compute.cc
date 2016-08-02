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
/*************************************************************************
 * GRAPHIC_COMPUTE.C : Routines that are useful for graphics operations 
 *
 *
 * For the Configurable Interactive Data Display (CIDD)
 * Frank Hage   July 1991 NCAR, Research Applications Program
 */

#define GRAPHIC_COMPUTE 1
#include "cidd.h"

/*************************************************************************
 * COMPUTE_TICK_INTERVAL: Return the tick interval given a range
 *        Range Assumed to be > 1.0 && < 10000.0
 */

double compute_tick_interval(double range)
{
    double    arange = fabs(range);

    if(arange < 1.5) return (0.1);
    if(arange < 3.0) return (0.2);
    if(arange < 7.5) return (0.5);
    if(arange < 15.0) return (1.0);
    if(arange < 30.0) return (2.0);
    if(arange < 75.0) return (5.0);
    if(arange < 150.0) return (10.0);
    if(arange < 300.0) return (20.0);
    if(arange <= 360.0) return (30.0);
    if(arange < 750.0) return (50.0);
    if(arange < 1500.0) return (100.0);
    if(arange < 3000.0) return (200.0);
    if(arange < 7500.0) return (500.0);
    if(arange < 10000.0) return (1000.0);
    if(arange < 25000.0) return (2000.0);
    return(5000.0);
}

/*************************************************************************
 * COMPUTE_CONT_INTERVAL: Return a nice even contour interval given a
 * computed one.
 */

double compute_cont_interval(double value)
{
    if(value < .01) return (0.01);
    if(value < .03) return (0.02);
    if(value < .075) return (0.05);
    if(value < .1) return (0.1);
    if(value < .3) return (0.2);
    if(value < .75) return (0.5);
    if(value < 1.5) return (1.0);
    if(value < 3.0) return (2.0);
    if(value < 7.5) return (5.0);
    if(value < 15.0) return (10.0);
    if(value < 30.0) return (20.0);
    if(value < 75.0) return (50.0);
    if(value < 150.0) return (100.0);
    if(value < 300.0) return (200.0);
    if(value < 750.0) return (500.0);
    return(1000.0);
}

/*************************************************************************
 * COMPUTE_RANGE: Return distance between two points
 */       
double compute_range(double x1, double y1, double x2, double y2)
{
    double xdiff,ydiff;

    xdiff = x1 - x2;
    ydiff = y1 - y2;

    return (sqrt(((xdiff * xdiff) + (ydiff * ydiff))));
}
