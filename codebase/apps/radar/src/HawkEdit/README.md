build from instructions for manual build

export PKG_CONFIG_PATH="/usr/local/opt/qt/lib/pkgconfig"

---
cp  HawkEyeEdit_Elle.app/Contents/MacOS/HawkEyeEdit_Elle  ~/lrose/bin

~/lrose/bin/HawkEyeEdit_Elle -f ~/test_area/hawkeye/data/20170408/cfrad.20170408_001452.962_to_20170408_002320.954_KARX_Surveillance_SUR.nc

---
$ cp  HawkEye.app/Contents/MacOS/HawkEye  ~/lrose/bin

testing ...
HawkEye -f /Users/brenda/test_area/hawkeye/data/20170408/cfrad.20170408_001452.962_to_20170408_002320.954_KARX_Surveillance_SUR.nc -vv

Steps to add a Solo function

1. Add to Solo library
2. Add to SoloFunctionsController.cc
3. Add to SoloFunctionsModel.cc
4. Add to Resources function dictionary 
5. Add to ScriptEditorController.cc
---

Notes on F(x) or vector functions:

VEL + VEL2;
VEL * 2+DBZ sin (R);

  vector<double> RemoveAircraftMotion(string fieldName, RadxVol *vol,
				      int rayIdx, int sweepIdx);
CANNOT return vector<double> it is a huge vector; need to return string : name of new/temp field
BUT! the QEngine will evaluate the expression, but it needs the data values. or will it call into a
separate function as needed?

------

testing with this script ...
with 20170408 data

BAD_DATA = -9e+33
CLIP_GATE = 904
A_SPECKLE = 7
SPK = DESPECKLE(VEL, A_SPECKLE, BAD_DATA, CLIP_GATE)

----
 
testing RemoveAcMotion

How to get the correction factors?  Which data file?

RadxPrint --print_params ...
apply_georeference_corrections = TRUE; 

-----

testing bad flags 

BAD_DATA = -9e+33
CLIP_GATE = 904 
BFM = SET_BAD_FLAGS_ABOVE(VEL, 6.0, BAD_DATA, CLIP_GATE)
BFM2 = COMPLEMENT_BAD_FLAGS(BFM)
RESULT = ASSERT_BAD_FLAGS(VEL, BAD_DATA, CLIP_GATE, BFM2)

------

BAD_DATA = -777e+33
CLIP_GATE = 104 
BFM = SET_BAD_FLAGS_BETWEEN(VEL, 6.0, 10.0, BAD_DATA, CLIP_GATE)
BFM2 = COMPLEMENT_BAD_FLAGS(BFM)
BFM3 = CLEAR_BAD_FLAGS(BFM)
BFM4 = SET_BAD_FLAGS_BELOW(VEL, 0.0, BAD_DATA, CLIP_GATE)
VEL_CLEAN = ASSERT_BAD_FLAGS(VEL, BAD_DATA, CLIP_GATE, BFM4)

-----

BAD_DATA = -9e+33
CLIP_GATE = 904 
BFM = SET_BAD_FLAGS_BELOW(VEL, 0.0, BAD_DATA, CLIP_GATE)
BFM2 = FLAGGED_ADD(VEL, 100.0, BAD_DATA, CLIP_GATE, BFM)

----

BAD_DATA = -9e+33
CLIP_GATE = 904 
BFM = SET_BAD_FLAGS_ABOVE(VEL, 0.0, BAD_DATA, CLIP_GATE)
VEL_C = FLAGGED_MULTIPLY(VEL, -1.0, BAD_DATA, CLIP_GATE, BFM)

----
#
# testing default values for bad_data and clip_gate
#
BAD_DATA = -9e+33
CLIP_GATE = 904 
BFM = SET_BAD_FLAGS_BELOW(VEL, 0.0, BAD_DATA, CLIP_GATE)
BFM2 = XOR_BAD_FLAGS_BETWEEN(VEL, 10.0, 15.0, BFM)

----

  colorMap = constructColorMap(0.0, 100.0, "number.colors");
  colorMap.setName("numbers");
  colorMap.setUnits("units");
  ColorMapForUsualParm["BFM"] = colorMap;
-------


    CLIP = 35;
    BAD_DATA_VALUE = -9E+35;
    CLEAR-BAD-FLAGS(BAD_FLAG_MASK)
    BAD-FLAG-MASK = SET-BAD-FLAGS-BETWEEN(VE, -1., 1., BAD_DATA_VALUE, CLIP,  BAD_FLAG_MASK)
    BAD_FLAG_MASK = AND_BAD_FLAGS_ABOVE(DZ, 35., BAD_DATA_VALUE, CLIP,  BAD_FLAG_MASK)
    VE2 = ASSERT-BAD-FLAGS(VE, BAD_DATA_VALUE, CLIP, BAD_FLAG_MASK)
    DZ2 = ASSERT-BAD-FLAGS(DZ, BAD_DATA_VALUE, CLIP, BAD-FLAG-MASK)
