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

#include "GiniPDB.hh"
using namespace std;

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                                 CONSTRUCTOR                                //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
GiniPDB::GiniPDB()
   {
   } // End of GiniPDB constructor.

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                                  DESTRUCTOR                                //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
GiniPDB::~GiniPDB()
   {
   } // End of GiniPDB destructor.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  GiniPDB::decodeGiniPDB()                                                  //
//                                                                            //
// Description:                                                               //
//  Public method that uses the values in an unsigned char array to set       //
//  the data members of a GiniPDB object.                                     //
//                                                                            //
// Input:                                                                     //
//  pdb - unsigned char array read from Gini input file.                      //
//                                                                            //
// Output: None                                                               //
//  Data members of GiniPDB object are assigned values.                       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void GiniPDB::decodeGiniPDB(unsigned char pdb[512])
   {
   const string methodName = "GiniPDB::decodeGiniPDB()";

   if (pdb[0] == 1)
      {
      setSource("NESDIS");
      }
   else
      {
      setSource("UNKNOWN");
      }

   /////////////////////////////////////////////////

   switch ((int)pdb[1])
      {
      case 6 :
         setCreator("Composite");
         break;

      case 7 :
         setCreator("DMSP");
         break;

      case 8 :
         setCreator("GMS");
         break;

      case 9 :
         setCreator("METEOSAT");
         break;

      case 10 :
         setCreator("GOES-7");
         break;

      case 11 :
         setCreator("GOES-8");
         break;

      case 12 :
         setCreator("GOES-9");
         break;

      case 13 :
         setCreator("GOES-10");
         break;

      case 14 :
         setCreator("GOES-11");
         break;

      case 15 :
         setCreator("GOES-12");
         break;

      default :
         setCreator("UNKNOWN");
         break;
      }

   /////////////////////////////////////////////////

   switch ((int)pdb[2])
      {
      case 0 :
         setSector("NorthHemComposite");
         break;

      case 1 :
         setSector("EastCONUS");
         break;

      case 2 :
         setSector("WestCONUS");
         break;

      case 3 :
         setSector("AlaskaRegional");
         break;

      case 4 :
         setSector("AlaskaNational");
         break;

      case 5 :
         setSector("HawaiiRegional");
         break;

      case 6 :
         setSector("HawaiiNational");
         break;

      case 7 :
         setSector("PuertoRicoRegional");
         break;

      case 8 :
         setSector("PuertoRicoNational");
         break;

      case 9 :
         setSector("SuperNational");
         break;

      case 10 :
         setSector("GeoNorthHemComp");
         break;

      default :
         setSector("UNKNOWN");
         break;
      }

   /////////////////////////////////////////////////
   switch((int)pdb[3])
      {
      case 1 :
         setChannel("Visible");
         break;

      case 2 :
         setChannel("3.9_micron");
         break;

      case 3 :
         setChannel("6.7_micron");
         break;

      case 4 :
         setChannel("11_micron");
         break;

      case 5 :
         setChannel("12_micron");
         break;

      case 6 :
         setChannel("13_micron");
         break;

      case 7 :
         setChannel("1.3_micron");
         break;

      case 13 :
         setChannel("ImagerLI");
         break;

      case 14 :
         setChannel("ImagerPrecipWater");
         break;

      case 15 :
         setChannel("ImagerSurfSkinTemp");
         break;

      case 16 :
         setChannel("SounderLI");
         break;

      case 17 :
         setChannel("SounderPrecipWater");
         break;

      case 18 :
         setChannel("SounderSurfSkinTemp");
         break;

      case 26 :
         setChannel("Scatterometer");
         break;

      case 27 :
         setChannel("CloudTopPres");
         break;

      case 28 :
         setChannel("CloudAmount");
         break;

      case 29 :
         setChannel("RainRate");
         break;

      case 30 :
         setChannel("SurfWinds");
         break;

      case 31 :
         setChannel("SurfWetness");
         break;

      case 32 :
         setChannel("IceConcentration");
         break;

      case 33 :
         setChannel("IceType");
         break;

      case 34 :
         setChannel("IceEdge");
         break;

      case 35 :
         setChannel("CloudWaterContent");
         break;

      case 36 :
         setChannel("SurfaceType");
         break;

      case 37 :
         setChannel("SnowIndicator");
         break;

      case 38 :
         setChannel("SnowWaterContent");
         break;

      case 39 :
         setChannel("DerivedVolcano");
         break;

      case 41 :
         setChannel("Sounder_14.71m");
         break;

      case 42 :
         setChannel("Sounder_14.37m");
         break;

      case 43 :
         setChannel("Sounder_14.06m");
         break;

      case 44 :
         setChannel("Sounder_13.64m");
         break;

      case 45 :
         setChannel("Sounder_13.37m");
         break;

      case 46 :
         setChannel("Sounder_12.66m");
         break;

      case 47 :
         setChannel("Sounder_12.02m");
         break;

      case 48 :
         setChannel("Sounder_11.03m");
         break;

      case 49 :
         setChannel("Sounder_9.71m");
         break;

      case 50 :
         setChannel("Sounder_7.43m");
         break;

      case 51 :
         setChannel("Sounder_7.02m");
         break;

      case 52 :
         setChannel("Sounder_6.51m");
         break;

      case 53 :
         setChannel("Sounder_4.57m");
         break;

      case 54 :
         setChannel("Sounder_4.52m");
         break;

      case 55 :
         setChannel("Sounder_4.45m");
         break;

      case 56 :
         setChannel("Sounder_4.13m");
         break;

      case 57 :
         setChannel("Sounder_3.98m");
         break;

      case 58 :
         setChannel("Sounder_3.74m");
         break;

      case 59 :
         setChannel("SounderVisible");
         break;

      default :
         setChannel("UNKNOWN");
         break;
      }

   setNumRecords(256*((int) pdb[4]) + (int) pdb[5]);
   setSizeRecords(256*((int) pdb[6]) + (int) pdb[7]);
   setYear((int) pdb[8] + 1900);
   setMonth((int) pdb[9]);
   setDay((int) pdb[10]);
   setHour((int) pdb[11]);
   setMin((int) pdb[12]);
   setSec((int) pdb[13]);
   setHSec((int) pdb[14]);
 
   setProjection((int) pdb[15]);

   if(getProjection() == 3 || getProjection() == 5)
      {
      // Lambert conformal or polar stereographic projections

      setNx(256*((int) pdb[16]) + (int) pdb[17]);
      setNy(256*((int) pdb[18]) + (int) pdb[19]);

      double LatDiv, LonDiv;
      if (pdb[20] > 127)
         {
         pdb[20] = pdb[20] - 128;
         LatDiv = -10000.0;
         }
      else
         {
         LatDiv = 10000.0;
         }

      if (pdb[23] > 127)
         {
         pdb[23] = pdb[23] - 128;
         LonDiv = -10000.0;
         }
      else
         {
         LonDiv = 10000.0;
         }

      setLat1(double(pdb[20]*256*256 + pdb[21]*256 + pdb[22])/LatDiv);
      setLon1(double(pdb[23]*256*256 + pdb[24]*256 + pdb[25])/LonDiv);

      if (pdb[27] > 127)
         {
         pdb[27] = pdb[27] - 128;
         LonDiv = -10000.0;
         }
      else
         {
         LonDiv = 10000.0;
         }

      setLov(double(pdb[27]*256*256 + pdb[28]*256 + pdb[29])/LonDiv);

      setDx(double(pdb[30]*256*256 + pdb[31]*256 + pdb[32])/10.0);
      setDy(double(pdb[33]*256*256 + pdb[34]*256 + pdb[35])/10.0);

      setSouthPole(pdb[36]);

      if (pdb[37] > 127)
         {
         pdb[37]=pdb[37]-128;
         setScanLeftToRight(false);
         }
      else
         {
         setScanLeftToRight(true);
         } 

      if (pdb[37] > 63)
         {
         pdb[37]=pdb[37]-64;
         setScanTopToBottom(false);
         }
      else
         {
         setScanTopToBottom(true);
         }

      if (pdb[37] > 31)
         {
         pdb[37]=pdb[37]-32;
         setScanXInFirst(false);
         }
      else
         {
         setScanXInFirst(true);
         }

      if (pdb[38] > 127)
         {
         pdb[38] = pdb[38] - 128;
         LatDiv = -10000.0;
         }
      else
         {
         LatDiv = 10000.0;
         }
  
      setLatin(double(pdb[38]*256*256 + pdb[39]*256 + pdb[40])/LatDiv);
      }
   else
      {
      // Mercator projection
      setNx(pdb[16]*256 + pdb[17]);
      setNy(pdb[18]*256 + pdb[19]);

      double LatDiv, LonDiv;
      if (pdb[20] > 127)
         {
         pdb[20] = pdb[20] - 128;
         LatDiv = -10000.0;
         }
      else
         {
         LatDiv = 10000.0;
         }

      if (pdb[23] > 127)
         {
         pdb[23] = pdb[23] - 128;
         LonDiv = -10000.0;
         }
      else
         {
         LonDiv = 10000.0;
         }

      setLat1(double(pdb[20]*256*256 + pdb[21]*256 + pdb[22])/LatDiv);
      setLon1(double(pdb[23]*256*256 + pdb[24]*256 + pdb[25])/LonDiv);

      setResFlag(pdb[26]);

      if (pdb[27] > 127)
         {
         pdb[27] = pdb[27] - 128;
         LatDiv = -10000.0;
         }
      else
         {
         LatDiv = 10000.0;
         }

      if (pdb[30] > 127)
         {
         pdb[30] = pdb[30] - 128;
         LonDiv = -10000.0;
         }
      else
         {
         LonDiv = 10000.0;
         }

      setLat2(double(pdb[27]*256*256 + pdb[28]*256 + pdb[29])/LatDiv);
      setLon2(double(pdb[30]*256*256 + pdb[31]*256 + pdb[32])/LonDiv);

      setDi(double(pdb[33]*256 + pdb[34]));
      setDj(double(pdb[35]*256 + pdb[36]));

      if (pdb[37] > 127)
         {
         pdb[37]=pdb[37]-128;
         setScanLeftToRight(false);
         }
      else
         {
         setScanLeftToRight(true);
         }

      if (pdb[37] > 63)
         {
         pdb[37]=pdb[37]-64;
         setScanTopToBottom(false);
         }
      else
         {
         setScanTopToBottom(true);
         }

      if (pdb[37] > 31)
         {
         pdb[37]=pdb[37]-32;
         setScanXInFirst(false);
         }
      else
         {
         setScanXInFirst(true);
         }

      if (pdb[38] > 127)
         {
         pdb[38] = pdb[38] - 128;
         LatDiv = -10000.0;
         }
      else
         {
         LatDiv = 10000.0;
         }
  
      setLatin(double(pdb[38]*256*256 + pdb[39]*256 + pdb[40])/LatDiv);
      } // End of projection conditional.

   setImageRes((int)pdb[41]);
   setCompression((int)pdb[42]);
   setCreatorVersion((int)pdb[43]);
   setPDBSize(pdb[44]*256 + pdb[45]);

   if ( (pdb[46] == 1) || (pdb[46] == 2))
      {
      setNavIncluded(true);
      }
   else
      {
      setNavIncluded(false);
      }

   if ( (pdb[46] == 1) || (pdb[46] == 3))
      {
      setCalIncluded(true);
      }
   else
      {
      setCalIncluded(false);
      }
   } // End of decodeGiniPDB() method.
