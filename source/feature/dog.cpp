#include "dog.h"

int CDog::notOnEdge(const vector<vector<float> > &dog, int x, int y) {
    const float thresholdEdge = 0.06f;

    const float H00 = dog[y][x - 1] - 2.0f * dog[y][x] + dog[y][x + 1];
    const float H11 = dog[y - 1][x] - 2.0f * dog[y][x] + dog[y + 1][x];
    const float H01 = ((dog[y + 1][x + 1] - dog[y - 1][x + 1]) - (dog[y + 1][x - 1] - dog[y - 1][x - 1])) / 4.0f;
    const float det = H00 * H11 - H01 * H01;
    const float trace = H00 + H11;
    return det > thresholdEdge * trace * trace;
}

float CDog::getResponse(const vector<vector<float> > &pdog,
                        const vector<vector<float> > &cdog,
                        const vector<vector<float> > &ndog,
                        const int x, const int y) {
    return fabs(getResponse(pdog, x, y) + getResponse(cdog, x, y) + getResponse(ndog, x, y) +
                (cdog[y][x] - pdog[y][x]) + (cdog[y][x] - ndog[y][x]));
}

float CDog::getResponse(const vector<vector<float> > &dog,
                        const int x, const int y) {
    const float sum =
            dog[y - 1][x - 1] + dog[y][x - 1] +
            dog[y + 1][x - 1] + dog[y - 1][x] +
            dog[y + 1][x] + dog[y - 1][x + 1] +
            dog[y][x + 1] + dog[y + 1][x + 1];

    return 8 * dog[y][x] - sum;
}

int CDog::isLocalMax(const vector<vector<float> > &dog, const int x, const int y) {
    const float value = dog[y][x];

    if (0.0 < value) {
        if (dog[y - 1][x - 1] < value && dog[y][x - 1] < value &&
            dog[y + 1][x - 1] < value && dog[y - 1][x] < value &&
            dog[y + 1][x] < value && dog[y - 1][x + 1] < value &&
            dog[y][x + 1] < value && dog[y + 1][x + 1] < value)
            return 1;
    } else {
        if (dog[y - 1][x - 1] > value && dog[y][x - 1] > value &&
            dog[y + 1][x - 1] > value && dog[y - 1][x] > value &&
            dog[y + 1][x] > value && dog[y - 1][x + 1] > value &&
            dog[y][x + 1] > value && dog[y + 1][x + 1] > value)
            return -1;
    }
    return 0;
}

int CDog::isLocalMax(const vector<vector<float> > &pdog,
                     const vector<vector<float> > &cdog,
                     const vector<vector<float> > &ndog,
                     const int x, const int y) {

    const int flag = isLocalMax(cdog, x, y);

    if (flag == 1) {
        if (pdog[y][x] < cdog[y][x] && ndog[y][x] < cdog[y][x])
            return 1;
        else
            return 0;
    } else if (flag == -1) {
        if (cdog[y][x] < pdog[y][x] && cdog[y][x] < ndog[y][x])
            return -1;
        else
            return 0;
    }

    return 0;
}

void CDog::setDOG(const vector<vector<float> > &cres,
                  const vector<vector<float> > &nres,
                  vector<vector<float> > &dog) {
    const int height = (int) cres.size();
    const int width = (int) cres[0].size();

    dog = nres;
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            dog[y][x] -= cres[y][x];
}

