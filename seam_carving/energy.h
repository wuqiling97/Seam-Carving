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

typedef std::function<Mat(const Mat&)> GradOperator;

template <typename vec>
double RGBlength(const vec& v)
{
	int sum = 0;
	for (int i = 0; i < 3; i++) {
		sum += v[i] * v[i];
	}
	return sqrt(sum);
}

template <typename vec>
double RGBdistance(const vec& a, const vec& b)
{
	vec d = a - b;
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
	// 先计算导数, 后计算平方和的根(长度), 再相加
	Mat tmpx, tmpy;
	input.convertTo(tmpx, CV_16UC3);
	input.convertTo(tmpy, CV_16UC3);
	Sobel(tmpx, tmpx, -1, 1, 0, 3);
	Sobel(tmpy, tmpy, -1, 0, 1, 3);
	Mat ene = Mat::zeros(input.size(), CV_32S);
	for (int i = 0; i < ene.rows; i++) {
		for (int j = 0; j < ene.cols; j++) {
			ene.at<int>(i, j) = int(RGBlength(tmpx.at<Vec3s>(i, j)) + 
				                RGBlength(tmpy.at<Vec3s>(i, j)));
		}
	}

	return ene;
}

Mat scharr_energy(const Mat& input)
{
	// 先计算导数, 后计算平方和的根(长度), 再相加
	Mat tmpx, tmpy;
	input.convertTo(tmpx, CV_16UC3);
	input.convertTo(tmpy, CV_16UC3);
	Scharr(tmpx, tmpx, -1, 1, 0);
	Scharr(tmpy, tmpy, -1, 0, 1);
	
	Mat ene = Mat::zeros(input.size(), CV_32S);
	for (int i = 0; i < ene.rows; i++) {
		for (int j = 0; j < ene.cols; j++) {
			ene.at<int>(i, j) = int(RGBlength(tmpx.at<Vec3s>(i, j)) +
				                RGBlength(tmpy.at<Vec3s>(i, j)));
		}
	}

	return ene;
}

Mat laplace_energy(const Mat& input)
{
	Mat tmp = add_channels(input);
	Laplacian(tmp, tmp, -1, 3);
	tmp.convertTo(tmp, CV_32S);
	return tmp;
}