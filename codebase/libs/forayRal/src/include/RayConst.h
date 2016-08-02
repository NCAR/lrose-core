//
//
//
//

#ifndef RAYCONSTANTS
#define RAYCONSTANTS

class RayConst {
public:

    RayConst();
    ~RayConst();

    static const int   scanModeCAL = 0;
    static const int   scanModePPI = 1;
    static const int   scanModeCOP = 2;
    static const int   scanModeRHI = 3;
    static const int   scanModeVER = 4;
    static const int   scanModeTAR = 5;
    static const int   scanModeMAN = 6;
    static const int   scanModeIDL = 7;
    static const int   scanModeSUR = 8;
    static const int   scanModeAIR = 9;
    static const int   scanModeHOR = 10;

    static const char  *scanModes[];

    static const int   radarTypeGround            = 0;
    static const int   radarTypeAirborneFore      = 1;
    static const int   radarTypeAirborneAft       = 2;
    static const int   radarTypeAirborneTail      = 3;
    static const int   radarTypeAirborneFuselage  = 4;
    static const int   radarTypeShipborne         = 5;

    static const char  *radarTypes[];

    static const int   rayStatusNormal     = 0;
    static const int   rayStatusTransition = 1;
    static const int   rayStatusBad        = 2;

    static const int   cellSpacingByVector  = 0;
    static const int   cellSpacingBySegment = 1;

    static const int   binaryFormatUnknown    = 0;
    static const int   binaryFormat1ByteInt   = 1;
    static const int   binaryFormat2ByteInt   = 2;
    static const int   binaryFormat3ByteInt   = 3;
    static const int   binaryFormat4ByteFloat = 4;
    static const int   binaryFormat2ByteFloat = 5;

    static const int   unitsUnknown       = 0;
    static const int   unitsNone          = 1;
    static const int   unitsReflectivity  = 2;
    static const int   unitsPower         = 3;
    static const int   unitsVelocity      = 4;
    static const int   unitsLogCounts      = 5;
    static const int   unitsDigitizerCounts = 6;

    static const int   horizontalPolarization    = 0;    
    static const int   verticalPolarization      = 1;
    static const int   circularPolarization      = 2;
    static const int   ellipticalPolarization    = 3;
    static const int   noPolarization            = 4;
    static const int   unknownPolarization       = 5;
    static const int   parallelPolarization      = 6;
    static const int   perpendicularPolarization = 7;

    static const int maxShort;
    static const int minShort;
    static const int badShort;
    static const double badDouble;

};

#endif  // RAYCONSTANTS
