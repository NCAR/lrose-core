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
// VisCalc.hh
//
// Visibility Calculation
// This class contains a translation from Fortran of the Stoelinga-Warner
// Visibility Algorithm for calculating atmospheric visibility based on
// precipitation, cloud water, ice, snow, etc.  The original code
// came from a code file named viscalc.f, containing subroutine viscalc().
//
// @author Carl Drews, RAL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// @date November 4, 2004
//
///////////////////////////////////////////////////////////////

#ifndef VisCalc_H
#define VisCalc_H

#include <dataport/port_types.h>


class VisCalc {

protected:

    // numeric constants
    fl32 _rgas;
    fl32 _eps;
    fl32 _celkel;
    fl32 _rhoice;
    fl32 _rhowat;

    fl32 _tice;
    fl32 _coeflc;
    fl32 _coeflp;
    fl32 _coeffc;
    fl32 _coeffp;
    fl32 _exponlc;
    fl32 _exponlp;
    fl32 _exponfc;
    fl32 _exponfp;
    fl32 _const1;

public:

    // constructor
    // This could be an abstract class with only a static method,
    // but I figure that someday we might want to instantiate the object
    // and have it remember some information between calls.
    // So I'll set up that usage pattern from the beginning.
    VisCalc();
    virtual ~VisCalc() {};
    /**
     * This function returns the virtual temperature in degrees Kelvin.
     * @param temp temperature in degrees K
     * @param ratmix mixing ratio in kg/kg
     */
    fl32 virtualTemp(fl32 temp, fl32 ratmix);

    /**
     * @return the minimum of the two supplied values
     */
    fl32 minimum(fl32 a, fl32 b);

    /**
     * This routine computes horizontal visibility at the lowest sigma
     * layer, from qcw, qra, qci, and qsn.  NOTE!  qvp is passed in with
     * kg/kg units, whereas the hydrometeor mixing ratios are passed in as
     * g/kg.  At each grid point, the routine assigns values of total
     * liq. eq. precip, total liq.  equiv. cloud, rain, snow, cloud
     * water, and cloud ice, based on the value of iice.
     *
     * If iice=0:
     * qprc=qra     qrain=qra and qclw=qcw if T>0C
     * qcld=qcw          =0           =0   if T<0C
     *             qsnow=qsn and qclice=qcw  if T<0C
     *                  =0             =0    if T>0C
     *
     * If iice=1:
     * qprc=qra+qsn   qrain=qra and qclw=qcw
     * qcld=qcw+qci   qsnow=qsn and qclice=qcw
     *
     * Independent of the above definitions, the scheme can use different
     * assumptions of the state of hydrometeors:
     *   meth='d': qprc is all frozen if T<0, liquid if T>0
     *   meth='b': Bocchieri scheme used to determine whether qprc
     *      is rain or snow. A temperature assumption is used to
     *      determine whether qcld is liquid or frozen.
     *   meth='r': Uses the four mixing ratios qrain, qsnow, qclw,
     *      and qclice
     *
     * The routine uses the following
     * expressions for extinction coefficient, beta (in km**-1),
     * with C being the mass concentration (in g/m**3):
     *
     * cloud water:  beta = 144.7 * C ** (0.8800)
     * rain water:   beta =  2.24 * C ** (0.7500)
     * cloud ice:    beta = 327.8 * C ** (1.0000)
     * snow:         beta = 10.36 * C ** (0.7776)
     *
     * These expressions were obtained from the following sources:
     * for cloud water: from Kunkel (1984)
     * for rainwater: from Marshall-Palmer distribution,
     *    with No=8e6 m**-4 and rho_w=1000 kg/m**3
     * for cloud ice: assume randomly oriented plates which follow
     *    mass-diameter relationship from Rutledge and Hobbs (1983)
     * for snow: from Stallabrass (1985), assuming beta = -ln(.02)/vis
     *
     * The extinction coefficient for each water species present is
     * calculated, and then all applicable betas are summed to yield
     * a single beta. Then the following relationship is used to
     * determine visibility (in km), where epsilon is the threshhold
     * of contrast, usually taken to be .02:
     *
     * vis = -ln(epsilon)/beta      [found in Kunkel (1984)]
     *
     * Publication reference:
     * Nonhydrostatic, Mesobeta-Scale Model Simulations of Cloud Ceiling
     * and Visibility for an East Coast Winter Precipitation Event;
     * by Mark Stoelinga and Thomas Warner,
     * American Meteorological Society, 1999.
     *
     * Parameters:
     * @param qvp Water vapor mixing ratio, in kg kg-1
     * @param qcw Cloud water mixing ratio, in g kg-1
     * @param qra Rain water mixing ratio, in g kg-1
     * @param qci Cloud ice mixing ratio, in g kg-1
     * @param qsn Snow mixing ratio, in g kg-1
     * @param tmk Temperature, in K
     * @param prs Pressure, in mb
     * @param prsnow Fraction of precipitation that is snow (not rain), as a percentage 0-100
     * @param iice 1 = cloud ice and snow arrays are present, 0 = not present
     * @param meth Method for calculating from hydrometeors (see above).
     * @param vis Returned as the calculated surface visibility, in km.
     */
    virtual void viscalc(fl32 qvp, fl32 qcw, fl32 qra, fl32 qci, fl32 qsn,
        fl32 tmk, fl32 prs, fl32 prsnow, int iice, char meth, fl32 *vis);

};

#endif


