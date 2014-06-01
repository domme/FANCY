/****************************************************************************
** Meta object code from reading C++ file 'DeferredRendererQApp.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.2.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../DeferredRendererQApp.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DeferredRendererQApp.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.2.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_DeferredRendererQApp_t {
    QByteArrayData data[7];
    char stringdata[57];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_DeferredRendererQApp_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_DeferredRendererQApp_t qt_meta_stringdata_DeferredRendererQApp = {
    {
QT_MOC_LITERAL(0, 0, 20),
QT_MOC_LITERAL(1, 21, 8),
QT_MOC_LITERAL(2, 30, 0),
QT_MOC_LITERAL(3, 31, 10),
QT_MOC_LITERAL(4, 42, 1),
QT_MOC_LITERAL(5, 44, 1),
QT_MOC_LITERAL(6, 46, 9)
    },
    "DeferredRendererQApp\0onGLinit\0\0"
    "onGLresize\0x\0y\0onGLpaint\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DeferredRendererQApp[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   29,    2, 0x0a,
       3,    2,   30,    2, 0x0a,
       6,    0,   35,    2, 0x0a,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    4,    5,
    QMetaType::Void,

       0        // eod
};

void DeferredRendererQApp::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        DeferredRendererQApp *_t = static_cast<DeferredRendererQApp *>(_o);
        switch (_id) {
        case 0: _t->onGLinit(); break;
        case 1: _t->onGLresize((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->onGLpaint(); break;
        default: ;
        }
    }
}

const QMetaObject DeferredRendererQApp::staticMetaObject = {
    { &QApplication::staticMetaObject, qt_meta_stringdata_DeferredRendererQApp.data,
      qt_meta_data_DeferredRendererQApp,  qt_static_metacall, 0, 0}
};


const QMetaObject *DeferredRendererQApp::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DeferredRendererQApp::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DeferredRendererQApp.stringdata))
        return static_cast<void*>(const_cast< DeferredRendererQApp*>(this));
    return QApplication::qt_metacast(_clname);
}

int DeferredRendererQApp::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QApplication::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
