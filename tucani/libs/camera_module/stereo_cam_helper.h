#ifndef STEREO_CAM_HELPER_H
#define	STEREO_CAM_HELPER_H

using namespace std;

//STRUCTURES



//FUNCTIONS

static void myCvRemap(const cv::Mat originalImg, cv::Mat resultImg,
        cv::InputArray mapx, cv::InputArray mapy) {
    vector<cv::Mat> layers;

    cv::split(originalImg, layers);

    cv::remap(layers[0], layers[0], mapx, mapy, cv::INTER_LINEAR); // Undistort image
    cv::remap(layers[1], layers[1], mapx, mapy, cv::INTER_LINEAR); // Undistort image
    cv::remap(layers[2], layers[2], mapx, mapy, cv::INTER_LINEAR); // Undistort image

    cv::merge(layers, resultImg);
}

static long getTime() {
    struct timeval timeNow, end;

    long seconds, useconds;

    gettimeofday(&timeNow, NULL);

    seconds = timeNow.tv_sec;
    useconds = timeNow.tv_usec;

    return ((seconds) * 1000 + useconds / 1000.0) + 0.5;
}

const int max_hist(int* hist, int hist_size) {
    int max_index = 0;
    int max_val = 0;
    for (int i = 0; i < hist_size; i++) {
        if (hist[i] >= max_val) {
            max_val = hist[i];
            max_index = i;
        }
    }
    return max_index;
}

#endif	/* STEREO_CAM_HELPER_H */

