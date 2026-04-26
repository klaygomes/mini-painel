#pragma once

#include "dashboard.h"
#include "draw.h"
#include "layout.h"

void comp_fetch(xf_component_t *comp);
void comp_render(xf_component_t *comp, uint8_t *buf, int w, int h);
