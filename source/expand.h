//CExpand:: class for matches expansion
#ifndef SUMMER_EXPAND_H
#define SUMMER_EXPAND_H

#include "match.h"
#include "mhash.h"

class CFindMatch;

class CExpand {
public:
    CExpand(CFindMatch &findmatch);

    ~CExpand() {};

    void init(void);

    void run();

    void fusion(const int level, const int src, vector<vector<CMatch>> &allmatches, CMhash &candidate);

    void expansion(const int level, const int src, const int dst, CMhash &candidate);

    void down_sampling(const int uplevel, const int level, const int src, vector<CMatch> &matches, CMhash &candidate);

    void up_sampling(const int downlevel, const int level, const int src, vector<CMatch> &matches, CMhash &candidate);

    //re compute the match's ncc / prior in level
    void update_match(const int level, const int src, const int nccsize, CMatch &newmatch, bool isprior = 0);

    void up_get_optimal_match(const int level, const int src, const int nccsize, CMatch &inmatch);

    //choose best match in d-1,d,d+1
    void get_optimal_match(const int level, const int src, const int nccsize, CMatch &inmatch);

    void expand_in_neighbor(const int level, const int src, const CMatch rawmatch, const int nsize,
                            vector<CMatch> &newmatches);

    //run in blocks
    //void blockrun();

    static const int st_neighbor_size;

protected:
    CFindMatch &fm_;
};

#endif



