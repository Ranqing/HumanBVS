#ifndef SUMMER_OPTIMIZER_H
#define SUMMER_OPTIMIZER_H

#include "linklist.h"

class CFindMatch;

class COptimizer {
public:
    COptimizer(CFindMatch &findmatch);

    ~COptimizer();

    void init();

    void run();

    void interpolate(const int src, const int level);

    void eliminate(const int src, const int level);

    void findNoMatchedPixels(const vector<float> &rawdisp,
                             const vector<unsigned char> &rawmask,
                             const int width,
                             const int height,
                             const int maxnum,
                             int &totalnum,
                             vector<CLinklist> &regions);

    void showNoMatchedPixels(const int src, const int level, const vector<float> &rawdisp, vector<CLinklist> &regions);

    void findMatchedPixels(const vector<float> &rawdisp,
                           const vector<unsigned char> &rawmask,
                           const int width,
                           const int height,
                           const int maxnum,
                           int &totalnum, vector<CLinklist> &regions);

    void erodeRawDisp(vector<float> &rawdisp,
                      vector<unsigned char> &rawmask,
                      const int ksize, const int w, const int h);


    void debug();

    void fill(const int level, vector<float> &disp);

    void smooth(const int level, vector<float> &disp);

protected:
    CFindMatch &fm_;
};

#endif