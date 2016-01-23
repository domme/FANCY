#ifndef INCLUDE_STATICMANAGEDOBJECT_H
#define INCLUDE_STATICMANAGEDOBJECT_H

#include "ObjectName.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class BaseManagedObject
  {
    // Currently just a dummy base class as a helper for is_base_of on T
  };
//---------------------------------------------------------------------------//
  template <class T>
  class StaticManagedObject : public BaseManagedObject
  {
    public:
      typedef std::map<uint64, T> MapType;
//---------------------------------------------------------------------------//
      template <class DescT>
      static T* FindFromDesc(const DescT& aDesc)
      {
        return Find(aDesc.GetHash());
      }
//---------------------------------------------------------------------------//
      static bool Register(const T& _object, uint64 aHash)
      {
        ASSERT(nullptr == Find(aHash));
        m_objectMap.insert(std::pair<uint64, T>(aHash, _object));
        return true;
      }
//---------------------------------------------------------------------------//
      static bool Register(const T& _object)
      {
        return Register(_object, _object.GetDescription().GetHash());
      }
//---------------------------------------------------------------------------//
      static T* Find(uint64 aHash)
      {
        auto it = m_objectMap.find(aHash);
        if (it != m_objectMap.end())
          return &(*it).second;
        
        return nullptr;
      }
//---------------------------------------------------------------------------//
      static const MapType& getRegisterMap()
      {
        return m_objectMap;
      }
//---------------------------------------------------------------------------//
      static T* FindWithFunction(std::function<bool(const T&)> _predicateFunc)
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
      static T* FindEqual(const T& _other)
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
    std::map<uint64, T> StaticManagedObject<T>::m_objectMap;
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
    template <class T>
    class StaticManagedHeapObject : public BaseManagedObject
    {
    public:
      typedef std::map<uint64, T*> MapType;
//---------------------------------------------------------------------------//
      static bool Register(T* anObject, uint64 aHash)
      {
        ASSERT(nullptr == Find(aHash));
        m_objectMap.insert(std::pair<uint64, T*>(aHash, anObject));
        return true;
      }
//---------------------------------------------------------------------------//
      static bool Register(T* anObject)
      {
        return Register(anObject, anObject->GetDescription().GetHash());
      }
//---------------------------------------------------------------------------//
      template <class DescT>
      static T* FindFromDesc(const DescT& aDesc)
      {
        return Find(aDesc.GetHash());
      }
//---------------------------------------------------------------------------//
      static T* Find(uint64 aHash)
      {
        auto it = m_objectMap.find(aHash);
        if (it != m_objectMap.end())
          return (*it).second;

        return nullptr;
      }
//---------------------------------------------------------------------------//
      static const MapType& getRegisterMap()
      {
        return m_objectMap;
      }
//---------------------------------------------------------------------------//
      static T* FindWithFunction(std::function<bool(T*)> _predicateFunc)
      {
        for (typename MapType::iterator it = m_objectMap.begin(); it != m_objectMap.end(); ++it)
        {
          if (_predicateFunc(it->second))
          {
            return it->second;
          }
        }

        return nullptr;
      }
//---------------------------------------------------------------------------//
      static T* FindEqual(const T& _other)
      {
        for (typename MapType::iterator it = m_objectMap.begin(); it != m_objectMap.end(); ++it)
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
    std::map<uint64, T*> StaticManagedHeapObject<T>::m_objectMap;
//---------------------------------------------------------------------------//
}  // end of namespace Fancy

#endif  // INCLUDE_STATICMANAGEDOBJECT_H