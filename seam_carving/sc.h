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

#include "energy.h"

using std::cout;
using std::endl;
using std::cin;
using std::string;
using namespace cv;


template<typename T>
inline T minene_above(Mat& m, int i, int j, Mat& pointer)
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

void cut1seam(Mat& origin)
{
	Mat enemat = e1(origin);

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
	int tmp = INT_MAX;
	for (int j = 0; j < cols; j++) {
		if (enemat.at<int>(rows - 1, j) < tmp) {
			tmp = enemat.at<int>(rows-1, j);
			minene_col = j;
		}
	}
	//cout<<"min energy column "<<minene_col<<endl;

	//back trace
	//seam[row] = col selected
	std::vector<int> seam(rows);
	for (int i = rows - 1; i >= 0; i--) {
		seam[i] = minene_col;
		minene_col = pointer.at<int>(i, minene_col);
	}
	
	//for (int i = 0; i < rows; i++) {
	//	origin.at<Vec3b>(i, seam[i]) = Vec3b(0, 0, 255);
	//}
	//imshow("hello", origin);
	//waitKey();

	//delete seam
	for (int i = 0; i < rows; i++) {
		int seami = seam[i];
		//平滑图像
		if(seami-1 >= 0)
			origin.at<Vec3b>(i, seami-1) = (Vec3i(origin.at<Vec3b>(i, seami)) + Vec3i(origin.at<Vec3b>(i, seami-1)))/2;
		if (seami + 1 < cols)
			origin.at<Vec3b>(i, seami + 1) = (Vec3i(origin.at<Vec3b>(i, seami)) + Vec3i(origin.at<Vec3b>(i, seami + 1))) / 2;

		for (int j = seami; j < cols-1; j++) {
			origin.at<Vec3b>(i, j) = origin.at<Vec3b>(i, j+1);
		}
	}
	origin = origin.colRange(0, cols-1);
}

Mat scCut(Mat origin, int len, bool cutwidth=true)
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
		cut1seam(ret);
		printf("%d lines cut\n", i);
	}
	if(cutwidth)
		return ret;
	else
		return ret.t();
}