void CDog::run(const vector<unsigned char> &image, const vector<unsigned char> &mask,
               const vector<unsigned char> &edge, const int width, const int height,
               const int gspeedup,
               const float firstScale,   // 1.4f
               const float lastScale,    // 4.0f
               multiset<CPoint> &result) {
    //std::cerr << "DoG running..." << std::flush;
    m_width = width;
    m_height = height;

    m_firstScale = firstScale;
    m_lastScale = lastScale;

    init(image, mask, edge);

    const int factor = 2;
    const int maxPointsGrid = factor * factor;
    const int gridsize = gspeedup * factor;

    const int w = (m_width + gridsize - 1) / gridsize;
    const int h = (m_height + gridsize - 1) / gridsize;

    int y, x, i;

    /*
    const int gridsize = 50;
    const int w = (int)ceil(m_width / (float)gridsize);
    const int h = (int)ceil(m_height / (float)gridsize);
    const int maxPointsGrid = (int)(m_width * m_height * 0.0025 / (w * h));
    */

    vector<std::vector<std::multiset<CPoint> > > resultgrids;
    resultgrids.resize(h);
    for (y = 0; y < h; ++y)
        resultgrids[y].resize(w);

    const float scalestep = pow(2.0f, 1 / 2.0f);
    //const float scalestep = pow(2.0f, 1.0f);
    const int steps = max(4, (int) ceil(log(m_lastScale / m_firstScale) / log(scalestep)));

    vector<vector<float> > pdog, cdog, ndog, cres, nres;

    setRes(m_firstScale, cres);
    setRes(m_firstScale * scalestep, nres);
    setDOG(cres, nres, cdog);
    cres.swap(nres);
    setRes(m_firstScale * scalestep * scalestep, nres);
    setDOG(cres, nres, ndog);

    vector<vector<unsigned char> > alreadydetected;
    alreadydetected.resize(m_height);
    for (y = 0; y < m_height; ++y) {
        alreadydetected[y].resize(m_width);
        for (x = 0; x < m_width; ++x) {
            alreadydetected[y][x] = (unsigned char) 0;
        }
    }

    for (int i = 2; i <= steps - 1; ++i) {
        const float cscale = m_firstScale * pow(scalestep, i + 1);
        cres.swap(nres);
        setRes(cscale, nres);

        pdog.swap(cdog);
        cdog.swap(ndog);
        setDOG(cres, nres, ndog);

        const int margin = (int) ceil(2 * cscale);
        // now 3 response maps are ready
        for (y = margin; y < m_height - margin; ++y) {
            for (x = margin; x < m_width - margin; ++x) {
                if (alreadydetected[y][x])
                    continue;
                if (cdog[y][x] == 0.0)
                    continue;

                //if (isCloseBoundary(x, y, margin))
                //continue;

                // check local maximum
                if (isLocalMax(pdog, cdog, ndog, x, y) && notOnEdge(cdog, x, y)) {
                    const int x0 = min(x / gridsize, w - 1);
                    const int y0 = min(y / gridsize, h - 1);

                    alreadydetected[y][x] = 1;

                    CPoint p;
                    p.icoord_ = cv::Vec3f(x, y, 1.0f);
                    p.response_ = fabs(cdog[y][x]);
                    p.type_ = 1;

                    resultgrids[y0][x0].insert(p);

                    if (maxPointsGrid < (int) resultgrids[y0][x0].size())
                        resultgrids[y0][x0].erase(resultgrids[y0][x0].begin());
                }
            }
        }
    }

    for (y = 0; y < h; ++y)
        for (x = 0; x < w; ++x) {
            //const float threshold = setThreshold(resultgrids[y][x]);
            multiset<CPoint>::iterator begin = resultgrids[y][x].begin();
            multiset<CPoint>::iterator end = resultgrids[y][x].end();
            while (begin != end) {
                //if (threshold <= begin->m_response)
                result.insert(*begin);
                begin++;
            }
        }

    std::cerr << "Dog features: " << (int) result.size() << " done.     " << std::endl;
}

void CDog::setRes(const float sigma, vector<vector<float> > &res) {
    vector<float> gauss;
    setGaussI(sigma, gauss);

    vector<vector<Vec3f> > vvftmp;
    vvftmp.resize((int) m_image.size());
    for (int y = 0; y < (int) m_image.size(); ++y)
        vvftmp[y].resize((int) m_image[y].size());

    vector<vector<Vec3f> > restmp = m_image;
    convolveX(restmp, m_mask, gauss, vvftmp);
    convolveY(restmp, m_mask, gauss, vvftmp);

    res.resize((int) m_image.size());
    for (int y = 0; y < (int) m_image.size(); ++y) {
        res[y].resize((int) m_image[y].size());
        for (int x = 0; x < (int) m_image[y].size(); ++x)
            res[y][x] = norm(restmp[y][x]);
    }
}

void CDog::init(const vector<unsigned char> &image,
                const vector<unsigned char> &mask,
                const vector<unsigned char> &edge) {
    m_image.clear();
    m_image.resize(m_height);

    int count = 0, y, x;
    for (y = 0; y < m_height; ++y) {
        m_image[y].resize(m_width);
        for (x = 0; x < m_width; ++x) {
            m_image[y][x][0] = ((int) image[count++]) / 255.0f;
            m_image[y][x][1] = ((int) image[count++]) / 255.0f;
            m_image[y][x][2] = ((int) image[count++]) / 255.0f;
        }
    }

    m_mask.clear();
    if (!mask.empty() || !edge.empty()) {
        m_mask.resize(m_height);

        count = 0;
        for (y = 0; y < m_height; ++y) {
            m_mask[y].resize(m_width);
            for (x = 0; x < m_width; ++x) {
                if (mask.empty())
                    m_mask[y][x] = edge[count++];
                else if (edge.empty())
                    m_mask[y][x] = mask[count++];
                else {
                    if (mask[count] && edge[count])
                        m_mask[y][x] = (unsigned char) 255;
                    else
                        m_mask[y][x] = 0;
                    count++;
                }
            }
        }
    }
}