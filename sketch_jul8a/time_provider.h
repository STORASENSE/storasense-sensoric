#pragma once
#include <Arduino.h>

extern unsigned long startupTs;

unsigned long getTimestamp();

void waitForValidTime(unsigned long minEpoch = 1700000000UL,
                      uint16_t maxTries = 40,
                      uint16_t delayMs = 250);