/****************************************************************************
** Meta object code from reading C++ file 'CartManager.hh'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "CartManager.hh"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CartManager.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.5.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSCartManagerENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSCartManagerENDCLASS = QtMocHelpers::stringData(
    "CartManager",
    "colorMapRedefineReceived",
    "",
    "string",
    "fieldName",
    "ColorMap",
    "newColorMap",
    "gridColor",
    "emphasisColor",
    "annotationColor",
    "backgroundColor",
    "setVolume",
    "_freeze",
    "_unzoom",
    "_refresh",
    "_changeField",
    "fieldId",
    "guiMode",
    "_createSweepPanel",
    "_createSweepRadioButtons",
    "_clearSweepRadioButtons",
    "_changeSweep",
    "value",
    "_changeSweepRadioButton",
    "_horizLocationClicked",
    "xkm",
    "ykm",
    "const RadxRay*",
    "closestRay",
    "_vertLocationClicked",
    "_locationClicked",
    "ray",
    "_setRealtime",
    "enabled",
    "_showFieldMenu",
    "_placeFieldMenu",
    "_setMapsEnabled",
    "enable",
    "_setProductsEnabled",
    "_setWindsEnabled",
    "_showTimeControl",
    "_placeTimeControl",
    "_circleRadiusSliderValueChanged",
    "_brushRadiusSliderValueChanged",
    "_saveImageToFile",
    "interactive",
    "_createRealtimeImageFiles",
    "_createArchiveImageFiles",
    "_createImageFilesAllSweeps",
    "_createImageFiles",
    "_createFileChooserDialog",
    "_refreshFileChooserDialog",
    "_showFileChooserDialog",
    "ShowContextMenu",
    "pos"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSCartManagerENDCLASS_t {
    uint offsetsAndSizes[110];
    char stringdata0[12];
    char stringdata1[25];
    char stringdata2[1];
    char stringdata3[7];
    char stringdata4[10];
    char stringdata5[9];
    char stringdata6[12];
    char stringdata7[10];
    char stringdata8[14];
    char stringdata9[16];
    char stringdata10[16];
    char stringdata11[10];
    char stringdata12[8];
    char stringdata13[8];
    char stringdata14[9];
    char stringdata15[13];
    char stringdata16[8];
    char stringdata17[8];
    char stringdata18[18];
    char stringdata19[25];
    char stringdata20[24];
    char stringdata21[13];
    char stringdata22[6];
    char stringdata23[24];
    char stringdata24[22];
    char stringdata25[4];
    char stringdata26[4];
    char stringdata27[15];
    char stringdata28[11];
    char stringdata29[21];
    char stringdata30[17];
    char stringdata31[4];
    char stringdata32[13];
    char stringdata33[8];
    char stringdata34[15];
    char stringdata35[16];
    char stringdata36[16];
    char stringdata37[7];
    char stringdata38[20];
    char stringdata39[17];
    char stringdata40[17];
    char stringdata41[18];
    char stringdata42[32];
    char stringdata43[31];
    char stringdata44[17];
    char stringdata45[12];
    char stringdata46[26];
    char stringdata47[25];
    char stringdata48[27];
    char stringdata49[18];
    char stringdata50[25];
    char stringdata51[26];
    char stringdata52[23];
    char stringdata53[16];
    char stringdata54[4];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSCartManagerENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSCartManagerENDCLASS_t qt_meta_stringdata_CLASSCartManagerENDCLASS = {
    {
        QT_MOC_LITERAL(0, 11),  // "CartManager"
        QT_MOC_LITERAL(12, 24),  // "colorMapRedefineReceived"
        QT_MOC_LITERAL(37, 0),  // ""
        QT_MOC_LITERAL(38, 6),  // "string"
        QT_MOC_LITERAL(45, 9),  // "fieldName"
        QT_MOC_LITERAL(55, 8),  // "ColorMap"
        QT_MOC_LITERAL(64, 11),  // "newColorMap"
        QT_MOC_LITERAL(76, 9),  // "gridColor"
        QT_MOC_LITERAL(86, 13),  // "emphasisColor"
        QT_MOC_LITERAL(100, 15),  // "annotationColor"
        QT_MOC_LITERAL(116, 15),  // "backgroundColor"
        QT_MOC_LITERAL(132, 9),  // "setVolume"
        QT_MOC_LITERAL(142, 7),  // "_freeze"
        QT_MOC_LITERAL(150, 7),  // "_unzoom"
        QT_MOC_LITERAL(158, 8),  // "_refresh"
        QT_MOC_LITERAL(167, 12),  // "_changeField"
        QT_MOC_LITERAL(180, 7),  // "fieldId"
        QT_MOC_LITERAL(188, 7),  // "guiMode"
        QT_MOC_LITERAL(196, 17),  // "_createSweepPanel"
        QT_MOC_LITERAL(214, 24),  // "_createSweepRadioButtons"
        QT_MOC_LITERAL(239, 23),  // "_clearSweepRadioButtons"
        QT_MOC_LITERAL(263, 12),  // "_changeSweep"
        QT_MOC_LITERAL(276, 5),  // "value"
        QT_MOC_LITERAL(282, 23),  // "_changeSweepRadioButton"
        QT_MOC_LITERAL(306, 21),  // "_horizLocationClicked"
        QT_MOC_LITERAL(328, 3),  // "xkm"
        QT_MOC_LITERAL(332, 3),  // "ykm"
        QT_MOC_LITERAL(336, 14),  // "const RadxRay*"
        QT_MOC_LITERAL(351, 10),  // "closestRay"
        QT_MOC_LITERAL(362, 20),  // "_vertLocationClicked"
        QT_MOC_LITERAL(383, 16),  // "_locationClicked"
        QT_MOC_LITERAL(400, 3),  // "ray"
        QT_MOC_LITERAL(404, 12),  // "_setRealtime"
        QT_MOC_LITERAL(417, 7),  // "enabled"
        QT_MOC_LITERAL(425, 14),  // "_showFieldMenu"
        QT_MOC_LITERAL(440, 15),  // "_placeFieldMenu"
        QT_MOC_LITERAL(456, 15),  // "_setMapsEnabled"
        QT_MOC_LITERAL(472, 6),  // "enable"
        QT_MOC_LITERAL(479, 19),  // "_setProductsEnabled"
        QT_MOC_LITERAL(499, 16),  // "_setWindsEnabled"
        QT_MOC_LITERAL(516, 16),  // "_showTimeControl"
        QT_MOC_LITERAL(533, 17),  // "_placeTimeControl"
        QT_MOC_LITERAL(551, 31),  // "_circleRadiusSliderValueChanged"
        QT_MOC_LITERAL(583, 30),  // "_brushRadiusSliderValueChanged"
        QT_MOC_LITERAL(614, 16),  // "_saveImageToFile"
        QT_MOC_LITERAL(631, 11),  // "interactive"
        QT_MOC_LITERAL(643, 25),  // "_createRealtimeImageFiles"
        QT_MOC_LITERAL(669, 24),  // "_createArchiveImageFiles"
        QT_MOC_LITERAL(694, 26),  // "_createImageFilesAllSweeps"
        QT_MOC_LITERAL(721, 17),  // "_createImageFiles"
        QT_MOC_LITERAL(739, 24),  // "_createFileChooserDialog"
        QT_MOC_LITERAL(764, 25),  // "_refreshFileChooserDialog"
        QT_MOC_LITERAL(790, 22),  // "_showFileChooserDialog"
        QT_MOC_LITERAL(813, 15),  // "ShowContextMenu"
        QT_MOC_LITERAL(829, 3)   // "pos"
    },
    "CartManager",
    "colorMapRedefineReceived",
    "",
    "string",
    "fieldName",
    "ColorMap",
    "newColorMap",
    "gridColor",
    "emphasisColor",
    "annotationColor",
    "backgroundColor",
    "setVolume",
    "_freeze",
    "_unzoom",
    "_refresh",
    "_changeField",
    "fieldId",
    "guiMode",
    "_createSweepPanel",
    "_createSweepRadioButtons",
    "_clearSweepRadioButtons",
    "_changeSweep",
    "value",
    "_changeSweepRadioButton",
    "_horizLocationClicked",
    "xkm",
    "ykm",
    "const RadxRay*",
    "closestRay",
    "_vertLocationClicked",
    "_locationClicked",
    "ray",
    "_setRealtime",
    "enabled",
    "_showFieldMenu",
    "_placeFieldMenu",
    "_setMapsEnabled",
    "enable",
    "_setProductsEnabled",
    "_setWindsEnabled",
    "_showTimeControl",
    "_placeTimeControl",
    "_circleRadiusSliderValueChanged",
    "_brushRadiusSliderValueChanged",
    "_saveImageToFile",
    "interactive",
    "_createRealtimeImageFiles",
    "_createArchiveImageFiles",
    "_createImageFilesAllSweeps",
    "_createImageFiles",
    "_createFileChooserDialog",
    "_refreshFileChooserDialog",
    "_showFileChooserDialog",
    "ShowContextMenu",
    "pos"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSCartManagerENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
      35,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    6,  224,    2, 0x0a,    1 /* Public */,
      11,    0,  237,    2, 0x0a,    8 /* Public */,
      12,    0,  238,    2, 0x08,    9 /* Private */,
      13,    0,  239,    2, 0x08,   10 /* Private */,
      14,    0,  240,    2, 0x08,   11 /* Private */,
      15,    2,  241,    2, 0x08,   12 /* Private */,
      15,    1,  246,    2, 0x28,   15 /* Private | MethodCloned */,
      18,    0,  249,    2, 0x08,   17 /* Private */,
      19,    0,  250,    2, 0x08,   18 /* Private */,
      20,    0,  251,    2, 0x08,   19 /* Private */,
      21,    1,  252,    2, 0x08,   20 /* Private */,
      23,    1,  255,    2, 0x08,   22 /* Private */,
      24,    3,  258,    2, 0x08,   24 /* Private */,
      29,    3,  265,    2, 0x08,   28 /* Private */,
      30,    3,  272,    2, 0x08,   32 /* Private */,
      32,    1,  279,    2, 0x08,   36 /* Private */,
      34,    0,  282,    2, 0x08,   38 /* Private */,
      35,    0,  283,    2, 0x08,   39 /* Private */,
      36,    1,  284,    2, 0x08,   40 /* Private */,
      38,    1,  287,    2, 0x08,   42 /* Private */,
      39,    1,  290,    2, 0x08,   44 /* Private */,
      40,    0,  293,    2, 0x08,   46 /* Private */,
      41,    0,  294,    2, 0x08,   47 /* Private */,
      42,    1,  295,    2, 0x08,   48 /* Private */,
      43,    1,  298,    2, 0x08,   50 /* Private */,
      44,    1,  301,    2, 0x08,   52 /* Private */,
      44,    0,  304,    2, 0x28,   54 /* Private | MethodCloned */,
      46,    0,  305,    2, 0x08,   55 /* Private */,
      47,    0,  306,    2, 0x08,   56 /* Private */,
      48,    0,  307,    2, 0x08,   57 /* Private */,
      49,    0,  308,    2, 0x08,   58 /* Private */,
      50,    0,  309,    2, 0x08,   59 /* Private */,
      51,    0,  310,    2, 0x08,   60 /* Private */,
      52,    0,  311,    2, 0x08,   61 /* Private */,
      53,    1,  312,    2, 0x08,   62 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 5, QMetaType::QColor, QMetaType::QColor, QMetaType::QColor, QMetaType::QColor,    4,    6,    7,    8,    9,   10,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,   16,   17,
    QMetaType::Void, QMetaType::Int,   16,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   22,
    QMetaType::Void, QMetaType::Int,   22,
    QMetaType::Void, QMetaType::Double, QMetaType::Double, 0x80000000 | 27,   25,   26,   28,
    QMetaType::Void, QMetaType::Double, QMetaType::Double, 0x80000000 | 27,   25,   26,   28,
    QMetaType::Void, QMetaType::Double, QMetaType::Double, 0x80000000 | 27,   25,   26,   31,
    QMetaType::Void, QMetaType::Bool,   33,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   37,
    QMetaType::Void, QMetaType::Bool,   37,
    QMetaType::Void, QMetaType::Bool,   37,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   22,
    QMetaType::Void, QMetaType::Int,   22,
    QMetaType::Void, QMetaType::Bool,   45,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QPoint,   54,

       0        // eod
};

