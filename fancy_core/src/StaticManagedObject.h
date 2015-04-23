#ifndef INCLUDE_STATICMANAGEDOBJECT_H
#define INCLUDE_STATICMANAGEDOBJECT_H

#include "ObjectName.h"
#include "BinaryCache.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  template <class T>
  class StaticManagedObject
  {
    public:
      typedef std::map<ObjectName, T> MapType;
//---------------------------------------------------------------------------//
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
      static bool registerWithName(const T& _object)
      {
        return registerWithName(_object.getName(), _object);
      }
//---------------------------------------------------------------------------//
      static T* getByName(const ObjectName& _name, bool _shouldTryLoad = false)
      {
        MapType::iterator it = m_objectMap.find(_name);
        if (it != m_objectMap.end())
        {
          return &(*it).second;
        }

        if (_shouldTryLoad)
        {
          return IO::BinaryCache::loadOrRetrieve<T>(_name);
        }
        
        return nullptr;
      }
//---------------------------------------------------------------------------//
      static const MapType& getRegisterMap()
      {
        return m_objectMap;
      }
//---------------------------------------------------------------------------//
      static T* find(std::function<bool(const T&)> _predicateFunc)
      {
        for (MapType::iterator it = m_objectMap.begin(); it != m_objectMap.end(); ++it)
        {
          if (_predicateFunc(it->second))
          {
            return &(it->second);
          }
        }

        return nullptr;
      }
//---------------------------------------------------------------------------//
      static T* findEqual(const T& _other)
      {
        for (MapType::iterator it = m_objectMap.begin(); it != m_objectMap.end(); ++it)
        {
          if ((*it).second == _other)
          {
            return;
          }
        }

        return nullptr;
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
//---------------------------------------------------------------------------//
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
      static bool registerWithName(T* _object)
      {
        return registerWithName(_object->getName(), _object);
      }
//---------------------------------------------------------------------------//
      static T* getByName(const ObjectName& _name, bool _shouldTryLoad = false)
      {
        MapType::const_iterator it = m_objectMap.find(_name);
        if (it != m_objectMap.end())
        {
          return (*it).second;
        }

        if (_shouldTryLoad)
        {
          return IO::BinaryCache::loadOrRetrieve<T>(_name);
        }

        return nullptr;
      }
//---------------------------------------------------------------------------//
      static const MapType& getRegisterMap()
      {
        return m_objectMap;
      }
//---------------------------------------------------------------------------//
      static T* find(std::function<bool(T*)> _predicateFunc)
      {
        for (MapType::iterator it = m_objectMap.begin(); it != m_objectMap.end(); ++it)
        {
          if (_predicateFunc(it->second))
          {
            return it->second;
          }
        }

        return nullptr;
      }
//---------------------------------------------------------------------------//
      static T* findEqual(const T& _other)
      {
        for (MapType::iterator it = m_objectMap.begin(); it != m_objectMap.end(); ++it)
        {
          if (*(it->second) == _other)
          {
            return it->second;
          }
        }

        return nullptr;
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