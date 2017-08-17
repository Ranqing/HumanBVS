#ifndef SUMMER_FEATURE_H
#define SUMMER_FEATURE_H

#include "point.h"

class CFeature {
public:
    CFeature(void);
    virtual ~CFeature(void);

    static void setGaussD(const float sigmaD, vector<float> &gaussD);
    static void setGaussI(const float sigmaI, vector<float> &gaussI);

protected:
    static float setThreshold(multiset<CPoint> &grid);   //set threshold
    int isCloseBoundary(const int x, const int y, const int margin) const;

    int m_width;
    int m_height;
    vector<vector<Vec3f> > m_image;
    vector<vector<uchar> > m_mask;

public:
    template<class T>
    void convolveX(vector<vector<T> > &image, const vector<vector<uchar> > &mask, const vector<float> &filter,
                   vector<vector<T> > &buffer) {
        const int width = image[0].size();
        const int height = image.size();
        const int margin = ((int) filter.size()) / 2;

        int x, y, j, xtmp, ytmp;
        for (y = 0; y < height; ++y) {
            for (x = 0; x < width; ++x) {
                buffer[y][x] = buffer[y][x] * 0.0;

                if (!mask.empty() && mask[y][x] == 0)
                    continue;

                for (j = 0; j < (int) filter.size(); ++j) {
                    xtmp = min(max(0, (x + j - margin)), width - 1);
                    ytmp = y;

                    if (!mask.empty() && mask[ytmp][xtmp] == 0)
                        continue;

                    buffer[y][x] += image[ytmp][xtmp] * filter[j];
                }
            }
        }
        buffer.swap(image);
    }


    template<class T>
    void convolveY(vector<vector<T> > &image, const vector<vector<unsigned char> > &mask, const vector<float> &filter,
                   vector<vector<T> > &buffer) {
        const int width = image[0].size();
        const int height = image.size();
        const int margin = ((int) filter.size()) / 2;

        int y, x, j, xtmp, ytmp;

        for (y = 0; y < height; ++y) {
            for (x = 0; x < width; ++x) {
                buffer[y][x] = buffer[y][x] * 0.0;

                if (!mask.empty() && mask[y][x] == 0)
                    continue;

                for (j = 0; j < (int) filter.size(); ++j) {
                    xtmp = x;
                    ytmp = min(height - 1, max(0, (y + j - margin)));

                    if (!mask.empty() && mask[ytmp][xtmp] == 0)
                        continue;

                    buffer[y][x] += image[ytmp][xtmp] * filter[j];
                }
            }
        }

        buffer.swap(image);
    }

    //no mask!
    template<class T>
    void convolveX(vector<vector<T> > &image, const vector<float> &filter, vector<vector<T> > &buffer) {
        const int width = image[0].size();
        const int height = image.size();
        const int margin = ((int) filter.size()) / 2;

        int y, x, j, xtmp, ytmp;
        for (y = 0; y < height; ++y) {
            for (x = 0; x < width; ++x) {
                buffer[y][x] *= 0.0;

                for (j = 0; j < (int) filter.size(); ++j) {
                    xtmp = x + j - margin;
                    ytmp = y;
                    if (xtmp < 0 || width <= xtmp)
                        continue;

                    buffer[y][x] += filter[j] * image[ytmp][xtmp];
                }
            }
        }

        buffer.swap(image);
    }


    template<class T>
    void convolveY(vector<vector<T> > &image, const vector<float> &filter, vector<vector<T> > &buffer) {
        const int width = image[0].size();
        const int height = image.size();
        const int margin = ((int) filter.size()) / 2;

        int y, x, j, xtmp, ytmp;

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                buffer[y][x] *= 0.0;

                for (int j = 0; j < (int) filter.size(); ++j) {
                    xtmp = x;
                    ytmp = y + j - margin;
                    if (ytmp < 0 || height <= ytmp)
                        continue;

                    buffer[y][x] += filter[j] * image[ytmp][xtmp];
                }
            }
        }
        buffer.swap(image);
    }
};

#endif
