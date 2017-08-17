#include "satable.h"
//#include "omp.h"

//pixel
void compute_sat(const vector<float> &pixels, const int width, const int height, vector<long> &satable) {
    int cols = width;
    int rows = height;
    int pixelsnum = cols * rows;

    satable.resize(pixelsnum, 0);

    // can not be parallelized
    for (int index = 0; index < pixelsnum; index++) {
        int y = index / cols;
        int x = index % cols;

        long item0, item1, item2, item3;

        item0 = (x - 1 >= 0) ? satable[index - 1] : 0;
        item1 = (y - 1 >= 0) ? satable[index - cols] : 0;
        item2 = (x - 1 >= 0 && y - 1 >= 0) ? satable[index - cols - 1] : 0;
        item3 = pixels[index];
        satable[index] = item0 + item1 + item3 - item2;
    }
}

void compute_mean(const vector<long> &satable, const int width, const int height, const int wsize,
                  vector<float> &meanpixels) {
    int cols = width;  // width
    int rows = height;  // height
    int pixelsnum = cols * rows;

    meanpixels.resize(pixelsnum, 0);

    int offset = wsize / 2;

//	int g_ncore = omp_get_num_procs();
//  double start = omp_get_wtime( );
//#pragma omp parallel for num_threads(g_ncore)
    for (int index = 0; index < pixelsnum; index++) {
        float meanvalue;

        compute_meanvalue(satable, index, cols, rows, offset, meanvalue);
        meanpixels[index] = meanvalue;
    }
}

void
compute_meanvalue(const std::vector<long> &satable, const int index, const int cols, const int rows, const int offset,
                  float &meanvalue) {
    int y = index / cols;
    int x = index % cols;

    int leftx = ((x - offset) >= 0) ? (x - offset) : 0;
    int lefty = ((y - offset) >= 0) ? (y - offset) : 0;
    int rightx = ((x + offset) <= (cols - 1)) ? (x + offset) : (cols - 1);  //width
    int righty = ((y + offset) <= (rows - 1)) ? (y + offset) : (rows - 1);  //height

    int cnt0 = (rightx - leftx + 1) * (y - lefty + 1);  //(rightx, y) <-> (leftx - 1, lefty - 1)
    int cnt1 = (x - leftx + 1) * (righty - lefty + 1);  //(x, righty) <-> (leftx - 1 , lefty - 1)
    int cnt2 = (x - leftx + 1) * (y - lefty + 1);        //(x,y) <-> (leftx  - 1, lefty - 1)
    int cnt3 = (rightx - x) * (righty - y);            //(rightx, righty) <-> (x,y)
    int totalcnt = cnt0 + cnt1 + cnt3 - cnt2;

    long item0, item1, item2, item3;
    long itemx_0, itemx_1, itemx_2, itemx_3;

    {
        //item0: (rightx, y) + (leftx - 1, lefty - 1) - (rightx, lefty - 1) - (leftx - 1, y)
        itemx_0 = satable[y * cols + rightx];
        itemx_1 = (lefty >= 1 && leftx >= 1) ? satable[(lefty - 1) * cols + (leftx - 1)] : 0;
        itemx_2 = (lefty >= 1) ? satable[(lefty - 1) * cols + rightx] : 0;
        itemx_3 = (leftx >= 1) ? satable[y * cols + (leftx - 1)] : 0;

        item0 = itemx_0 + itemx_1 - itemx_2 - itemx_3;
    }

    {
        //item1: (x, righty) + (leftx - 1, lefty - 1) - (leftx - 1, righty) - (x, lefty - 1)
        itemx_0 = satable[righty * cols + x];
        itemx_1 = (lefty >= 1 && leftx >= 1) ? satable[(lefty - 1) * cols + (leftx - 1)] : 0;
        itemx_2 = (leftx >= 1) ? satable[righty * cols + (leftx - 1)] : 0;
        itemx_3 = (lefty >= 1) ? satable[(lefty - 1) * cols + x] : 0;

        item1 = itemx_0 + itemx_1 - itemx_2 - itemx_3;
    }

    {
        //item2: (x, y) + (leftx - 1, lefty - 1) - (x, lefty - 1) - (leftx - 1, y)
        itemx_0 = satable[y * cols + x];
        itemx_1 = (lefty >= 1 && leftx >= 1) ? satable[(lefty - 1) * cols + (leftx - 1)] : 0;
        itemx_2 = (lefty >= 1) ? satable[(lefty - 1) * cols + x] : 0;
        itemx_3 = (leftx >= 1) ? satable[y * cols + (leftx - 1)] : 0;

        item2 = itemx_0 + itemx_1 - itemx_2 - itemx_3;
    }

    {
        //item3:  //(rightx, righty) + (x, y) - (rightx, y) - (x, righty)
        itemx_0 = satable[righty * cols + rightx];
        itemx_1 = satable[y * cols + x];
        itemx_2 = satable[y * cols + rightx];
        itemx_3 = satable[righty * cols + x];

        item3 = itemx_0 + itemx_1 - itemx_2 - itemx_3;
    }

    meanvalue = (float) ((double) (item0 + item1 + item3 - item2) / (double) (totalcnt));

}