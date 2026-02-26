#pragma once
#include "Environment.h"
#include "Camera.h"
#include <mutex>

extern std::shared_ptr<Environment> EnvironmentInstance;
extern std::shared_ptr<Camera> CameraInstance;
extern std::mutex EntityMutex;