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
//////////////////////////////////////////////////////////////////////////
// 
// Class for getting location of NEXRAD station
// 
// Mike Dixon, EOL, NCAR, Boulder, CO, 80307, USA
// August 2012
//
//////////////////////////////////////////////////////////////////////////

#include <Radx/NexradLoc.hh>
#include <Radx/RadxPath.hh>
#include <cmath>
using namespace std;

const NexradLoc::locInfo_t NexradLoc::_locInfo[_nStations] = {

  { 14929,	"KABR",	"ABERDEEN",	"SD",	45,	27,	21,	-98,	24,	47,	397 },
  { 3019,	"KABX",	"ALBUQUERQUE",	"NM",	35,	8,	59,	-106,	49,	26,	1789 },
  { 93773,	"KAKQ",	"NORFOLK/RICH",	"VA",	36,	59,	2,	-77,	0,	26,	34 },
  { 23047,	"KAMA",	"AMARILLO",	"TX",	35,	14,	0,	-101,	42,	33,	1093 },
  { 12899,	"KAMX",	"MIAMI",	"FL",	25,	36,	40,	-80,	24,	46,	4 },
  { 4837,	"KAPX",	"GAYLORD",	"MI",	44,	54,	26,	-84,	43,	11,	446 },
  { 94987,	"KARX",	"LA_CROSSE",	"WI",	43,	49,	22,	-91,	11,	28,	389 },
  { 94287,	"KATX",	"SEATTLE",	"WA",	48,	11,	40,	-122,	29,	45,	151 },
  { 93240,	"KBBX",	"BEALE_AFB",	"CA",	39,	29,	46,	-121,	37,	54,	53 },
  { 4725,	"KBGM",	"BINGHAMTON",	"NY",	42,	11,	59,	-75,	59,	5,	490 },
  { 94289,	"KBHX",	"EUREKA",	"CA",	40,	29,	54,	-124,	17,	31,	732 },
  { 24011,	"KBIS",	"BISMARCK",	"ND",	46,	46,	15,	-100,	45,	38,	505 },
  { 94046,	"KBLX",	"BILLINGS",	"MT",	45,	51,	14,	-108,	36,	24,	1097 },
  { 53823,	"KBMX",	"BIRMINGHAM",	"AL",	33,	10,	20,	-86,	46,	12,	197 },
  { 54765,	"KBOX",	"BOSTON",	"MA",	41,	57,	21,	-71,	8,	13,	36 },
  { 12919,	"KBRO",	"BROWNSVILLE",	"TX",	25,	54,	58,	-97,	25,	8,	7 },
  { 14733,	"KBUF",	"BUFFALO",	"NY",	42,	56,	56,	-78,	44,	12,	211 },
  { 92804,	"KBYX",	"KEY_WEST",	"FL",	24,	35,	51,	-81,	42,	11,	3 },
  { 13883,	"KCAE",	"COLUMBIA",	"SC",	33,	56,	55,	-81,	7,	6,	70 },
  { 94625,	"KCBW",	"HOULTON",	"ME",	46,	2,	22,	-67,	48,	24,	227 },
  { 4101,	"KCBX",	"BOISE",	"ID",	43,	29,	27,	-116,	14,	8,	933 },
  { 54764,	"KCCX",	"STATE_COLLEGE","PA",	40,	55,	23,	-78,	0,	13,	733 },
  { 14820,	"KCLE",	"CLEVELAND",	"OH",	41,	24,	47,	-81,	51,	35,	233 },
  { 53845,	"KCLX",	"CHARLESTON",	"SC",	32,	39,	20,	-81,	2,	31,	30 },
  { 12924,	"KCRP",	"CORP_CHRISTI",	"TX",	27,	47,	3,	-97,	30,	40,	13 },
  { 54774,	"KCXX",	"BURLINGTON",	"VT",	44,	30,	40,	-73,	10,	1,	97 },
  { 24018,	"KCYS",	"CHEYENNE",	"WY",	41,	9,	7,	-104,	48,	22,	1868 },
  { 93235,	"KDAX",	"SACRAMENTO",	"CA",	38,	30,	4,	-121,	40,	40,	9 },
  { 13985,	"KDDC",	"DODGE_CITY",	"KS",	37,	45,	39,	-99,	58,	7,	789 },
  { 22015,	"KDFX",	"LAUGHLIN_AFB",	"TX",	29,	16,	22,	-100,	16,	50,	345 },
  { 93771,	"KDIX",	"PHILADELPHIA",	"PA",	39,	56,	49,	-74,	24,	39,	45 },
  { 14913,	"KDLH",	"DULUTH",	"MN",	46,	50,	13,	-92,	12,	35,	435 },
  { 94984,	"KDMX",	"DES_MOINES",	"IA",	41,	43,	53,	-93,	43,	22,	299 },
  { 93770,	"KDOX",	"DOVER_AFB",	"DE",	38,	49,	32,	-75,	26,	24,	15 },
  { 4830,	"KDTX",	"DETROIT",	"MI",	42,	41,	59,	-83,	28,	18,	327 },
  { 94982,	"KDVN",	"DAVENPORT",	"IA",	41,	36,	42,	-90,	34,	51,	230 },
  { 3987,	"KDYX",	"DYESS_AFB",	"TX",	32,	32,	18,	-99,	15,	15,	462 },
  { 3983,	"KEAX",	"KANSAS_CITY",	"MO",	38,	48,	37,	-94,	15,	52,	303 },
  { 53112,	"KEMX",	"TUCSON",	"AZ",	31,	53,	37,	-110,	37,	49,	1586 },
  { 54766,	"KENX",	"ALBANY",	"NY",	42,	35,	11,	-74,	3,	50,	557 },
  { 53851,	"KEOX",	"FORT_RUCKER",	"AL",	31,	27,	38,	-85,	27,	34,	132 },
  { 3020,	"KEPZ",	"EL_PASO",	"TX",	31,	52,	23,	-106,	41,	53,	1251 },
  { 53110,	"KESX",	"LAS_VEGAS",	"NV",	35,	42,	4,	-114,	53,	29,	1483 },
  { 53825,	"KEVX",	"EGLIN_AFB",	"FL",	30,	33,	52,	-85,	55,	17,	43 },
  { 12971,	"KEWX",	"AUSTIN/S_ANT",	"TX",	29,	42,	14,	-98,	1,	42,	193 },
  { 53114,	"KEYX",	"EDWARDS_AFB",	"CA",	35,	5,	52,	-117,	33,	39,	840 },
  { 53831,	"KFCX",	"ROANOKE",	"VA",	37,	1,	28,	-80,	16,	26,	874 },
  { 3981,	"KFDR",	"ALTUS_AFB",	"OK",	34,	21,	44,	-98,	58,	35,	386 },
  { 3022,	"KFDX",	"CANNON_AFB",	"NM",	34,	38,	7,	-103,	37,	48,	1417 },
  { 53819,	"KFFC",	"ATLANTA",	"GA",	33,	21,	49,	-84,	33,	57,	262 },
  { 14944,	"KFSD",	"SIOUX_FALLS",	"SD",	43,	35,	16,	-96,	43,	46,	436 },
  { 53113,	"KFSX",	"FLAGSTAFF",	"AZ",	34,	34,	28,	-111,	11,	52,	2261 },
  { 3018,	"KFTG",	"DENVER",	"CO",	39,	47,	12,	-104,	32,	45,	1675 },
  { 3985,	"KFWS",	"DALLAS/FTW",	"TX",	32,	34,	23,	-97,	18,	11,	208 },
  { 94008,	"KGGW",	"GLASGOW",	"MT",	48,	12,	23,	-106,	37,	30,	694 },
  { 3025,	"KGJX",	"GRAND_JUNCT",	"CO",	39,	3,	44,	-108,	12,	50,	3046 },
  { 23065,	"KGLD",	"GOODLAND",	"KS",	39,	21,	59,	-101,	42,	2,	1113 },
  { 14898,	"KGRB",	"GREEN_BAY",	"WI",	44,	29,	54,	-88,	6,	41,	208 },
  { 3992,	"KGRK",	"FORT_HOOD",	"TX",	30,	43,	19,	-97,	22,	59,	164 },
  { 94860,	"KGRR",	"GRAND_RAPIDS",	"MI",	42,	53,	38,	-85,	32,	41,	237 },
  { 3870,	"KGSP",	"GREER",	"SC",	34,	52,	0,	-82,	13,	12,	287 },
  { 53837,	"KGWX",	"COLUMBUS_AFB",	"MS",	33,	53,	48,	-88,	19,	44,	145 },
  { 54762,	"KGYX",	"PORTLAND",	"ME",	43,	53,	29,	-70,	15,	24,	125 },
  { 3023,	"KHDX",	"HOLLOMAN_AFB",	"NM",	33,	4,	35,	-106,	7,	22,	1287 },
  { 3980,	"KHGX",	"HOUSTON",	"TX",	29,	28,	19,	-95,	4,	45,	5 },
  { 53111,	"KHNX",	"SAN_JOAQUIN_V","CA",	36,	18,	51,	-119,	37,	56,	75 },
  { 53839,	"KHPX",	"FORT_CAMPBELL","KY",	36,	44,	12,	-87,	17,	6,	176 },
  { 53857,	"KHTX",	"HUNTSVILLE",	"AL",	34,	55,	50,	-86,	05,	00,	537 },
  { 3928,	"KICT",	"WICHITA",	"KS",	37,	39,	17,	-97,	26,	34,	407 },
  { 13841,	"KILN",	"CINCINNATI",	"OH",	39,	25,	13,	-83,	49,	18,	322 },
  { 4833,	"KILX",	"LINCOLN",	"IL",	40,	9,	2,	-89,	20,	13,	177 },
  { 93819,	"KIND",	"INDIANAPOLIS",	"IN",	39,	42,	27,	-86,	16,	49,	241 },
  { 3984,	"KINX",	"TULSA",	"OK",	36,	10,	30,	-95,	33,	53,	204 },
  { 23104,	"KIWA",	"PHOENIX",	"AZ",	33,	17,	21,	-111,	40,	12,	412 },
  { 3940,	"KJAN",	"JACKSON",	"MS",	32,	19,	4,	-90,	4,	48,	91 },
  { 13889,	"KJAX",	"JACKSONVILLE",	"FL",	30,	29,	5,	-81,	42,	7,	10 },
  { 53824,	"KJGX",	"ROBINS_AFB",	"GA",	32,	40,	30,	-83,	21,	4,	159 },
  { 3889,	"KJKL",	"JACKSON",	"KY",	37,	35,	27,	-83,	18,	47,	415 },
  { 23042,	"KLBB",	"LUBBOCK",	"TX",	33,	39,	14,	-101,	48,	51,	993 },
  { 3937,	"KLCH",	"LAKE_CHARLES",	"LA",	30,	7,	31,	-93,	12,	57,	4 },
  { 53813,	"KLIX",	"NEW_ORLEANS",	"LA",	30,	20,	12,	-89,	49,	32,	7 },
  { 94049,	"KLNX",	"NORTH_PLATTE",	"NE",	41,	57,	28,	-100,	34,	35,	905 },
  { 4831,	"KLOT",	"CHICAGO",	"IL",	41,	36,	17,	-88,	5,	5,	202 },
  { 4108,	"KLRX",	"ELKO",	        "NV",	40,	44,	23,	-116,	48,	10,	2056 },
  { 3982,	"KLSX",	"ST_LOUIS",	"MO",	38,	41,	56,	-90,	40,	58,	185 },
  { 93774,	"KLTX",	"WILMINGTON",	"NC",	33,	59,	22,	-78,	25,	44,	20 },
  { 53827,	"KLVX",	"LOUISVILLE",	"KY",	37,	58,	31,	-85,	56,	38,	219 },
  { 93767,	"KLWX",	"STERLING",	"VA",	38,	58,	31,	-77,	28,	40,	83 },
  { 3952,	"KLZK",	"LITTLE_ROCK",	"AR",	34,	50,	12,	-92,	15,	44,	173 },
  { 23023,	"KMAF",	"MIDLAND/ODSSA","TX",	31,	56,	36,	-102,	11,	21,	874 },
  { 94296,	"KMAX",	"MEDFORD",	"OR",	42,	4,	52,	-122,	43,	2,	2290 },
  { 94048,	"KMBX",	"MINOT_AFB",	"ND",	48,	23,	33,	-100,	51,	54,	455 },
  { 93768,	"KMHX",	"MOREHEAD_CITY","NC",	34,	46,	34,	-76,	52,	34,	9 },
  { 4834,	"KMKX",	"MILWAUKEE",	"WI",	42,	58,	4,	-88,	33,	2,	292 },
  { 12838,	"KMLB",	"MELBOURNE",	"FL",	28,	6,	48,	-80,	39,	15,	11 },
  { 13894,	"KMOB",	"MOBILE",	"AL",	30,	40,	46,	-88,	14,	23,	63 },
  { 94983,	"KMPX",	"MINNEAPOLIS",	"MN",	44,	50,	56,	-93,	33,	56,	288 },
  { 94850,	"KMQT",	"MARQUETTE",	"MI",	46,	31,	52,	-87,	32,	54,	430 },
  { 53832,	"KMRX",	"KNOXVILLE",	"TN",	36,	10,	7,	-83,	24,	6,	408 },
  { 4103,	"KMSX",	"MISSOULA",	"MT",	47,	2,	28,	-113,	59,	10,	2394 },
  { 4104,	"KMTX",	"SALT_LAKE_CTY","UT",	41,	15,	46,	-112,	26,	52,	1969 },
  { 93236,	"KMUX",	"SAN_FRANCISCO","CA",	37,	9,	18,	-121,	53,	54,	1057 },
  { 94986,	"KMVX",	"GRAND_FORKS",	"ND",	47,	31,	40,	-97,	19,	32,	300 },
  { 53826,	"KMXX",	"MAXWELL_AFB",	"AL",	32,	32,	12,	-85,	47,	23,	122 },
  { 53115,	"KNKX",	"SAN_DIEGO",	"CA",	32,	55,	8,	-117,	2,	31,	291 },
  { 93839,	"KNQA",	"MEMPHIS",	"TN",	35,	20,	41,	-89,	52,	24,	86 },
  { 94980,	"KOAX",	"OMAHA",	"NE",	41,	19,	13,	-96,	22,	0,	350 },
  { 53833,	"KOHX",	"NASHVILLE",	"TN",	36,	14,	50,	-86,	33,	45,	176 },
  { 94703,	"KOKX",	"NEW_YORK_CITY","NY",	40,	51,	56,	-72,	51,	50,	26 },
  { 4106,	"KOTX",	"SPOKANE",	"WA",	47,	40,	49,	-117,	37,	36,	727 },
  { 3948,	"KOUN",	"NORMAN",	"OK",	35,	14,	10,	-97,	27,	44,	390 },
  { 3816,	"KPAH",	"PADUCAH",	"KY",	37,	4,	6,	-88,	46,	19,	119 },
  { 4832,	"KPBZ",	"PITTSBURGH",	"PA",	40,	31,	54,	-80,	13,	6,	361 },
  { 24155,	"KPDT",	"PENDLETON",	"OR",	45,	41,	26,	-118,	51,	10,	462 },
  { 3993,	"KPOE",	"FORT_POLK",	"LA",	31,	9,	20,	-92,	58,	33,	124 },
  { 3021,	"KPUX",	"PUEBLO",	"CO",	38,	27,	34,	-104,	10,	53,	1600 },
  { 93772,	"KRAX",	"RALEIGH/DUR",	"NC",	35,	39,	56,	-78,	29,	23,	106 },
  { 53104,	"KRGX",	"RENO",	         "NV",	39,	45,	19,	-119,	27,	44,	2530 },
  { 24061,	"KRIW",	"RIVERTON",	"WY",	43,	3,	58,	-108,	28,	38,	1697 },
  { 53834,	"KRLX",	"CHARLESTON",	"WV",	38,	18,	40,	-81,	43,	23,	329 },
  { 54763,	"KRMX",	"GRIFFISS_AFB",	"NY",	43,	28,	4,	-75,	27,	29,	462 },
  { 94288,	"KRTX",	"PORTLAND",	"OR",	45,	42,	53,	-122,	57,	56,	479 },
  { 4107,	"KSFX",	"POCATELLO",	"ID",	43,	6,	21,	-112,	41,	10,	1364 },
  { 13995,	"KSGF",	"SPRINGFIELD",	"MO",	37,	14,	7,	-93,	24,	2,	390 },
  { 13957,	"KSHV",	"SHREVEPORT",	"LA",	32,	27,	3,	-93,	50,	29,	83 },
  { 23034,	"KSJT",	"SAN_ANGELO",	"TX",	31,	22,	17,	-100,	29,	33,	576 },
  { 53117,	"KSOX",	"SANTA_ANA_MT",	"CA",	33,	49,	4,	-117,	38,	9,	923 },
  { 92801,	"KTBW",	"TAMPA",	"FL",	27,	42,	20,	-82,	24,	6,	12 },
  { 4102,	"KTFX",	"GREAT_FALLS",	"MT",	47,	27,	35,	-111,	23,	7,	1132 },
  { 93805,	"KTLH",	"TALLAHASSEE",	"FL",	30,	23,	51,	-84,	19,	44,	19 },
  { 3979,	"KTLX",	"OKLAHOMA_CITY","OK",	35,	19,	59,	-97,	16,	40,	370 },
  { 3986,	"KTWX",	"TOPEKA",	"KS",	38,	59,	49,	-96,	13,	57,	417 },
  { 94047,	"KUDX",	"RAPID_CITY",	"SD",	44,	7,	30,	-102,	49,	47,	919 },
  { 94981,	"KUEX",	"HASTINGS",	"NE",	40,	19,	15,	-98,	26,	31,	602 },
  { 53856,	"KVAX",	"MOODY_AFB",	"GA",	30,	53,	25,	-83,	0,	6,	54 },
  { 94234,	"KVBX",	"VANDENBRG_AFB","CA",	34,	50,	17,	-120,	23,	45,	376 },
  { 3995,	"KVNX",	"VANCE_AFB",	"OK",	36,	44,	27,	-98,	7,	40,	369 },
  { 53101,	"KVTX",	"LOS_ANGELES",	"CA",	34,	24,	42,	-119,	10,	47,	831 },
  { 53116,	"KYUX",	"YUMA",	        "AZ",	32,	29,	43,	-114,	39,	24,	53 },
  { 26548,	"PAHG",	"ANCHORAGE",	"AK",	60,	43,	33,	-151,	21,	5,	74 },
  { 25404,	"PAIH",	"MIDDLETON_IS",	"AK",	59,	27,	41,	-146,	18,	11,	20 },
  { 24690,	"PAPD",	"FAIRBANKS",	"AK",	65,	2,	6,	-147,	30,	6,	790 },
  { 41417,	"PGUA",	"ANDERSEN_AFB",	"GU",	13,	27,	16,	144,	48,	30,	80 },
  { 22548,	"PHKI",	"SOUTH_KAUAI",	"HI",	21,	53,	39,	-159,	33,	8,	55 },
  { 43216,	"RKSG",	"CMP_HUMPHRYS",	"KOR",	36,	57,	21,	127,	1,	16,	16 },
  { 42219,	"RODN",	"KADENA",	"OKI",	26,	18,	7,	127,	54,	35,	66 },
  { 33771,	"KDGX",	"JACKSON",	"MS",	32,	16,	33,	-89,	58,	48,	151 },
  { 53118,	"KICX",	"CEDAR_CITY",	"UT",	37,	35,	9,	-112,	51,	21,	3231 },
  { 4844,	"KIWX",	"N_INDIANA",	"IN",	41,	21,	0,	-85,	42,	0,	293 },
  { 53906,	"KSRX",	"W_ARKANSAS",	"AR",	35,	16,	48,	-94,	21,	0,	195 },
  { 54776,	"KTYX",	"MONTAGUE",	"NY",	43,	45,	3,	-75,	40,	30,	563 },
  { 0,	"KVWX",	"EVANSVILLE",	"IN",	38,	16,	12,	-87,	43,	12,	190 },
  { 0,	"PHKM",	"KOHALA",	"HI",	20,	7,	12,	-155,	46,	12,	1162 },
  { 0,	"PHMO",	"MOLOKAI",	"HI",	21,	7,	12,	-157,	10,	12,	415 },
  { 0,	"PHWA",	"HAWAII",	"HI",	19,	4,	48,	-155,	34,	12,	421 },
  { 0,	"TJUA",	"SAN_JUAN",	"PR",	18,	7,	12,	-66,	4,	48,	931 }

};

