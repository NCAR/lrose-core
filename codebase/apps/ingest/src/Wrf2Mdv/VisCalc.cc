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
///////////////////////////////////////////////////////////////
// VisCalc.cc
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

#include "VisCalc.hh"

#include <math.h>


VisCalc::VisCalc()
{
    // set up the constants
    _rgas = 287.04;           // J/K/kg
    _eps = 0.622;
    _celkel = 273.15;
    _rhoice = 917;
    _rhowat = 1000;

    _tice = _celkel - 10.0;
    _coeflc = 144.7;
    _coeflp = 2.24;
    _coeffc = 327.8;
    _coeffp = 10.36;
    _exponlc = 0.8800;
    _exponlp = 0.7500;
    _exponfc = 1.0000;
    _exponfp = 0.7776;
    _const1 = -log(.02);
}

VisCalc::~VisCalc()
{
}

fl32 VisCalc::virtualTemp(fl32 temp, fl32 ratmix)
{
    return temp * (_eps + ratmix) / (_eps * (1.0 + ratmix));
}


fl32 VisCalc::minimum(fl32 a, fl32 b)
{
    return a <= b ? a : b;
}


void VisCalc::viscalc(fl32 qvp, fl32 qcw, fl32 qra, fl32 qci, fl32 qsn,
        fl32 tmk, fl32 prs, fl32 prsnow, int iice, char meth, fl32 *vis)
{
    // set default return in case of error
    *vis = 0.0;

    fl32 wmix=qvp;
    fl32 qprc, qcld;
    fl32 qrain, qsnow, qclw, qclice;
    if (iice == 0)
    {
        qprc = qra;
        qcld = qcw;
        if (tmk < _celkel)
        {
            qrain = 0.0;
            qsnow = qprc;
            qclw = 0.0;
            qclice = qcld;
        }
        else
        {
            qrain = qprc;
            qsnow = 0.0;
            qclw = qcld;
            qclice = 0.0;
        }
    }
    else
    {
        qprc = qra + qsn;
        qcld = qcw + qci;
        qrain = qra;
        qsnow = qsn;
        qclw = qcw;
        qclice = qci;
    }

    fl32 tv = virtualTemp(tmk, qvp);
    fl32 rhoair =100.0 * prs / (_rgas * tv);
    fl32 conclc=0, conclp=0, concfc=0, concfp =0;
    if (meth == 'd')
    {
        if (tmk < _celkel)
        {
            fl32 vovermd = (1.0 + wmix) / rhoair + 0.001 * (qprc + qcld) / _rhoice;
            conclc = 0.0;
            conclp = 0.0;
            concfc = qcld / vovermd;
            concfp = qprc / vovermd;
        }
        else
        {
            fl32 vovermd = (1.0 + wmix) / rhoair + 0.001 * (qprc + qcld) / _rhowat;
            conclc = qcld / vovermd;
            conclp = qprc / vovermd;
            concfc = 0.0;
            concfp = 0.0;
        }
    }

    else if (meth == 'b')
    {
        if (tmk < _tice)
        {
            fl32 vovermd = (1.0 + wmix) / rhoair + 0.001 * (qprc + qcld) / _rhoice;
            conclc = 0.0;
            conclp = 0.0;
            concfc = qcld / vovermd;
            concfp = qprc / vovermd;
        }
        else if (prsnow >= 50.0)
        {
            fl32 vovermd = (1.0 + wmix) / rhoair + 0.001 * (qprc / _rhoice + qcld / _rhowat);
            conclc = qcld / vovermd;
            conclp = 0.0;
            concfc = 0.0;
            concfp = qprc / vovermd;
        }
        else
        {
            fl32 vovermd = (1.0 + wmix) / rhoair + 0.001 * (qprc + qcld) / _rhowat;
            conclc = qcld / vovermd;
            conclp = qprc / vovermd;
            concfc = 0.0;
            concfp = 0.0;
        }
    }

    else if (meth == 'r')
    {
        fl32 vovermd = (1.0 + wmix) / rhoair + 0.001 * ((qclw + qrain) / _rhowat
            + (qclice + qsnow) / _rhoice);
        conclc = qclw / vovermd;
        conclp = qrain / vovermd;
        concfc = qclice / vovermd;
        concfp = qsnow / vovermd;
    }

    else
    {
        // Warning in VisCalc::viscalc(): meth not recognized.
    }

    fl32 beta = _coeffc * pow(concfc, _exponfc) + _coeffp * pow(concfp, _exponfp)
        + _coeflc * pow(conclc, _exponlc) + _coeflp * pow(conclp, _exponlp)
        + 1.0e-10;

    *vis = minimum(90.0, _const1 / beta);  // max of 90km
}

