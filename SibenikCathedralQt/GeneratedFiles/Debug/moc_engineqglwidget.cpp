/****************************************************************************
** Meta object code from reading C++ file 'engineqglwidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.2.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../engineqglwidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'engineqglwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.2.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_EngineQGLwidget_t {
    QByteArrayData data[8];
    char stringdata[100];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_EngineQGLwidget_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_EngineQGLwidget_t qt_meta_stringdata_EngineQGLwidget = {
    {
QT_MOC_LITERAL(0, 0, 15),
QT_MOC_LITERAL(1, 16, 9),
QT_MOC_LITERAL(2, 26, 0),
QT_MOC_LITERAL(3, 27, 20),
QT_MOC_LITERAL(4, 48, 11),
QT_MOC_LITERAL(5, 60, 12),
QT_MOC_LITERAL(6, 73, 13),
QT_MOC_LITERAL(7, 87, 11)
    },
    "EngineQGLwidget\0onPaintGL\0\0"
    "onToggleDebugDisplay\0iCheckState\0"
    "onToggleFXAA\0onToggleBloom\0onToggleHDR\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_EngineQGLwidget[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   39,    2, 0x08,
       3,    1,   40,    2, 0x0a,
       5,    1,   43,    2, 0x0a,
       6,    1,   46,    2, 0x0a,
       7,    1,   49,    2, 0x0a,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::Int,    4,

       0        // eod
};

void EngineQGLwidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        EngineQGLwidget *_t = static_cast<EngineQGLwidget *>(_o);
        switch (_id) {
        case 0: _t->onPaintGL(); break;
        case 1: _t->onToggleDebugDisplay((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->onToggleFXAA((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->onToggleBloom((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->onToggleHDR((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject EngineQGLwidget::staticMetaObject = {
    { &QGLWidget::staticMetaObject, qt_meta_stringdata_EngineQGLwidget.data,
      qt_meta_data_EngineQGLwidget,  qt_static_metacall, 0, 0}
};


const QMetaObject *EngineQGLwidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *EngineQGLwidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_EngineQGLwidget.stringdata))
        return static_cast<void*>(const_cast< EngineQGLwidget*>(this));
    return QGLWidget::qt_metacast(_clname);
}

int EngineQGLwidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGLWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
