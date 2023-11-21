#pragma once

#include "location.h"
#include "str.h"
#include "logger.h"

#include <stdio.h>


void point_to_error(Logger* logger, Str source, int index, int size);
