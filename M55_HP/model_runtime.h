#pragma once
#include <cstddef>

int model_init();
int model_call(const char *method, float *input, unsigned height, unsigned width,
               unsigned input_channels, float *output, unsigned output_elements);
