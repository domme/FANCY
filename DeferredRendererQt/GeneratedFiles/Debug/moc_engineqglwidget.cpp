/****************************************************************************
** Meta object code from reading C++ file 'engineqglwidget.h'
**
** Created: Sun 4. Nov 18:28:07 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../engineqglwidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'engineqglwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_EngineQGLwidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      17,   16,   16,   16, 0x08,
      41,   29,   16,   16, 0x0a,
      67,   29,   16,   16, 0x0a,
      85,   29,   16,   16, 0x0a,
     104,   29,   16,   16, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_EngineQGLwidget[] = {
    "EngineQGLwidget\0\0onPaintGL()\0iCheckState\0"
    "onToggleDebugDisplay(int)\0onToggleFXAA(int)\0"
    "onToggleBloom(int)\0onToggleHDR(int)\0"
};

void EngineQGLwidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
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

const QMetaObjectExtraData EngineQGLwidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject EngineQGLwidget::staticMetaObject = {
    { &QGLWidget::staticMetaObject, qt_meta_stringdata_EngineQGLwidget,
      qt_meta_data_EngineQGLwidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &EngineQGLwidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *EngineQGLwidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *EngineQGLwidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_EngineQGLwidget))
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
    }
    return _id;
}
QT_END_MOC_NAMESPACE