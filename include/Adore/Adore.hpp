#pragma once

#include <stdexcept>

#include <export.h>
#include "Logger.hpp"

#define ADORE_LOG(level, msg) Adore::log(Adore::Severity::level, msg);
