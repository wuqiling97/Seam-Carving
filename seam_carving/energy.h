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

Mat e1(Mat origin)
{
	Mat tmp;
	origin.convertTo(tmp, CV_16UC3);
	Mat tmp2 = tmp.clone();
	filter2D(tmp, tmp2, -1, sobel_ver);
	filter2D(tmp, tmp, -1, sobel_hor);
	tmp += tmp2;

	Mat channels[3];
	split(tmp, channels);
	assert(channels[0].type() == CV_16U);
	//for (int i = 0; i < 3; i++) {
	//	cout<<channels[i]<<endl;
	//	imshow("hello", channels[i]);
	//	waitKey();
	//}


	int rows = origin.rows, cols = origin.cols;
	Mat enemat = Mat::zeros(rows, cols, CV_32S);
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			for (int k = 0; k < 3; k++) {
				enemat.at<int>(i, j) += channels[k].at<ushort>(i, j);
			}
		}
	}

	return enemat;
}