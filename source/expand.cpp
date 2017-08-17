#include "expand.h"
#include "findmatch.h"

#define  DIRECT_UPSAMPLING

const int CExpand::st_neighbor_size = 3;

CExpand::CExpand(CFindMatch &findmatch) : fm_(findmatch) {
}

void CExpand::init() {
    printf("\nCExpand initialization\n");
}

void CExpand::run() {
    int maxlevels = fm_.max_level_, level;
    int nh, nm, maxcount, i;
    string prefix;

    for (level = maxlevels - 1; level >= 0; --level) {
        printf("\nlevel %d expand matches start. nccthreshold = %.2f, priorthreshold = %.2f\n", level,
               fm_.ncc_threshold_, fm_.prior_threshold_);

        maxcount = fm_.st_ncandidates_;
        nh = max( fm_.photopair_->photos_[0].get_validnum(level), fm_.photopair_->photos_[1].get_validnum(level)) * maxcount;
        nm = nh;

        CMhash candidates[2];
        candidates[0].init(nh, nm);
        candidates[1].init(nh, nm);

        // only integer pixel
        fusion(level, 0, fm_.matchset_->matches_, candidates[0]);  //matches fusion
        fusion(level, 1, fm_.matchset_->matches_, candidates[1]);
        fm_.cross_check(candidates[0], candidates[1], fm_.matchset_->matches_[level]);
        fm_.matchset_->saveMatches("seed_fused_", level);

        nh = nh + nm;
        nm = nh;

        candidates[0].init(nh, nm);    // clear former matches candidates
        expansion(level, 0, 1, candidates[0]);
        candidates[1].init(nh, nm);    // clear former matches candidates
        expansion(level, 1, 0, candidates[1]);
        fm_.cross_check(candidates[0], candidates[1], fm_.matchset_->matches_[level]);
        fm_.matchset_->saveMatches("expansion_", level);

        //fm_.matchset_.optimize(level);
        //return ;
    }
}

void CExpand::fusion(const int level, const int src, vector<vector<CMatch>> &allmatches, CMhash &candidate) {
    candidate.copyfrom(src, allmatches[level]);

    printf("level %d seeds fusion. %d <- %d\n", level, src, 1 - src);

    for (int tmplevel = 0; tmplevel < level; ++tmplevel)
        down_sampling(tmplevel, level, src, allmatches[tmplevel], candidate);   //tmplevel down sampling to level

    if (level < fm_.max_level_ - 1)
        up_sampling(level + 1, level, src, allmatches[level + 1], candidate);  // level + 1 up sampling to level
}

void
CExpand::down_sampling(const int uplevel, const int level, const int src, vector<CMatch> &matches, CMhash &candidate) {
    printf("level %d downsampling. %4d matches + %4d matches -> ", level, candidate.numofvalues_, matches.size());

    int scale = 1 << (level - uplevel);
    int width = fm_.photopair_->photos_[src].get_width(level);
    int height = fm_.photopair_->photos_[src].get_height(level);
    int maxdisp = width * 0.75;
    int mindisp = 0;
    int nccsize = fm_.get_nccwsize_(level);
    int maxcount = fm_.st_ncandidates_;
    int i, tmpdisp;

    CMatch tmpmatch;
    Point2i tmpkey;
    CMatchValue tmpvalue;

    for (i = 0; i < matches.size(); i++) {
        tmpmatch = matches[i];
        tmpmatch.srcpt = Point2i(tmpmatch.srcpt.x / scale, tmpmatch.srcpt.y / scale);
        tmpmatch.dstpt = Point2i(tmpmatch.dstpt.x / scale, tmpmatch.dstpt.y / scale);

        if (src == 1) {
            Point2i mutpt = tmpmatch.srcpt;
            tmpmatch.srcpt = tmpmatch.dstpt;
            tmpmatch.dstpt = mutpt;
        }

        tmpdisp = abs(tmpmatch.srcpt.x - tmpmatch.dstpt.x);
        if (tmpdisp <= mindisp || tmpdisp > maxdisp)
            continue;

        if (fm_.photopair_->photos_[src].get_mask(tmpmatch.srcpt.x, tmpmatch.srcpt.y, level) != 255 ||
            fm_.photopair_->photos_[1 - src].get_mask(tmpmatch.dstpt.x, tmpmatch.dstpt.y, level) != 255)
            continue;

        update_match(level, src, nccsize, tmpmatch);

        tmpkey = tmpmatch.srcpt;
        tmpvalue = CMatchValue(tmpmatch.dstpt, tmpmatch.ncc, -1.0f);

        if (candidate.isexist(tmpkey, tmpvalue) == true)
            continue;

        candidate.maxstore(tmpkey, tmpvalue, maxcount);
    }
    printf(" %d matches\n", candidate.numofvalues_);
}

