#pragma once

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <fstream>
#include <cassert>
#include <functional>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "energy.h"

using std::cout;
using std::endl;
using std::cin;
using std::string;
using namespace cv;

struct SeamPoint
{
	int row, col;
	SeamPoint(int r, int c): row(r), col(c) { }
};

typedef std::vector<SeamPoint> Seam;

template<typename T>
inline T minene_above(const Mat& m, int i, int j, Mat& pointer)
{
	T& ref = pointer.at<T>(i, j);

	T minene = INT_MAX;
	int tmp = min(j+1, m.cols-1);
	for (int col = max(j - 1, 0); col < tmp; col++) {
		if (m.at<T>(i-1, col) < minene) {
			minene = m.at<T>(i-1, col);
			ref = col;
		}
	}
	return minene;
}

Seam getSeam(Mat& enemat)
{
	int rows = enemat.rows, cols = enemat.cols;
	//指向上一行被选中的能量最小点
	Mat pointer = Mat::zeros(enemat.size(), enemat.type());
	for (int i = 1; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			enemat.at<int>(i, j) += minene_above<int>(enemat, i, j, pointer);
		}
	}

	//col with minimun energy at last row
	int minene_col = 0;
	int minene = INT_MAX;
	for (int j = 0; j < cols; j++) {
		if (enemat.at<int>(rows - 1, j) < minene) {
			minene = enemat.at<int>(rows - 1, j);
			minene_col = j;
		}
	}

	//back trace
	//seam[row] = col selected
	Seam seam;
	for (int i = rows - 1; i >= 0; i--) {
		seam.push_back(SeamPoint(i, minene_col));
		minene_col = pointer.at<int>(i, minene_col);
	}
	return seam;
}

void cut1seam(Mat& origin, GradOperator gradop)
{
	Mat enemat = gradop(origin);

	//Mat showmat;
	//enemat /= origin.channels();
	//enemat.convertTo(showmat, CV_8U);
	//imshow("hello", showmat);
	//waitKey();

	Seam seam = getSeam(enemat);
	
	//for (int i = 0; i < rows; i++) {
	//	origin.at<Vec3b>(i, seam[i]) = Vec3b(0, 0, 255);
	//}
	//imshow("hello", origin);
	//waitKey();

	//delete seam
	for (int i = 0; i < origin.rows; i++) {
		const int& scol = seam[i].col; //seam column
		const int& srow = seam[i].row;
		//平滑图像
		if(scol-1 >= 0)
			origin.at<Vec3b>(srow, scol-1) = 
			(Vec3i(origin.at<Vec3b>(srow, scol)) + Vec3i(origin.at<Vec3b>(srow, scol-1)))/2;
		if (scol + 1 < origin.cols)
			origin.at<Vec3b>(srow, scol + 1) = 
			(Vec3i(origin.at<Vec3b>(srow, scol)) + Vec3i(origin.at<Vec3b>(srow, scol + 1))) / 2;

		for (int j = scol; j < origin.cols-1; j++) {
			origin.at<Vec3b>(srow, j) = origin.at<Vec3b>(srow, j+1);
		}
	}
	origin = origin.colRange(0, origin.cols-1);
}

Mat scCut(Mat origin, GradOperator gradop, int len, bool cutwidth=true)
{
	int cutlen;
	Mat ret = origin;
	if (cutwidth) {
		cutlen = origin.cols - len;
	} else {
		cutlen = origin.rows - len;
		ret = origin.t();
	}
	for (int i = 1; i <= cutlen; i++) {
		cut1seam(ret, gradop);
		printf("%d lines cut\n", i);
	}
	if(cutwidth)
		return ret;
	else
		return ret.t();
}

Mat scAdd(Mat origin, int len, bool addwidth = true)
{
	int add;
	Mat ret = origin;
	if (addwidth) {
		add = len - origin.cols;
	} else {
		add = len - origin.rows;
		ret = origin.t();
	}



	if(addwidth)
		return ret;
	else
		return ret.t();
}