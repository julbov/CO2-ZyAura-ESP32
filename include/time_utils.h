#pragma once

#include <Arduino.h>
#include <time.h>

String isoTime(time_t timestamp);
bool timeIsReady();