void
CExpand::up_sampling(const int downlevel, const int level, const int src, vector<CMatch> &matches, CMhash &candidate) {
    printf("level %d up-sampling. %4d matches + %4d matches -> ", level, candidate.numofvalues_, matches.size());

    int scale = 2;
    int width = fm_.photopair_->photos_[src].get_width(level);
    int height = fm_.photopair_->photos_[src].get_height(level);
    int maxdisp = width * 0.75;
    int mindisp = 0;
    int nccsize = fm_.get_nccwsize_(level);
    int maxcount = fm_.st_ncandidates_;
    int i, j, deltax, deltay, tmpdisp;
    Point2i tmpkey;
    CMatchValue tmpvalue;

#ifdef DIRECT_UPSAMPLING
    for (i = 0; i < matches.size(); ++i) {
        CMatch tmpmatch = matches[i];
        CMatch upmatches[4];

        if (src == 1) {
            Point2i mutpt = tmpmatch.srcpt;
            tmpmatch.srcpt = tmpmatch.dstpt;
            tmpmatch.dstpt = mutpt;
        }

        for (j = 0; j < 4; j++) {
            deltax = j % 2;
            deltay = j / 2;

            //initial: d, d-1, d+1
            upmatches[j].srcpt = Point2i(tmpmatch.srcpt.x * scale + deltax, tmpmatch.srcpt.y * scale + deltay);
            upmatches[j].dstpt = Point2i(tmpmatch.dstpt.x * scale + deltax, tmpmatch.dstpt.y * scale + deltay);

            tmpdisp = abs(upmatches[j].srcpt.x - upmatches[j].dstpt.x);
            if (tmpdisp <= mindisp || tmpdisp > maxdisp)
                continue;

            if (fm_.photopair_->photos_[src].get_mask(upmatches[j].srcpt.x, upmatches[j].srcpt.y, level) != 255 ||
                fm_.photopair_->photos_[1 - src].get_mask(upmatches[j].dstpt.x, upmatches[j].dstpt.y, level) != 255)
                continue;

            update_match(level, src, nccsize, upmatches[j]);
            get_optimal_match(level, src, nccsize, upmatches[j]);

            tmpkey = upmatches[j].srcpt;
            tmpvalue = CMatchValue(upmatches[j].dstpt, upmatches[j].ncc, upmatches[j].prior);

            if (candidate.isexist(tmpkey, tmpvalue))
                continue;

            candidate.maxstore(tmpkey, tmpvalue, maxcount);
        }
    }
#else
    for (i = 0; i < matches.size(); ++ i)
    {
        CMatch tmpmatch = matches[i];
        tmpmatch.srcpt = Point2i( tmpmatch.srcpt.x * scale, tmpmatch.srcpt.y * scale );
        tmpmatch.dstpt = Point2i( tmpmatch.dstpt.x * scale, tmpmatch.dstpt.y * scale );

        if (src == 1)
        {
            Point2i mutpt = tmpmatch.srcpt;
            tmpmatch.srcpt = tmpmatch.dstpt;
            tmpmatch.dstpt = mutpt;
        }

        tmpdisp = abs( tmpmatch.srcpt.x - tmpmatch.dstpt.x );
        if (tmpdisp <= mindisp || tmpdisp > maxdisp )
            continue;

        if (fm_.photopair_.photos_[src].getMask(tmpmatch.srcpt.x, tmpmatch.srcpt.y, level) != 255 ||
            fm_.photopair_.photos_[1-src].getMask(tmpmatch.dstpt.x, tmpmatch.dstpt.y, level) != 255 )
            continue;

        update_match(level, src, nccsize, tmpmatch);
        up_get_optimal_match(level, src, nccsize, tmpmatch);

        tmpkey = tmpmatch.srcpt;
        tmpvalue = CMatchValue(tmpmatch.dstpt, tmpmatch.ncc, -1.0f);

        if ( candidate.isexist(tmpkey, tmpvalue) == true )
            continue;

        candidate.maxstore(tmpkey, tmpvalue, maxcount);
    }
#endif

    printf("%4d matches\n", candidate.numofvalues_);
}

