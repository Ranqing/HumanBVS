//CHarris: harris feature points

#ifndef SUMMER_HARRIS_H
#define SUMMER_HARRIS_H

#include "feature.h"

class CHarris : public CFeature {
public:
    CHarris() {}

    virtual ~CHarris() {}

    void run(const vector<unsigned char> &image,
             const vector<unsigned char> &mask,
             const vector<unsigned char> &edge,
             const int width, const int height,
             const int gspeedup, const float sigma,
             multiset<CPoint> &result);

protected:
    float m_sigmaD;
    float m_sigmaI;

    vector<float> m_gaussD;
    vector<float> m_gaussI;

    vector<vector<Vec3f> > m_dIdx;
    vector<vector<Vec3f> > m_dIdy;

    vector<vector<float> > m_dIdxdIdx;
    vector<vector<float> > m_dIdydIdy;
    vector<vector<float> > m_dIdxdIdy;

    vector<vector<float> > m_response;

    void init(const vector<unsigned char> &image,
              const vector<unsigned char> &mask,
              const vector<unsigned char> &edge);

    void setDerivatives(void);

    void preprocess(void);

    void preprocess2(void);

    void setResponse(void);
};


#endif