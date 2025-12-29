#pragma once

#include "boot/init.h"

void timeEvent();
void displayEvent();
void buttonEvent();
void srEvent();

void recordEvent(void *param);