#ifndef SUMMER_DEBUG_H
#define SUMMER_DEBUG_H

#include "../../Qing/qing_common.h"

#include "match.h"
#include "mhash.h"
#include "basic.h"

bool readDispList(const string &listfn, const int w, const int h, vector<float> &disp);

Mat showDisp(const vector<float> &disp, const int w, const int h, bool issaved = false);

Mat showMask(const vector<unsigned char> &mask, const int w, const int h, bool issaved = false);

bool readMatches(const string &mfn, vector<CMatch> &matches);

bool saveMatches(const string &mfn, const vector<CMatch> &matches);

bool showMatches(const string &imfn, const vector<unsigned char> &src, const vector<unsigned char> &dst, const int w,
                 const int h, const int ch, const vector<CMatch> &matches);

//c_and: common matches;  a_and: a-common matches;  b_and: b-common matches
void compareMatches(const vector<CMatch> &amatch,
                    const vector<CMatch> &bmatch,
                    vector<CMatch> &c_and,
                    vector<CMatch> &a_and,
                    vector<CMatch> &b_and);


#endif