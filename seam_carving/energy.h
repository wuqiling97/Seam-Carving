#pragma once

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <fstream>
#include <cassert>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

using std::cout;
using std::endl;
using std::cin;
using std::string;
using namespace cv;

const Mat blurmat = (Mat_<float>(3, 3) <<
	1, 1, 1,
	1, 1, 1,
	1, 1, 1
	) / 9;
const Mat sobel_hor = (Mat_<float>(3, 3) <<
	-1, 0, 1,
	-2, 0, 2,
	-1, 0, 1
	);
const Mat sobel_ver = sobel_hor.t();

Mat e1(const Mat& origin)
{
	Mat channels[3];
	split(origin, channels);
	//assert(channels[0].type() == CV_8U);

	Mat tmp = Mat::zeros(origin.size(), CV_16U);
	for (int i = 0; i < 3; i++) {
		channels[i].convertTo(channels[i], CV_16U);
		tmp += channels[i];
	}
	Mat tmp2 = tmp.clone();
	filter2D(tmp, tmp2, -1, sobel_ver);
	filter2D(tmp, tmp, -1, sobel_hor);
	tmp += tmp2;

	Mat enemat;
	tmp.convertTo(enemat, CV_32S);

	return enemat;
}