///////////////////////////////////////////////////////////////
// constructor

NexradLoc::NexradLoc()

{
  _id = 0;
  _latDecimalDeg = 0.0;
  _lonDecimalDeg = 0.0;
  _htMeters = 0.0;
}

///////////////////////////////////////////////////////////////
// destructor

NexradLoc::~NexradLoc()

{

}

///////////////////////////////////////////////////////////////
// load up location from the station id

int NexradLoc::loadLocationFromId(int id)

{

  for (int ii = 0; ii < _nStations; ii++) {
    if (id == _locInfo[ii].id) {
      _load(ii);
      return 0;
    }
  }

  return -1;

}

///////////////////////////////////////////////////////////////
// load up location from the station name

int NexradLoc::loadLocationFromName(const string &name)

{

  for (int ii = 0; ii < _nStations; ii++) {
    string tableName = _locInfo[ii].name;
    if (tableName.compare(name) == 0) {
      _load(ii);
      return 0;
    }
  }

  return -1;

}

///////////////////////////////////////////////////////////////
// load up location from the file path

int NexradLoc::loadLocationFromFilePath(const string &path)

{

  RadxPath fpath(path);
  string fileName = fpath.getFile();

  for (int ii = 0; ii < _nStations; ii++) {
    string tableName = _locInfo[ii].name;
    if (fileName.find(tableName) != string::npos) {
      _load(ii);
      return 0;
    }
  }

  return -1;

}

///////////////////////////////////////////////////////////////
// load up location based on index

void NexradLoc::_load(int index)

{

  if (index >= _nStations) {
    return;
  }
  
  const locInfo_t &loc = _locInfo[index];

  _name = loc.name;
  _city = loc.city;
  _state = loc.state;
  _latDecimalDeg = fabs(loc.latDeg) + (loc.latMin / 60.0) + (loc.latSec / 3600.0);
  if (loc.latDeg < 0) {
    _latDecimalDeg *= -1.0;
  }
  _lonDecimalDeg = fabs(loc.lonDeg) + (loc.lonMin / 60.0) + (loc.lonSec / 3600.0);
  if (loc.lonDeg < 0) {
    _lonDecimalDeg *= -1.0;
  }
  _htMeters = loc.htMeters;

}

