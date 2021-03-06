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
int RGBSquare(const vec& v)
{
	int sum = 0;
	for (int i = 0; i < 3; i++) {
		sum += v[i] * v[i];
	}
	return sum;
}

template <typename vec>
double RGBlength(const vec& v)
{
	return sqrt(RGBSquare(v));
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

Mat e1(const Mat& input)
{
	Mat img_with_border;
	copyMakeBorder(input, img_with_border, 1, 1, 1, 1, BORDER_REPLICATE);
	Mat enemat = Mat(input.size(), CV_32S);
	for (int i = 1; i <= input.rows; i++)
		for (int j = 1; j <= input.cols; j++) {
			int grad = RGBdistance(img_with_border.at<Vec3b>(i - 1, j), img_with_border.at<Vec3b>(i + 1, j))
				+ RGBdistance(img_with_border.at<Vec3b>(i, j - 1), img_with_border.at<Vec3b>(i, j + 1));
			enemat.at<int>(i-1, j-1) = grad;
		}
	return enemat;
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
	/*Mat tmp;
	input.convertTo(tmp, -1);
	Laplacian(tmp, tmp, -1, 3);
	Mat ene = Mat(tmp.size(), CV_32S);
	for (int i = 0; i < ene.rows; i++) {
		for (int j = 0; j < ene.cols; j++) {
			ene.at<int>(i, j) = int(RGBlength(tmp.at<Vec3b>(i, j)));
		}
	}*/
	
	Mat ene = add_channels(input);
	Laplacian(ene, ene, -1, 3);
	ene.convertTo(ene, CV_32S);
	return ene;
}
