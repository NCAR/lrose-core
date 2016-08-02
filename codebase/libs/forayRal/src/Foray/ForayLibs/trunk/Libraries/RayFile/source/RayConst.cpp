//
//
//
//

#include "RayConst.h"

const char *RayConst::scanModes[] = {"CAL","PPI","COP","RHI","VER","TAR","MAN","IDL","SUR","AIR","HOR"};
const char *RayConst::radarTypes[] = {"Ground","AirborneFore","AirBorneAft","AirBorneTail","AirborneFuselage","Shipborne"};

const int RayConst::maxShort =  32767;
const int RayConst::minShort = -32767;
const int RayConst::badShort = -32768;
const double RayConst::badDouble = -9999;

RayConst::RayConst(){

}

RayConst::~RayConst(){

}
