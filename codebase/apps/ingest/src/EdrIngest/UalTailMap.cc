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
#include "UalTailMap.hh"

using namespace std;

UalTailMap::UalTailMap()
{
  isOK = TRUE;
  if ( _createMap() )
    isOK = FALSE;
      

}

UalTailMap::~UalTailMap()
{
  _tailMap.clear();

}

const char* UalTailMap::lookup(char *encryptedTail)
{
  string input(encryptedTail);
  string result = _tailMap[input];
  return result.c_str();
}

int UalTailMap::_createMap()
{
	_tailMap["OVWYR4JA"] = "N203UA";
	_tailMap["04VYR4BA"] = "N301UA";
	_tailMap["2MGYR4RA"] = "N302UA";
	_tailMap["OXFIR4JA"] = "N303UA";
	_tailMap["KG4IR4ZA"] = "N304UA";
	_tailMap["IUZYR3BA"] = "N305UA";
	_tailMap["WLKYR3RA"] = "N306UA";
	_tailMap["UZJIR3JA"] = "N307UA";
	_tailMap["QKSIR3ZA"] = "N308UA";
	_tailMap["CYHYR5BA"] = "N309UA";
	_tailMap["AZRIR4BA"] = "N310UA";
	_tailMap["4KCIR4BA"] = "N311UA";
	_tailMap["2T5YR4RA"] = "N312UA";
	_tailMap["YB2YR4JA"] = "N313UA";
	_tailMap["KSPIR4ZA"] = "N314UA";
	_tailMap["MAMIR3BA"] = "N315UA";
	_tailMap["RODIR3ZA"] = "N318UA";
	_tailMap["NEVIR3BA"] = "N325UA";
	_tailMap["HDZIR5BA"] = "N329UA";
	_tailMap["FKZYR4BA"] = "N330UA";
	_tailMap["5YKYR4BA"] = "N331UA";
	_tailMap["1DJIR4RA"] = "N332UA";
	_tailMap["ZRSIR4JA"] = "N333UA";
	_tailMap["PCHYR4ZA"] = "N334UA";
	_tailMap["NQEYR3BA"] = "N335UA";
	_tailMap["IN5IR3RA"] = "N336UA";
	_tailMap["S42IR3JA"] = "N337UA";
	_tailMap["UMLYR3ZA"] = "N338UA";
	_tailMap["GXIYR5BA"] = "N339UA";
	_tailMap["E4MIR4BA"] = "N340UA";
	_tailMap["0XQYR4RA"] = "N342UA";
	_tailMap["IZOYR3RA"] = "N346UA";
	_tailMap["WKNIR3JA"] = "N347UA";
	_tailMap["UYWIR3ZA"] = "N348UA";
	_tailMap["QDRYR5BA"] = "N349UA";
	_tailMap["CKVIR4BA"] = "N350UA";
	_tailMap["AYGIR4RA"] = "N351UA";
	_tailMap["ZEIYR4ZA"] = "N364UA";
	_tailMap["L1TIR3BA"] = "N365UA";
	_tailMap["NJQIR3RA"] = "N366UA";
	_tailMap["X0FYR3JA"] = "N367UA";
	_tailMap["RR3IR5BA"] = "N369UA";
	_tailMap["DY3YR4BA"] = "N370UA";
	_tailMap["FDOYR4RA"] = "N371UA";
	_tailMap["5RNIR4RA"] = "N372UA";
	_tailMap["1CWIR4JA"] = "N373UA";
	_tailMap["ZQRYR4ZA"] = "N374UA";
	_tailMap["OPCYR3BA"] = "N375UA";
	_tailMap["M3BIR3RA"] = "N376UA";
	_tailMap["IM0IR3JA"] = "N377UA";
	_tailMap["SXPYR3ZA"] = "N378UA";
	_tailMap["UFMYR5BA"] = "N379UA";
	_tailMap["EXXYR4RA"] = "N381UA";
	_tailMap["AFUYR4JA"] = "N382UA";
	_tailMap["0WHIR4JA"] = "N383UA";
	_tailMap["OZ1YR3BA"] = "N385UA";
	_tailMap["KKYYR3RA"] = "N386UA";
	_tailMap["XHRIR3ZA"] = "N398UA";
	_tailMap["VVCIR5BA"] = "N399UA";
	_tailMap["03NIR4BA"] = "N501UA";
	_tailMap["2OWIR4RA"] = "N502UA";
	_tailMap["O2RYR4JA"] = "N503UA";
	_tailMap["KHCYR4ZA"] = "N504UA";
	_tailMap["IVBIR3BA"] = "N505UA";
	_tailMap["WE0IR3RA"] = "N506UA";
	_tailMap["U1PYR3JA"] = "N507UA";
	_tailMap["QJMYR3ZA"] = "N508UA";
	_tailMap["C0XIR5BA"] = "N509UA";
	_tailMap["A1XYR4BA"] = "N510UA";
	_tailMap["4JUYR4BA"] = "N511UA";
	_tailMap["20HIR4RA"] = "N512UA";
	_tailMap["YIEIR4JA"] = "N513UA";
	_tailMap["KR1YR4ZA"] = "N514UA";
	_tailMap["MCYYR3BA"] = "N515UA";
	_tailMap["WQLIR3RA"] = "N516UA";
	_tailMap["V5IIR3JA"] = "N517UA";
	_tailMap["RNVYR3ZA"] = "N518UA";
	_tailMap["D4GYR5BA"] = "N519UA";
	_tailMap["B5QIR4BA"] = "N520UA";
	_tailMap["5NFYR4BA"] = "N521UA";
	_tailMap["324YR4RA"] = "N522UA";
	_tailMap["ZH3IR4JA"] = "N523UA";
	_tailMap["LVOIR4ZA"] = "N524UA";
	_tailMap["NGJYR3BA"] = "N525UA";
	_tailMap["XUSYR3RA"] = "N526UA";
	_tailMap["TLRIR3JA"] = "N527UA";
	_tailMap["RZCIR3ZA"] = "N528UA";
	_tailMap["HI5YR5BA"] = "N529UA";
	_tailMap["FLBIR4BA"] = "N530UA";
	_tailMap["500IR4BA"] = "N531UA";
	_tailMap["1IPYR4RA"] = "N532UA";
	_tailMap["ZTMYR4JA"] = "N533UA";
	_tailMap["PBXIR4ZA"] = "N534UA";
	_tailMap["NSUIR3BA"] = "N535UA";
	_tailMap["JADYR3RA"] = "N536UA";
	_tailMap["S5AYR3JA"] = "N537UA";
	_tailMap["UO1IR3ZA"] = "N538UA";
	_tailMap["G2YIR5BA"] = "N539UA";
	_tailMap["E3YYR4BA"] = "N540UA";
	_tailMap["AOLIR4RA"] = "N541UA";
	_tailMap["02IIR4RA"] = "N542UA";
	_tailMap["2HVYR4JA"] = "N543UA";
	_tailMap["OVGYR4ZA"] = "N544UA";
	_tailMap["KGFIR3BA"] = "N545UA";
	_tailMap["I14IR3RA"] = "N546UA";
	_tailMap["WJZYR3JA"] = "N547UA";
	_tailMap["U0KYR3ZA"] = "N548UA";
	_tailMap["QIJIR5BA"] = "N549UA";
	_tailMap["CJJYR4BA"] = "N550UA";
	_tailMap["A0SYR4RA"] = "N551UA";
	_tailMap["4IRIR4RA"] = "N552UA";
	_tailMap["2TCIR4JA"] = "N553UA";
	_tailMap["YC5YR4ZA"] = "N554UA";
	_tailMap["KQ2YR3BA"] = "N555UA";
	_tailMap["J5PIR3RA"] = "N556UA";
	_tailMap["XNMIR3JA"] = "N557UA";
	_tailMap["V4TYR3ZA"] = "N558UA";
	_tailMap["RMQYR5BA"] = "N559UA";
	_tailMap["DNUIR4BA"] = "N560UA";
	_tailMap["B4DYR4RA"] = "N561UA";
	_tailMap["5MAYR4RA"] = "N562UA";
	_tailMap["3V1IR4JA"] = "N563UA";
	_tailMap["ZGYIR4ZA"] = "N564UA";
	_tailMap["LUNYR3BA"] = "N565UA";
	_tailMap["NLWYR3RA"] = "N566UA";
	_tailMap["XZVIR3JA"] = "N567UA";
	_tailMap["TKGIR3ZA"] = "N568UA";
	_tailMap["RYBYR5BA"] = "N569UA";
	_tailMap["DZFIR4BA"] = "N570UA";
	_tailMap["FI4IR4RA"] = "N571UA";
	_tailMap["5TZYR4RA"] = "N572UA";
	_tailMap["1BKYR4JA"] = "N573UA";
	_tailMap["ZSJIR4ZA"] = "N574UA";
	_tailMap["PASIR3BA"] = "N575UA";
	_tailMap["M5HYR3RA"] = "N576UA";
	_tailMap["INEYR3JA"] = "N577UA";
	_tailMap["S25IR3ZA"] = "N578UA";
	_tailMap["UH2IR5BA"] = "N579UA";
	_tailMap["GO2YR4BA"] = "N580UA";
	_tailMap["E2PIR4RA"] = "N581UA";
	_tailMap["AHMIR4JA"] = "N582UA";
	_tailMap["0VTYR4JA"] = "N583UA";
	_tailMap["2GQYR4ZA"] = "N584UA";
	_tailMap["OUDIR3BA"] = "N585UA";
	_tailMap["KLAIR3RA"] = "N586UA";
	_tailMap["I03YR3JA"] = "N587UA";
	_tailMap["WIOYR3ZA"] = "N588UA";
	_tailMap["UTNIR5BA"] = "N589UA";
	_tailMap["G0NYR4BA"] = "N590UA";
	_tailMap["ATVIR4JA"] = "N592UA";
	_tailMap["4BGIR4JA"] = "N593UA";
	_tailMap["2SBYR4ZA"] = "N594UA";
	_tailMap["P50YR3BA"] = "N595UA";
	_tailMap["LNZIR3RA"] = "N596UA";
	_tailMap["J4KIR3JA"] = "N597UA";
	_tailMap["XMXYR3ZA"] = "N598UA";

  
  return 0;
}
