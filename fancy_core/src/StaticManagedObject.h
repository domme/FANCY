#ifndef INCLUDE_STATICMANAGEDOBJECT_H
#define INCLUDE_STATICMANAGEDOBJECT_H

#include "ObjectName.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  template <class T>
  class StaticManagedObject
  {
    public:
      typedef std::map<ObjectName, T> MapType;

      static bool registerWithName(const ObjectName& _name, const T& _object)
      {
        MapType::const_iterator it = m_objectMap.find(_name);
        if (it != m_objectMap.end())
        {
          return false;
        }

        m_objectMap.insert(std::pair<ObjectName, T>(_name, _object));
        return true;
      }
//---------------------------------------------------------------------------//
      static T* getByName(const ObjectName& _name)
      {
        MapType::const_iterator it = m_objectMap.find(_name);
        if (it == m_objectMap.end())
        {
          return nullptr;
        }

        return &(*m_objectMap).second;
      }
//---------------------------------------------------------------------------//
    protected:
      static MapType m_objectMap;
  };
//---------------------------------------------------------------------------//
    template<class T>
    std::map<ObjectName, T> StaticManagedObject<T>::m_objectMap;
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
    template <class T>
    class StaticManagedHeapObject
    {
    public:
      typedef std::map<ObjectName, T*> MapType;

      static bool registerWithName(const ObjectName& _name, T* _object)
      {
        MapType::const_iterator it = m_objectMap.find(_name);
        if (it != m_objectMap.end())
        {
          return false;
        }

        m_objectMap.insert(std::pair<ObjectName, T*>(_name, _object));
        return true;
      }
      //---------------------------------------------------------------------------//
      static T* getByName(const ObjectName& _name)
      {
        MapType::const_iterator it = m_objectMap.find(_name);
        if (it == m_objectMap.end())
        {
          return nullptr;
        }

        return (*it).second;
      }
      //---------------------------------------------------------------------------//
    protected:
      static  MapType m_objectMap;
    };
    //---------------------------------------------------------------------------//
    template<class T>
    std::map<ObjectName, T*> StaticManagedHeapObject<T>::m_objectMap;
//---------------------------------------------------------------------------//
}  // end of namespace Fancy

#endif  // INCLUDE_STATICMANAGEDOBJECT_H