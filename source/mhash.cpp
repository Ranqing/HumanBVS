#include "mhash.h"

CMhash::CMhash(int nh /* = 0 */, int nm /* = 0 */) : pkeys_(NULL), pmhash_(NULL), numofkeys_(0), numofvalues_(0) {
    if (nh && nm) {
        pkeys_ = new Point2i[nm];
        pmhash_ = new MhashMap(nh, nm);

        if (pkeys_ == NULL || pmhash_ == NULL) {
            cerr << "failed to construct a multi-hash map" << endl;
            exit(1);
        }
    }
}

CMhash::~CMhash() {
    if (pmhash_ != NULL) {
        delete pmhash_;
        pmhash_ = NULL;
    }

    if (pkeys_ != NULL) {
        delete[] pkeys_;
        pkeys_ = NULL;
    }
}

void CMhash::init(int nh, int nm) {
    if (pmhash_ != NULL) {
        delete pmhash_;
        pmhash_ = NULL;
    }
    if (pkeys_ != NULL) {
        delete[] pkeys_;
        pkeys_ = NULL;
    }

    pmhash_ = new MhashMap(nh, nm);
    pkeys_ = new Point2i[nm];

    if (pmhash_ == NULL || pkeys_ == NULL) {
        cerr << "failed to initialize a CMHashMap.." << endl;
        exit(1);
    }

    numofkeys_ = 0;
    numofvalues_ = 0;
    capacity_ = nh;
}

bool CMhash::isexist(const Point2i tkey, const CMatchValue tvalue) {
    if (this->getinit(tkey)) {
        CMatchValue tmpvalue;
        while (this->getnext(tmpvalue)) {
            /*if (tmpvalue == tvalue) return true;*/
            if (tmpvalue.dstpt == tvalue.dstpt)
                return true;
        }
    }
    return false;
}

//return true: 1.(tkey, tvalue) is not exist
//             2.count(tkey) < maxcount
//             3.count(tkey) == maxcount but minvalue < tvalue  
bool CMhash::maxstore(const Point2i tkey, const CMatchValue tvalue, const int maxcount) {
    int count = this->count(tkey);
    if (count < maxcount) {
        store(tkey, tvalue);
        return true;
    }

    CMatchValue minvalue = get_minvalue(tkey);
    if (minvalue < tvalue) {
        erase(tkey, minvalue);
        store(tkey, tvalue);
        return true;
    }

    return false;
}

cv::Point2i CMhash::get_akey(int i) {
    if (i >= numofkeys_) {
        cerr << "out of number. " << endl;
        return cv::Point2i(-1, -1);
    }
    return pkeys_[i];
}

CMatchValue CMhash::get_minvalue(const Point2i tkey) {
    CMatchValue tvalue, minvalue;
    if (this->getinit(tkey)) {
        this->getnext(minvalue);
        while (this->getnext(tvalue)) {
            if (tvalue < minvalue)
                minvalue = tvalue;
        }
    } else {
        cout << "CMHashMap::getminvalue() : no key." << endl;
        //minvalue = CMatchNode(-1, 0.0, 0.0);
    }

    return minvalue;
}

int CMhash::store(const Point2i &tkey, const CMatchValue &tvalue) {
    if (numofkeys_ == capacity_ || numofvalues_ == capacity_) {
        cerr << "CMhash::store() failed. full of capcity. capacity = " << capacity_ << endl;
        exit(1);
    }

    int count = pmhash_->count(tkey);
    if (count == 0) {
        pkeys_[numofkeys_++] = tkey;
    }
    numofvalues_++;

    return pmhash_->store(tkey, tvalue);
}

int CMhash::erase(const Point2i &tkey, const CMatchValue &tvalue) {
    int count = pmhash_->count(tkey), i;
    if (count == 0) {
        cerr << "no key-value pair in multi-hash map" << endl;
        return -1;
    }

    if (count == 1) {
        for (i = 0; i < numofkeys_; i++) {
            if (pkeys_[i] == tkey) {
                pkeys_[i] = pkeys_[numofkeys_ - 1];
                pkeys_[numofkeys_ - 1] = pkeys_[numofkeys_];
                numofkeys_--;
                break;
            }
        }
    }
    numofvalues_--;

    return pmhash_->erase(tkey, tvalue);
}

int CMhash::count(const Point2i &tkey) {
    return pmhash_->count(tkey);
}

//return the initial value's index of "tkey", otherwise 0
int CMhash::getinit(const Point2i &tkey) {
    return pmhash_->getinit(tkey);
}

int CMhash::getnext(CMatchValue &value) {
    return pmhash_->getnext(value);
}

void CMhash::copyfrom(const int src, const vector<CMatch> &matches) {
    if (pmhash_ == NULL) {
        cerr << "init first." << endl;
        exit(1);
    }

    int msize = matches.size();
    for (int i = 0; i < msize; ++i) {
        Point2i tmpsrcpt = matches[i].srcpt;
        Point2i tmpdstpt = matches[i].dstpt;
        float tmpncc = matches[i].ncc;
        float tmpprior = matches[i].prior;

        if (src == 0) {
            CMatchValue tmpvalue = CMatchValue(tmpdstpt, tmpncc, tmpprior);
            store(tmpsrcpt, tmpvalue);
        } else {
            CMatchValue tmpvalue = CMatchValue(tmpsrcpt, tmpncc, tmpprior);
            store(tmpdstpt, tmpvalue);
        }
    }

    printf("CMhash::copy from a matches vector: numofmatches = %d. mhash : numofkeys = %d, numofvalues = %d\n",
           matches.size(), numofkeys_, numofvalues_);
}

void CMhash::copyto(const int src, vector<CMatch> &matches) {
    matches.clear();
    matches.resize(0);

    int msize = numofkeys_;
    for (int i = 0; i < msize; ++i) {
        Point2i tmpsrcpt = pkeys_[i];
        CMatchValue tmpvalue;

        if (getinit(tmpsrcpt)) {
            while (getnext(tmpvalue)) {
                Point2i tmpdstpt = tmpvalue.dstpt;
                float tmpncc = tmpvalue.ncc;
                float tmpprior = tmpvalue.prior;

                if (src == 0) {
                    CMatch tmpmatch = CMatch(tmpsrcpt, tmpdstpt, tmpncc, tmpprior);
                    matches.push_back(tmpmatch);
                } else {
                    CMatch tmpmatch = CMatch(tmpdstpt, tmpsrcpt, tmpncc, tmpprior);
                    matches.push_back(tmpmatch);
                }

            }
        }
    }
    printf("CMhash::copy to a matches vector: mhash : numofkeys = %d, numofvalues = %d\n..  numofmatches = %d, ",
           numofkeys_, numofvalues_, matches.size());
}





