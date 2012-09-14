#ifndef EVENTS_H
#define EVENTS_H

#include "../includes.h"

template <typename Param1T>
class BaseEvent
{
	public:
		virtual void raiseEvent( Param1T param ) = 0;
};



template<typename ObjectT, typename Param1T>
class Event1Param : public BaseEvent<Param1T>
{
	typedef void (ObjectT::*CallbackFuncT ) ( Param1T );

	public:
		Event1Param(ObjectT* pObject, CallbackFuncT pCallbackFunc )
		{
			m_pCallbackFunc = pCallbackFunc;
			m_pListenerObject = pObject;
		}

		virtual void raiseEvent( Param1T param )
		{
			(m_pListenerObject->*m_pCallbackFunc)( param );
		}

	private:

		CallbackFuncT m_pCallbackFunc;
		ObjectT* m_pListenerObject;
};


template<typename Param1T>
class EventHandler1Param
{
	public:
		EventHandler1Param() : m_iCount( 0 )
		{

		}

		template<typename ObjectT>
		int registerListener( ObjectT* pObject, void (ObjectT::*CallbackFuncT ) ( Param1T ) )
		{
			m_mapListeners[ m_iCount ] = (new Event1Param<ObjectT, Param1T >( pObject, CallbackFuncT ) );
			return m_iCount++;
		}

		bool unregisterListener( int iIndex )
		{
			typename std::map<int, BaseEvent<Param1T>*>::iterator it;
			it = m_mapListeners.find( iIndex );
			if( it != m_mapListeners.end() )
			{
				delete it->second;
				m_mapListeners.erase( it );
				return true;
			}

			return false;
		}

		void raiseEvent( Param1T param )
		{
			typename std::map<int, BaseEvent<Param1T>*>::iterator it;
			it = m_mapListeners.begin();
			for( ; it != m_mapListeners.end(); it++ )
			{
				it->second->raiseEvent( param );
			}
		}

	private:
		std::map<int, BaseEvent<Param1T>*> m_mapListeners;
		int m_iCount;
};		


#endif