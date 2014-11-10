#ifndef INCLUDE_EVENTS_H
#define INCLUDE_EVENTS_H

#include "FancyCorePrerequisites.h"

/**********************************************************************//**
* This header-file includes different types of events and event-handlers for different
* parameters.
* In this event-system, each Listener-class should store a Listener-Object
* as a member-variable. This Listener is used as an adapter which encapsulates
* the pointer to the listener-object and the member-function pointer to call back.
* The Listener-Adapter object should be stored to ensure unregistration can be done.
************************************************************************/

/**********************************************************************//**
* The Base-Listener is used as a common base-type for all kinds of 
* object-types with the same parameter in their callback-functions
************************************************************************/
template <typename Param1T>
class BaseListener1
{
	public:
		virtual void Notify( const Param1T& param ) = 0;
};


/**********************************************************************//**
* The Listener1 is the template for the actual Listener-Adapter
************************************************************************/
template<typename ObjectT, typename Param1T>
class Listener1 : public BaseListener1<Param1T>
{
	typedef void (ObjectT::*CallbackFuncT ) ( const Param1T& );

	public:
		Listener1() : 
		  m_pCallbackFunc( 0 ), m_pListenerObject( NULL ), BaseListener1<Param1T>()
		{

		}

		Listener1( ObjectT* pObject, CallbackFuncT pCallbackFunc ) :
			m_pCallbackFunc( 0 ), m_pListenerObject( NULL ), BaseListener1<Param1T>()
		{
			Init( pObject, pCallbackFunc );
		}

		void Init( ObjectT* pObject, CallbackFuncT pCallbackFunc )
		{
			m_pCallbackFunc = pCallbackFunc;
			m_pListenerObject = pObject;
		}

		virtual void Notify( const Param1T& param )
		{
			(m_pListenerObject->*m_pCallbackFunc)( param );
		}

	private:

		CallbackFuncT m_pCallbackFunc;
		ObjectT* m_pListenerObject;
};


/**********************************************************************//**
* The Delegate is the Event-Manager and allows registering/unregistering
* Listeners.
************************************************************************/
template<typename Param1T>
class Delegate1Param
{
	public:
		void RegisterListener( BaseListener1<Param1T>* pListener )
		{
			if( std::find( m_vListeners.begin(), m_vListeners.end(), pListener ) == m_vListeners.end() )
				m_vListeners.push_back( pListener );
		}

		void UnregisterListener( BaseListener1<Param1T>* pListener )
		{
			typename std::vector<BaseListener1<Param1T>*>::iterator it;
			it = std::find( m_vListeners.begin(), m_vListeners.end(), pListener );
			if( it != m_vListeners.end() )
				m_vListeners.erase( it );
		}

		void RaiseEvent( const Param1T& param )
		{
			for( uint32 i = 0; i < m_vListeners.size(); ++i )
			{
				m_vListeners[ i ]->Notify( param );
			}
		}

	private:
		std::vector<BaseListener1<Param1T>*> m_vListeners;
};		

#endif  // INCLUDE_EVENTS_H