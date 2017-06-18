#pragma once

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <fstream>
#include <cassert>
#include <cmath>

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

double RGBlength(Vec3b v)
{
	int sum = 0;
	for (int i = 0; i < 3; i++) {
		sum += v[i] * v[i];
	}
	return sqrt(sum);
}

double RGBdistance(Vec3b a, Vec3b b)
{
	Vec3b d = a - b;
	return RGBlength(d);
}


Mat add_channels(const Mat& input)
{
	std::vector<Mat> channels(input.channels());
	split(input, channels);
	//assert(channels[0].type() == CV_8U);

	//fileter2D  只接受<=16bit int
	Mat ret = Mat::zeros(input.size(), CV_16U);
	for (Mat i: channels) {
		i.convertTo(i, CV_16U);
		ret += i;
	}
	return ret;
}


Mat sobel_energy(const Mat& input)
{
	Mat tmp = add_channels(input);
	Mat tmp2 = tmp.clone();

	Sobel(tmp, tmp2, -1, 1, 0, 3);
	Sobel(tmp, tmp, -1, 0, 1, 3);
	tmp += tmp2;

	Mat enemat;
	tmp.convertTo(enemat, CV_32S);

	return enemat;
}

Mat laplace_energy(const Mat& input)
{
	Mat tmp = add_channels(input);
	Laplacian(tmp, tmp, -1, 3);
	return tmp;
}