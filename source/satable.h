//satable: function about satable

#ifndef SUMMER_SATABLE_H
#define SUMMER_SATABLE_H

#include "../../Qing/qing_common.h"

void compute_sat(const vector<float> &pixels, const int width, const int height, vector<long> &satable);

void compute_mean(const vector<long> &satable, const int width, const int height, const int wsize,
                  vector<float> &meanpixels);

void compute_meanvalue(const vector<long> &satable, const int index, const int cols, const int rows, const int offset,
                       float &meanvalue);


#endif