/****************************************************************************
** Meta object code from reading C++ file 'CartManager.hh'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "CartManager.hh"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CartManager.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CartManager_t {
    QByteArrayData data[68];
    char stringdata0[1056];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CartManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CartManager_t qt_meta_stringdata_CartManager = {
    {
QT_MOC_LITERAL(0, 0, 11), // "CartManager"
QT_MOC_LITERAL(1, 12, 24), // "colorMapRedefineReceived"
QT_MOC_LITERAL(2, 37, 0), // ""
QT_MOC_LITERAL(3, 38, 6), // "string"
QT_MOC_LITERAL(4, 45, 9), // "fieldName"
QT_MOC_LITERAL(5, 55, 8), // "ColorMap"
QT_MOC_LITERAL(6, 64, 11), // "newColorMap"
QT_MOC_LITERAL(7, 76, 9), // "gridColor"
QT_MOC_LITERAL(8, 86, 13), // "emphasisColor"
QT_MOC_LITERAL(9, 100, 15), // "annotationColor"
QT_MOC_LITERAL(10, 116, 15), // "backgroundColor"
QT_MOC_LITERAL(11, 132, 9), // "setVolume"
QT_MOC_LITERAL(12, 142, 7), // "_freeze"
QT_MOC_LITERAL(13, 150, 7), // "_unzoom"
QT_MOC_LITERAL(14, 158, 8), // "_refresh"
QT_MOC_LITERAL(15, 167, 12), // "_changeField"
QT_MOC_LITERAL(16, 180, 7), // "fieldId"
QT_MOC_LITERAL(17, 188, 7), // "guiMode"
QT_MOC_LITERAL(18, 196, 17), // "_createSweepPanel"
QT_MOC_LITERAL(19, 214, 24), // "_createSweepRadioButtons"
QT_MOC_LITERAL(20, 239, 23), // "_clearSweepRadioButtons"
QT_MOC_LITERAL(21, 263, 12), // "_changeSweep"
QT_MOC_LITERAL(22, 276, 5), // "value"
QT_MOC_LITERAL(23, 282, 23), // "_changeSweepRadioButton"
QT_MOC_LITERAL(24, 306, 21), // "_horizLocationClicked"
QT_MOC_LITERAL(25, 328, 3), // "xkm"
QT_MOC_LITERAL(26, 332, 3), // "ykm"
QT_MOC_LITERAL(27, 336, 14), // "const RadxRay*"
QT_MOC_LITERAL(28, 351, 10), // "closestRay"
QT_MOC_LITERAL(29, 362, 20), // "_vertLocationClicked"
QT_MOC_LITERAL(30, 383, 16), // "_locationClicked"
QT_MOC_LITERAL(31, 400, 3), // "ray"
QT_MOC_LITERAL(32, 404, 12), // "_setRealtime"
QT_MOC_LITERAL(33, 417, 7), // "enabled"
QT_MOC_LITERAL(34, 425, 20), // "_setArchiveStartTime"
QT_MOC_LITERAL(35, 446, 8), // "RadxTime"
QT_MOC_LITERAL(36, 455, 5), // "rtime"
QT_MOC_LITERAL(37, 461, 18), // "_setArchiveEndTime"
QT_MOC_LITERAL(38, 480, 27), // "_setArchiveStartTimeFromGui"
QT_MOC_LITERAL(39, 508, 3), // "qdt"
QT_MOC_LITERAL(40, 512, 25), // "_setArchiveEndTimeFromGui"
QT_MOC_LITERAL(41, 538, 15), // "_acceptGuiTimes"
QT_MOC_LITERAL(42, 554, 15), // "_cancelGuiTimes"
QT_MOC_LITERAL(43, 570, 8), // "_goBack1"
QT_MOC_LITERAL(44, 579, 7), // "_goFwd1"
QT_MOC_LITERAL(45, 587, 13), // "_goBackPeriod"
QT_MOC_LITERAL(46, 601, 12), // "_goFwdPeriod"
QT_MOC_LITERAL(47, 614, 27), // "_setArchiveRetrievalPending"
QT_MOC_LITERAL(48, 642, 16), // "_showTimeControl"
QT_MOC_LITERAL(49, 659, 17), // "_placeTimeControl"
QT_MOC_LITERAL(50, 677, 26), // "_timeSliderActionTriggered"
QT_MOC_LITERAL(51, 704, 6), // "action"
QT_MOC_LITERAL(52, 711, 23), // "_timeSliderValueChanged"
QT_MOC_LITERAL(53, 735, 19), // "_timeSliderReleased"
QT_MOC_LITERAL(54, 755, 18), // "_timeSliderPressed"
QT_MOC_LITERAL(55, 774, 31), // "_circleRadiusSliderValueChanged"
QT_MOC_LITERAL(56, 806, 30), // "_brushRadiusSliderValueChanged"
QT_MOC_LITERAL(57, 837, 16), // "_saveImageToFile"
QT_MOC_LITERAL(58, 854, 11), // "interactive"
QT_MOC_LITERAL(59, 866, 25), // "_createRealtimeImageFiles"
QT_MOC_LITERAL(60, 892, 24), // "_createArchiveImageFiles"
QT_MOC_LITERAL(61, 917, 26), // "_createImageFilesAllSweeps"
QT_MOC_LITERAL(62, 944, 17), // "_createImageFiles"
QT_MOC_LITERAL(63, 962, 24), // "_createFileChooserDialog"
QT_MOC_LITERAL(64, 987, 25), // "_refreshFileChooserDialog"
QT_MOC_LITERAL(65, 1013, 22), // "_showFileChooserDialog"
QT_MOC_LITERAL(66, 1036, 15), // "ShowContextMenu"
QT_MOC_LITERAL(67, 1052, 3) // "pos"

    },
    "CartManager\0colorMapRedefineReceived\0"
    "\0string\0fieldName\0ColorMap\0newColorMap\0"
    "gridColor\0emphasisColor\0annotationColor\0"
    "backgroundColor\0setVolume\0_freeze\0"
    "_unzoom\0_refresh\0_changeField\0fieldId\0"
    "guiMode\0_createSweepPanel\0"
    "_createSweepRadioButtons\0"
    "_clearSweepRadioButtons\0_changeSweep\0"
    "value\0_changeSweepRadioButton\0"
    "_horizLocationClicked\0xkm\0ykm\0"
    "const RadxRay*\0closestRay\0"
    "_vertLocationClicked\0_locationClicked\0"
    "ray\0_setRealtime\0enabled\0_setArchiveStartTime\0"
    "RadxTime\0rtime\0_setArchiveEndTime\0"
    "_setArchiveStartTimeFromGui\0qdt\0"
    "_setArchiveEndTimeFromGui\0_acceptGuiTimes\0"
    "_cancelGuiTimes\0_goBack1\0_goFwd1\0"
    "_goBackPeriod\0_goFwdPeriod\0"
    "_setArchiveRetrievalPending\0"
    "_showTimeControl\0_placeTimeControl\0"
    "_timeSliderActionTriggered\0action\0"
    "_timeSliderValueChanged\0_timeSliderReleased\0"
    "_timeSliderPressed\0_circleRadiusSliderValueChanged\0"
    "_brushRadiusSliderValueChanged\0"
    "_saveImageToFile\0interactive\0"
    "_createRealtimeImageFiles\0"
    "_createArchiveImageFiles\0"
    "_createImageFilesAllSweeps\0_createImageFiles\0"
    "_createFileChooserDialog\0"
    "_refreshFileChooserDialog\0"
    "_showFileChooserDialog\0ShowContextMenu\0"
    "pos"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CartManager[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      45,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    6,  239,    2, 0x0a /* Public */,
      11,    0,  252,    2, 0x0a /* Public */,
      12,    0,  253,    2, 0x08 /* Private */,
      13,    0,  254,    2, 0x08 /* Private */,
      14,    0,  255,    2, 0x08 /* Private */,
      15,    2,  256,    2, 0x08 /* Private */,
      15,    1,  261,    2, 0x28 /* Private | MethodCloned */,
      18,    0,  264,    2, 0x08 /* Private */,
      19,    0,  265,    2, 0x08 /* Private */,
      20,    0,  266,    2, 0x08 /* Private */,
      21,    1,  267,    2, 0x08 /* Private */,
      23,    1,  270,    2, 0x08 /* Private */,
      24,    3,  273,    2, 0x08 /* Private */,
      29,    3,  280,    2, 0x08 /* Private */,
      30,    3,  287,    2, 0x08 /* Private */,
      32,    1,  294,    2, 0x08 /* Private */,
      34,    1,  297,    2, 0x08 /* Private */,
      37,    1,  300,    2, 0x08 /* Private */,
      38,    1,  303,    2, 0x08 /* Private */,
      40,    1,  306,    2, 0x08 /* Private */,
      41,    0,  309,    2, 0x08 /* Private */,
      42,    0,  310,    2, 0x08 /* Private */,
      43,    0,  311,    2, 0x08 /* Private */,
      44,    0,  312,    2, 0x08 /* Private */,
      45,    0,  313,    2, 0x08 /* Private */,
      46,    0,  314,    2, 0x08 /* Private */,
      47,    0,  315,    2, 0x08 /* Private */,
      48,    0,  316,    2, 0x08 /* Private */,
      49,    0,  317,    2, 0x08 /* Private */,
      50,    1,  318,    2, 0x08 /* Private */,
      52,    1,  321,    2, 0x08 /* Private */,
      53,    0,  324,    2, 0x08 /* Private */,
      54,    0,  325,    2, 0x08 /* Private */,
      55,    1,  326,    2, 0x08 /* Private */,
      56,    1,  329,    2, 0x08 /* Private */,
      57,    1,  332,    2, 0x08 /* Private */,
      57,    0,  335,    2, 0x28 /* Private | MethodCloned */,
      59,    0,  336,    2, 0x08 /* Private */,
      60,    0,  337,    2, 0x08 /* Private */,
      61,    0,  338,    2, 0x08 /* Private */,
      62,    0,  339,    2, 0x08 /* Private */,
      63,    0,  340,    2, 0x08 /* Private */,
      64,    0,  341,    2, 0x08 /* Private */,
      65,    0,  342,    2, 0x08 /* Private */,
      66,    1,  343,    2, 0x08 /* Private */,

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
    QMetaType::Void, 0x80000000 | 35,   36,
    QMetaType::Void, 0x80000000 | 35,   36,
    QMetaType::Void, QMetaType::QDateTime,   39,
    QMetaType::Void, QMetaType::QDateTime,   39,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   51,
    QMetaType::Void, QMetaType::Int,   22,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   22,
    QMetaType::Void, QMetaType::Int,   22,
    QMetaType::Void, QMetaType::Bool,   58,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QPoint,   67,

       0        // eod
};

void CartManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CartManager *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->colorMapRedefineReceived((*reinterpret_cast< string(*)>(_a[1])),(*reinterpret_cast< ColorMap(*)>(_a[2])),(*reinterpret_cast< QColor(*)>(_a[3])),(*reinterpret_cast< QColor(*)>(_a[4])),(*reinterpret_cast< QColor(*)>(_a[5])),(*reinterpret_cast< QColor(*)>(_a[6]))); break;
        case 1: _t->setVolume(); break;
        case 2: _t->_freeze(); break;
        case 3: _t->_unzoom(); break;
        case 4: _t->_refresh(); break;
        case 5: _t->_changeField((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 6: _t->_changeField((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->_createSweepPanel(); break;
        case 8: _t->_createSweepRadioButtons(); break;
        case 9: _t->_clearSweepRadioButtons(); break;
        case 10: _t->_changeSweep((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 11: _t->_changeSweepRadioButton((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->_horizLocationClicked((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< const RadxRay*(*)>(_a[3]))); break;
        case 13: _t->_vertLocationClicked((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< const RadxRay*(*)>(_a[3]))); break;
        case 14: _t->_locationClicked((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< const RadxRay*(*)>(_a[3]))); break;
        case 15: _t->_setRealtime((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 16: _t->_setArchiveStartTime((*reinterpret_cast< const RadxTime(*)>(_a[1]))); break;
        case 17: _t->_setArchiveEndTime((*reinterpret_cast< const RadxTime(*)>(_a[1]))); break;
        case 18: _t->_setArchiveStartTimeFromGui((*reinterpret_cast< const QDateTime(*)>(_a[1]))); break;
        case 19: _t->_setArchiveEndTimeFromGui((*reinterpret_cast< const QDateTime(*)>(_a[1]))); break;
        case 20: _t->_acceptGuiTimes(); break;
        case 21: _t->_cancelGuiTimes(); break;
        case 22: _t->_goBack1(); break;
        case 23: _t->_goFwd1(); break;
        case 24: _t->_goBackPeriod(); break;
        case 25: _t->_goFwdPeriod(); break;
        case 26: _t->_setArchiveRetrievalPending(); break;
        case 27: _t->_showTimeControl(); break;
        case 28: _t->_placeTimeControl(); break;
        case 29: _t->_timeSliderActionTriggered((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 30: _t->_timeSliderValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 31: _t->_timeSliderReleased(); break;
        case 32: _t->_timeSliderPressed(); break;
        case 33: _t->_circleRadiusSliderValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 34: _t->_brushRadiusSliderValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 35: _t->_saveImageToFile((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 36: _t->_saveImageToFile(); break;
        case 37: _t->_createRealtimeImageFiles(); break;
        case 38: _t->_createArchiveImageFiles(); break;
        case 39: _t->_createImageFilesAllSweeps(); break;
        case 40: _t->_createImageFiles(); break;
        case 41: _t->_createFileChooserDialog(); break;
        case 42: _t->_refreshFileChooserDialog(); break;
        case 43: _t->_showFileChooserDialog(); break;
        case 44: _t->ShowContextMenu((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CartManager::staticMetaObject = { {
    QMetaObject::SuperData::link<DisplayManager::staticMetaObject>(),
    qt_meta_stringdata_CartManager.data,
    qt_meta_data_CartManager,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CartManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CartManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CartManager.stringdata0))
        return static_cast<void*>(this);
    return DisplayManager::qt_metacast(_clname);
}

int CartManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DisplayManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 45)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 45;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 45)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 45;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
