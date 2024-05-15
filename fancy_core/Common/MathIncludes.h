#pragma once

//Math includes
#define GLM_FORCE_RADIANS 1
#define GLM_FORCE_CTOR_INIT 1  // We want e.g. matrices to be default-constructed to the identity, which we rely on in some places
#define GLM_FORCE_XYZW_ONLY 1
#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/integer.hpp>