//CDog: Dog feature points

#ifndef SUMMER_DOG_H
#define SUMMER_DOG_H

#include "feature.h"

class CDog : public CFeature {
public:
    CDog(void) {}

    virtual ~CDog() {}

public:
    void run(const vector<unsigned char> &image, const vector<unsigned char> &mask,
             const vector<unsigned char> &edge, const int width, const int height,
             const int gspeedup,
             const float firstscale,   // 1.4f
             const float lastscale,    // 4.0f
             multiset<CPoint> &result);

protected:
    float m_firstScale;
    float m_lastScale;

    void init(const vector<unsigned char> &image,
              const vector<unsigned char> &mask,
              const vector<unsigned char> &edge);

    void setRes(const float sigma, vector<vector<float> > &res);

    static int isLocalMax(const vector<vector<float> > &pdog,
                          const vector<vector<float> > &cdog,
                          const vector<vector<float> > &ndog,
                          const int x, const int y);

    static int isLocalMax(const vector<vector<float> > &dog, const int x, const int y);

    static int notOnEdge(const vector<vector<float> > &dog, int x, int y);

    static float getResponse(const vector<vector<float> > &pdog,
                             const vector<vector<float> > &cdog,
                             const vector<vector<float> > &ndog,
                             const int x, const int y);

    static float getResponse(const vector<vector<float> > &dog, const int x, const int y);

    static void
    setDOG(const vector<vector<float> > &cres, const vector<vector<float> > &nres, vector<vector<float> > &dog);
};

#endif