#include "Serializable.h"

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
    template<class T>
    MetaTableImpl<T> MetaTableImpl<T>::ourMetaTable;

    template<class T>
    MetaTableImpl<T*> MetaTableImpl<T*>::ourMetaTable;
  //---------------------------------------------------------------------------//
} }  // end of namespace Fancy::IO


