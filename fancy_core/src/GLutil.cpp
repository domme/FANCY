#include "GLutil.h"
//-----------------------------------------------------------------------//
GLutil::GlEnumFunc GLutil::m_glEnableDisableFunc[] = {
  &glEnable,
  &glDisable
};
//-----------------------------------------------------------------------//
GLutil::GLEnumIndexFunc GLutil::m_glEnableDisableIndexedFunc[] = {
  &glEnablei,
  &glDisablei
};