//
//
//
//

#include "Dorade.h"

// RADD
const char *Dorade::radarTypes[] = {"Ground","AirborneFore","AirBorneAft","AirBorneTail","AirborneFuselage","Shipborne"};
const char *Dorade::scanModes[] = {"CAL","PPI","COP","RHI","VER","TAR","MAN","IDL","SUR","AIR","HOR"};

// PARM
const char *Dorade::binaryFormats[] = {"Unknown","8BitInt","16BitInt","24BitInt","32BitFloat","16BitFloat"};
const char *Dorade::polarization[] = {"Horizaontal","Vertical","Circular","Elliptical","No Polarization","Unknown",
				      "Parallel","Perpendicular"};

// RYIB
const char *Dorade::rayStatus[] = {"Normal","Transition","Bad"};