Q_CONSTINIT const QMetaObject CartManager::staticMetaObject = { {
    QMetaObject::SuperData::link<DisplayManager::staticMetaObject>(),
    qt_meta_stringdata_CLASSCartManagerENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSCartManagerENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSCartManagerENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<CartManager, std::true_type>,
        // method 'colorMapRedefineReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<string, std::false_type>,
        QtPrivate::TypeAndForceComplete<ColorMap, std::false_type>,
        QtPrivate::TypeAndForceComplete<QColor, std::false_type>,
        QtPrivate::TypeAndForceComplete<QColor, std::false_type>,
        QtPrivate::TypeAndForceComplete<QColor, std::false_type>,
        QtPrivate::TypeAndForceComplete<QColor, std::false_type>,
        // method 'setVolume'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_freeze'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_unzoom'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_refresh'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_changeField'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method '_changeField'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method '_createSweepPanel'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_createSweepRadioButtons'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_clearSweepRadioButtons'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_changeSweep'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method '_changeSweepRadioButton'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method '_horizLocationClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<const RadxRay *, std::false_type>,
        // method '_vertLocationClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<const RadxRay *, std::false_type>,
        // method '_locationClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<const RadxRay *, std::false_type>,
        // method '_setRealtime'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method '_showFieldMenu'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_placeFieldMenu'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_setMapsEnabled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method '_setProductsEnabled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method '_setWindsEnabled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method '_showTimeControl'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_placeTimeControl'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_circleRadiusSliderValueChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method '_brushRadiusSliderValueChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method '_saveImageToFile'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method '_saveImageToFile'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_createRealtimeImageFiles'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_createArchiveImageFiles'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_createImageFilesAllSweeps'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_createImageFiles'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_createFileChooserDialog'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_refreshFileChooserDialog'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method '_showFileChooserDialog'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'ShowContextMenu'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QPoint &, std::false_type>
    >,
    nullptr
} };

void CartManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CartManager *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->colorMapRedefineReceived((*reinterpret_cast< std::add_pointer_t<string>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<ColorMap>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QColor>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<QColor>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<QColor>>(_a[5])),(*reinterpret_cast< std::add_pointer_t<QColor>>(_a[6]))); break;
        case 1: _t->setVolume(); break;
        case 2: _t->_freeze(); break;
        case 3: _t->_unzoom(); break;
        case 4: _t->_refresh(); break;
        case 5: _t->_changeField((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 6: _t->_changeField((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 7: _t->_createSweepPanel(); break;
        case 8: _t->_createSweepRadioButtons(); break;
        case 9: _t->_clearSweepRadioButtons(); break;
        case 10: _t->_changeSweep((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 11: _t->_changeSweepRadioButton((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 12: _t->_horizLocationClicked((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<const RadxRay*>>(_a[3]))); break;
        case 13: _t->_vertLocationClicked((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<const RadxRay*>>(_a[3]))); break;
        case 14: _t->_locationClicked((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<const RadxRay*>>(_a[3]))); break;
        case 15: _t->_setRealtime((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 16: _t->_showFieldMenu(); break;
        case 17: _t->_placeFieldMenu(); break;
        case 18: _t->_setMapsEnabled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 19: _t->_setProductsEnabled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 20: _t->_setWindsEnabled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 21: _t->_showTimeControl(); break;
        case 22: _t->_placeTimeControl(); break;
        case 23: _t->_circleRadiusSliderValueChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 24: _t->_brushRadiusSliderValueChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 25: _t->_saveImageToFile((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 26: _t->_saveImageToFile(); break;
        case 27: _t->_createRealtimeImageFiles(); break;
        case 28: _t->_createArchiveImageFiles(); break;
        case 29: _t->_createImageFilesAllSweeps(); break;
        case 30: _t->_createImageFiles(); break;
        case 31: _t->_createFileChooserDialog(); break;
        case 32: _t->_refreshFileChooserDialog(); break;
        case 33: _t->_showFileChooserDialog(); break;
        case 34: _t->ShowContextMenu((*reinterpret_cast< std::add_pointer_t<QPoint>>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject *CartManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CartManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSCartManagerENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return DisplayManager::qt_metacast(_clname);
}

int CartManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DisplayManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 35)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 35;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 35)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 35;
    }
    return _id;
}
QT_WARNING_POP
