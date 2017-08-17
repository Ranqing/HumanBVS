#ifndef SUMMER_BASICFUNC_H
#define SUMMER_BASICFUNC_H

#include "../../Qing/qing_common.h"

template<typename PixelType>
void PixelsVector2Mat(const vector<PixelType> &inpixels, int width, int height, int channel, Mat &outmtx) {
    if (channel == 3)
        outmtx.create(height, width, CV_8UC3);
    else if (channel == 1)
        outmtx.create(height, width, CV_8UC1);

    int i, j, index;
    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            index = (j * width + i) * channel;

            if (channel == 3)
                outmtx.at<Vec3b>(j, i) = cv::Vec3i((uchar) (int) (inpixels[index]/* + 0.5*/),
                                                   (uchar) (int) (inpixels[index + 1]/* + 0.5*/),
                                                   (uchar) (int) (inpixels[index + 2] /*+ 0.5*/));
            else if (channel == 1)
                outmtx.at<uchar>(j, i) = (uchar) (int) (inpixels[index]/* + 0.5*/);
        }
    }
}

template<typename PixelType>
void Mat2PixelsVector(const Mat inmtx, vector<PixelType> &outpixels) {
    int width = inmtx.cols;
    int height = inmtx.rows;
    int channel = inmtx.channels();
    int index, j, i;

    outpixels.clear();
    outpixels.resize(width * height * channel);

    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            index = (j * width + i) * channel;
            if (channel == 3) {
                cv::Vec3i tcolor = inmtx.at<cv::Vec3b>(j, i);
                outpixels[index] = tcolor[0];
                outpixels[index + 1] = tcolor[1];
                outpixels[index + 2] = tcolor[2];
            } else if (channel == 1) {
                int tgray = inmtx.at<uchar>(j, i);
                outpixels[index] = tgray;
            }
        }
    }
}

template<typename T>
string type2string(T num) {
    stringstream ss;
    ss << num;
    return ss.str();
}

template<typename T>
T string2type(string str) {
    stringstream stream;

    int num = 0;
    stream << str;
    stream >> num;

    return num;
}

//float / double
template<typename T>
void Gauss1DFilter(int ksize, T sigma, T *&filter1d) {
    assert(ksize % 2);

    filter1d = new T[ksize];

    T sigma2 = sigma * sigma;
    T fenmu = sqrt(2 * QING_PI) * sigma;
    T norm = 0.0;

    int offset = ksize / 2;
    for (int i = -offset; i <= offset; i++) {
        filter1d[i + offset] = exp(-(i * i) / (2 * sigma2));
        filter1d[i + offset] /= fenmu;
        norm += filter1d[i + offset];
    }
    for (int i = 0; i < ksize; i++)
        filter1d[i] /= norm;
}

template<typename T>
void Gauss2DFilter(int ksize, T sigma, T *&filter2d) {
    assert(ksize % 2);

    filter2d = new T[ksize * ksize];

    T *filter1d;
    Gauss1DFilter(ksize, sigma, filter1d);

    T norm = 0.0f;
    int offset = ksize / 2;
    for (int i = -offset; i <= offset; i++) {
        for (int j = -offset; j <= offset; j++) {
            int index = (i + offset) * ksize + (j + offset);
            filter2d[index] = filter1d[i + offset] * filter1d[j + offset];
            norm += filter2d[index];
        }
    }
    for (int i = 0; i < ksize * ksize; i++)
        filter2d[i] /= norm;
}

inline float idot(const Vec3f a, const Vec3f b) {
    float innerp = a.val[0] * b.val[0] + a.val[1] * b.val[1] + a.val[2] * b.val[2];
    return innerp;
}

inline float idot(const Vec4f a, const Vec4f b) {
    float innerp = a.val[0] * b.val[0] + a.val[1] * b.val[1] + a.val[2] * b.val[2] + a.val[3] * b.val[3];
    return innerp;
}

inline void mul(const Mat mtx, const Vec4f a, Vec4f &ret) {
    if (mtx.cols != 4) {
        cerr << "can not multiply the mtx and the vector. mtx rows != 4" << endl;
        exit(1);
    }

    int rows = mtx.rows;
    int cols = mtx.cols;
    int i, j;

    for (i = 0; i < rows; ++i) {
        Vec4f tmpvec;
        for (j = 0; j < cols; ++j)
            tmpvec.val[j] = mtx.at<float>(i, j);

        ret.val[i] = idot(tmpvec, a);
    }
}

#endif

