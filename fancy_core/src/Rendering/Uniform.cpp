#include "Uniform.h"
#include "UniformUtil.h"
#include "UniformRegistry.h"

template<typename ValueT>
Uniform<ValueT>::Uniform( const Uniform<ValueT>* other )
{
	m_clValue = other->m_clValue;
	m_iglHandle = other->m_iglHandle;
	m_szName = other->m_szName;
	m_pObserver = other->m_pObserver;
}





