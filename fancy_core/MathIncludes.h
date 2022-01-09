#pragma once

//Math includes
#define GLM_FORCE_RADIANS 1
#define GLM_FORCE_CTOR_INIT 1  // We want e.g. matrices to be default-constructed to the identity, which we rely on in some places
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>