void
CExpand::update_match(const int level, const int src, const int nccsize, CMatch &newmatch, bool isprior /* = 0 */) {
    if (isprior == true)
        fm_.photopair_->compute_match(level, src, 1 - src, nccsize, newmatch);
    else {
        fm_.photopair_->compute_match_ncc(level, src, 1 - src, nccsize, newmatch.srcpt, newmatch.dstpt, newmatch.ncc);
        newmatch.prior = -1.0f;
    }
}

//special for upsampling 
void CExpand::up_get_optimal_match(const int level, const int src, const int nccsize, CMatch &inmatch) {
    CMatch tmpmatch = inmatch;
    CMatch optimalmatch = inmatch;

    int width = fm_.photopair_->photos_[src].get_width(level);
    int height = fm_.photopair_->photos_[src].get_height(level);
    int iy, ix, id;

    for (iy = -1; iy <= 1; ++iy) {
        for (ix = -1; ix <= 1; ++ix) {
            for (id = -1; id <= 1; ++id) {
                if (ix == 0 && iy == 0 && id == 0) continue;

                tmpmatch.srcpt = Point2i(inmatch.srcpt.x + ix + id, inmatch.srcpt.y + iy);
                tmpmatch.dstpt = Point2i(inmatch.dstpt.x + ix, inmatch.dstpt.y + iy);

                if (tmpmatch.srcpt.x < 0 || tmpmatch.srcpt.x >= width || tmpmatch.dstpt.x < 0 ||
                    tmpmatch.dstpt.x >= width ||
                    tmpmatch.srcpt.y < 0 || tmpmatch.srcpt.y >= height || tmpmatch.dstpt.y < 0 ||
                    tmpmatch.dstpt.y >= height)
                    continue;

                if (fm_.photopair_->photos_[src].get_mask(tmpmatch.srcpt.x, tmpmatch.srcpt.y, level) != 255 ||
                    fm_.photopair_->photos_[1 - src].get_mask(tmpmatch.dstpt.x, tmpmatch.dstpt.y, level) != 255)
                    continue;

                update_match(level, src, nccsize, tmpmatch);

                if (optimalmatch.ncc <
                    tmpmatch.ncc /*|| ( optimalmatch.ncc - tmpmatch.ncc < 0.01 && optimalmatch.prior < tmpmatch.prior )*/ )
                    optimalmatch = tmpmatch;
            }
        }
    }

    inmatch = optimalmatch;
}

//d, d-1, d+1
void CExpand::get_optimal_match(const int level, const int src, const int nccsize, CMatch &inmatch) {
    CMatch tmpmatch;
    CMatch optimalmatch = inmatch;

    for (int id = -1; id <= 1; ++id) {
        if (id == 0) continue;

        tmpmatch.srcpt = inmatch.srcpt;
        tmpmatch.dstpt = Point2i(inmatch.dstpt.x + id, inmatch.dstpt.y);

        if (fm_.photopair_->photos_[src].get_mask(tmpmatch.srcpt.x, tmpmatch.srcpt.y, level) != 255 ||
            fm_.photopair_->photos_[1 - src].get_mask(tmpmatch.dstpt.x, tmpmatch.dstpt.y, level) != 255)
            continue;

        update_match(level, src, nccsize, tmpmatch);

        if (optimalmatch.ncc <
            tmpmatch.ncc) // || ( optimalmatch.ncc - tmpmatch.ncc < 0.01 && optimalmatch.prior < tmpmatch.prior ))
            optimalmatch = tmpmatch;
    }

    inmatch = optimalmatch;
}

