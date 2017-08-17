#include "debug.h"

bool readDispList(const string &listfn, const int w, const int h, vector<float> &disp) {
    fstream fin(listfn.c_str(), ios::in);
    if (fin.is_open() != true) {
        printf("failed to open disp file %s..\n", listfn.c_str());
        return false;
    }
    for (int i = 0; i < w * h; ++i) {
        float tmp;
        fin >> tmp;
        disp[i] = tmp;
    }
    cout << "read " << listfn << " done." << endl;
    return true;
}

Mat showDisp(const vector<float> &disp, const int w, const int h, bool issaved/* = false*/) {
    Mat dispmtx = cv::Mat::zeros(h, w, CV_8UC1);
    int maxdisp = 0;
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int index = y * w + x;
            int tmpdisp = disp[index];

            if (maxdisp != 0.0f)
                tmpdisp = tmpdisp * 255 / maxdisp;

            dispmtx.at < unsigned
            char > (y, x) = tmpdisp;
        }
    }

    if (issaved) {
        //how to give a name
    }
    return dispmtx;
}

Mat showMask(const vector<unsigned char> &mask, const int w, const int h, bool issaved /* = false */) {
    Mat mskmtx = cv::Mat::zeros(h, w, CV_8UC1);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int index = y * w + x;
            mskmtx.at < unsigned
            char > (y, x) = mask[index];
        }
    }
    /*cv::imshow("test", mskmtx);
    cv::waitKey(0);
    cv::destroyWindow("test");*/

    if (issaved) {
        //how to give a name
    }

    return mskmtx;
}

bool readMatches(const string &mfn, vector<CMatch> &matches) {
    fstream fin(mfn.c_str(), std::ios::in);
    if (fin.is_open() == false) {
        cerr << "failed to open " << mfn << endl;
        return false;
    }

    matches.clear();

    int matchsize;
    float y, sx, dx;
    float ncc, prior;

    fin >> matchsize;
    for (int i = 0; i < matchsize; ++i) {
        fin >> y >> sx >> dx >> ncc >> prior;

        CMatch tmpmatch = CMatch(cv::Point2f(sx, y), cv::Point2f(dx, y), ncc, prior);
        matches.push_back(tmpmatch);
    }
    fin.close();

    printf("read %s done, %4d matches\n", mfn.c_str(), matchsize);
    return true;
}

bool saveMatches(const string &mfn, const vector<CMatch> &matches) {
    fstream fout(mfn.c_str(), ios::out);
    if (fout.is_open() == false) {
        cerr << "failed to open " << mfn << endl;
        return false;
    }

    fout << matches.size() << endl;
    for (int i = 0; i < matches.size(); ++i) {
        CMatch tmpmatch = matches[i];
        fout << tmpmatch.srcpt.y << ' '
             << tmpmatch.srcpt.x << ' '
             << tmpmatch.dstpt.x << ' '
             << tmpmatch.ncc << ' '
             << tmpmatch.prior << endl;
    }
    fout << -1 << endl;
    printf("save matches in file %s\n", mfn.c_str());
    return true;
}

bool showMatches(const string &imfn, const vector<unsigned char> &src, const vector<unsigned char> &dst, const int w,
                 const int h, const int ch, const vector<CMatch> &matches) {
    Mat srcmtx, dstmtx, pairmtx, partmtx;
    PixelsVector2Mat<unsigned char>(src, w, h, ch, srcmtx);
    PixelsVector2Mat<unsigned char>(dst, w, h, ch, dstmtx);

    if (ch == 1)
        pairmtx.create(h, w * 2, CV_8UC1);
    else if (ch == 3)
        pairmtx.create(h, w * 2, CV_8UC3);

    partmtx = pairmtx(cv::Rect(0, 0, w, h));
    srcmtx.copyTo(partmtx);
    partmtx = pairmtx(cv::Rect(w, 0, w, h));
    dstmtx.copyTo(partmtx);

    for (int i = 0; i < matches.size(); ++i) {
        CMatch tmpmatch = matches[i];
        circle(pairmtx, tmpmatch.srcpt, 1, cv::Scalar(255, 0, 0));
        circle(pairmtx, Point(tmpmatch.dstpt.x + w, tmpmatch.dstpt.y), 1, Scalar(255, 0, 0));
    }

    /*cv::imshow("show matches", pairmtx);
    cv::waitKey(0);
    cv::destroyWindow("show matches");*/
    imwrite(imfn, pairmtx);
    return true;
}

//and: common matches;  a_and: a-common matches;  b_and: b-common matches
void compareMatches(const vector<CMatch> &amatch,
                    const vector<CMatch> &bmatch,
                    vector<CMatch> &c_and,
                    vector<CMatch> &a_and,
                    vector<CMatch> &b_and) {
    printf("compare matches between amatch and bmatch\n");

    CMhash intermediate;
    int nh = bmatch.size();
    int nm = nh;
    intermediate.init(nh, nm);
    intermediate.copyfrom(0, bmatch);

    int asize = amatch.size();
    vector<bool> isand(asize, false);
    c_and.clear();
    a_and.clear();
    b_and.clear();

    printf("before look up %d keys, %d matches\n", intermediate.numofkeys_, intermediate.numofvalues_);
    for (int i = 0; i < asize; ++i) {
        Point2i tmpakey = amatch[i].srcpt;
        Point2i tmpampt = amatch[i].dstpt;   //amatch point

        bool isfind = false;
        if (intermediate.getinit(tmpakey)) {
            CMatchValue tmpbvalue;
            Point2i tmpbmpt;
            while (intermediate.getnext(tmpbvalue)) {
                tmpbmpt = tmpbvalue.dstpt;
                if (tmpbmpt == tmpampt) {
                    isfind = true;
                    intermediate.erase(tmpakey, tmpbvalue);
                    break;
                }
            }
        }


        if (isfind == true)
            isand[i] = true;
    }
    printf("after look up %d keys, %d matches\n", intermediate.numofkeys_, intermediate.numofvalues_);

    for (int i = 0; i < asize; ++i)
        if (isand[i] == true)
            c_and.push_back(amatch[i]);
        else
            a_and.push_back(amatch[i]);
    intermediate.copyto(0, b_and);
}