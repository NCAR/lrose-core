
Example of user defined function:


class MyMessage : public QObject
{
    Q_OBJECT
public:
    Q_PROPERTY(QString author MEMBER m_author) //  NOTIFY authorChanged)
    Q_PROPERTY(QVariantList avec MEMBER m_avec) // NOTIFY avecChanged)

    Q_PROPERTY(int anInt MEMBER m_int)
    Q_PROPERTY(int *anIntArray MEMBER m_intArray)
    Q_PROPERTY(QVector<int> qilist MEMBER m_qilist)  <<=== make each field a QVector property of radar volume,
 							   then the field is accessible as a JavaScript array.


Setup the engine like this ... 
Bring in the field values ...

QJSValue messageObj = myEngine.newQObject(new MyMessage);
myEngine.globalObject().setProperty("message", messageObj);


Write the script like this ...
  QString fworks = "function incr(val) { return val + 1 }; [8,9,10].map(incr)"; // works
  QString f = "function incr(val) { return val + 1 }; message.qilist.map(incr)";
  QJSValue result = myEngine.evaluate(f);
  std::cout << "result = " << result.toString().toStdString() << std::endl;



export PKG_CONFIG_PATH="/usr/local/opt/qt/lib/pkgconfig"

testing ...
HawkEye -f test_area/hawkeye/data/20170408/cfrad.20170408_001452.962_to_20170408_002320.954_KARX_Surveillance_SUR.nc -vv

Steps to add a Solo function

1. Add to Solo library
2. Add to SoloFunctionsController.cc
3. Add to SoloFunctionsModel.cc
4. Add to Resources function dictionary 
5. Add to ScriptEditorController.cc
---

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
try with/without boundary ...

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

BAD_DATA = -9e+33
CLIP_GATE = 904 
BFM = SET_BAD_FLAGS_BELOW(VEL, 0.0, BAD_DATA, CLIP_GATE)
BFM2 = AND_BAD_FLAGS_ABOVE(REF, 8.0, BFM, BAD_DATA, CLIP_GATE)

----


BAD_DATA = -9e+33
CLIP_GATE = 904 
BFM = COPY_BAD_FLAGS(VEL, BAD_DATA, CLIP_GATE)
BFM2 = OR_BAD_FLAGS_ABOVE(REF, 8.0, BFM, BAD_DATA, CLIP_GATE)

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

------

BF = ZERO_INSIDE_BOUNDARY(VEL)