void CExpand::expansion(const int level, const int src, const int dst, CMhash &candidate) {
    printf("level %d seeds expansion. %d <- %d\n", level, src, dst);

    int width = fm_.photopair_->photos_[src].get_width(level);
    int height = fm_.photopair_->photos_[src].get_height(level);
    int maxdisp = width * 0.75;
    int mindisp = 0;
    int maxcount = fm_.st_ncandidates_;
    float nccthreshold = fm_.ncc_threshold_;
    float priorthreshold = fm_.prior_threshold_;

    //build a priority queue
    priority_queue<CMatch> matchqueue;
    vector<CMatch> &localmatches = fm_.matchset_->matches_[level];

    int msize = localmatches.size();
    int i, issaved, curdisp, newdisp;
    int nnum = st_neighbor_size * st_neighbor_size;   //total neighbor number
    int valid;

    CMatch curmatch, newmatch;
    Point2i curkey, newkey;
    CMatchValue curvalue, newvalue;

    for (i = 0; i < msize; i++) {
        curmatch = localmatches[i];
        if (src == 1) {
            Point2i tmppt = curmatch.srcpt;
            curmatch.srcpt = curmatch.dstpt;
            curmatch.dstpt = tmppt;
        }
        matchqueue.push(curmatch);
    }
    printf("priority queue done. before expansion , %5d matches. ", matchqueue.size());

    while (!matchqueue.empty()) {
        curmatch = matchqueue.top();
        matchqueue.pop();

        curkey = curmatch.srcpt;
        curvalue = CMatchValue(curmatch.dstpt, curmatch.ncc, curmatch.prior);

        issaved = candidate.maxstore(curkey, curvalue, maxcount);
        if (issaved == false)
            continue;

        curdisp = abs(curmatch.srcpt.x - curmatch.dstpt.x);

        vector<CMatch> neimatchs(0);
        expand_in_neighbor(level, src, curmatch, st_neighbor_size, neimatchs);
        if (neimatchs.size() * 2 < nnum)
            continue;

        valid = 0;
        for (i = 0; i < neimatchs.size(); ++i) {
            newdisp = abs(neimatchs[i].srcpt.x - neimatchs[i].dstpt.x);
            if (abs(newdisp - curdisp) <= 1)
                ++valid;
        }
        if (valid * 2 < neimatchs.size())
            continue;

        for (i = 0; i < neimatchs.size(); ++i) {
            newmatch = neimatchs[i];
            newkey = newmatch.srcpt;
            newvalue = CMatchValue(newmatch.dstpt, newmatch.ncc, -1.0f);

            if (candidate.isexist(newkey, newvalue))
                continue;
            else
                matchqueue.push(newmatch);
        }
    }

    printf("after expansion, %6d matches\n", candidate.numofvalues_);
}


void CExpand::expand_in_neighbor(const int level, const int src, const CMatch rawmatch, const int nsize,
                                 vector<CMatch> &newmatches) {
    int nccsize = fm_.get_nccwsize_(level);
    int offset = nsize / 2;
    int iy, ix;
    CMatch tmpmatch;

    for (iy = -offset; iy <= offset; ++iy) {
        for (ix = -offset; ix <= offset; ++ix) {
            if (iy == 0 && ix == 0) continue;

            tmpmatch.srcpt = cv::Point2i(rawmatch.srcpt.x + ix, rawmatch.srcpt.y + iy);
            tmpmatch.dstpt = cv::Point2i(rawmatch.dstpt.x + ix, rawmatch.dstpt.y + iy);

            if (!fm_.photopair_->photos_[src].is_safe(tmpmatch.srcpt, level) ||
                !fm_.photopair_->photos_[1 - src].is_safe(tmpmatch.dstpt, level))
                continue;

            if (fm_.photopair_->photos_[src].get_mask(tmpmatch.srcpt.x, tmpmatch.srcpt.y, level) != 255 ||
                fm_.photopair_->photos_[1 - src].get_mask(tmpmatch.dstpt.x, tmpmatch.dstpt.y, level) != 255)
                continue;

            //only compute ncc?
            update_match(level, src, nccsize, tmpmatch);
            get_optimal_match(level, src, nccsize, tmpmatch);

            if (tmpmatch.ncc > fm_.ncc_threshold_ || fabs(tmpmatch.ncc - fm_.ncc_threshold_) <= 0.01)
                newmatches.push_back(tmpmatch);
        }
    }
}




