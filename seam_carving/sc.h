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
	Vec3b color;
	SeamPoint(int r, int c, Vec3b color): row(r), col(c), color(color) { }
	cv::Point2i cvpoint() {
		return cv::Point2i(col, row);
	}
	SeamPoint asswap() {
		return SeamPoint(col, row, color);
	}
	SeamPoint& swap() {
		std::swap(row, col);
		return *this;
	}
};

typedef std::vector<SeamPoint> Seam;

template<typename T>
inline T minene_above(const Mat& m, int i, int j, Mat& pointer)
{
	T& ref = pointer.at<T>(i, j);

	T minene = INT_MAX;
	int tmp = min(j+1, m.cols-1);
	for (int col = max(j - 1, 0); col <= tmp; col++) {
		if (m.at<T>(i-1, col) < minene) {
			minene = m.at<T>(i-1, col);
			ref = col;
		}
	}
	return minene;
}

Seam get_seam(Mat& enemat, const Mat& img)
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
		seam.push_back(SeamPoint(i, minene_col, img.at<Vec3b>(i, minene_col)));
		minene_col = pointer.at<int>(i, minene_col);
	}
	return seam;
}

Seam get_horizon_seam(Mat& enemat, const Mat& img) {
	Mat enemat_trans = enemat.t();
	Seam seam = get_seam(enemat_trans, img.t());
	for (SeamPoint& point : seam) {
		std::swap(point.row, point.col);
	}
	return seam;
}


class WorkStation
{
	const Mat origin;
	const GradOperator gradop;
	std::vector<Seam> seamseq;

	void cut1seam(Mat& cutimg)
	{
		Mat enemat = gradop(cutimg);
		assert(enemat.type() == CV_32S);

		/*Mat showmat;
		enemat /= cutimg.channels();
		enemat.convertTo(showmat, CV_8U);
		imshow("hello", showmat);
		waitKey();*/
		
		seamseq.push_back(get_seam(enemat, cutimg));
		Seam& seam = seamseq.back();

		// delete seam
		for (int i = 0; i < cutimg.rows; i++) {
			const int& scol = seam[i].col; //seam column
			const int& srow = seam[i].row;
			//平滑图像
			if (scol - 1 >= 0)
				cutimg.at<Vec3b>(srow, scol - 1) =
				(Vec3i(cutimg.at<Vec3b>(srow, scol)) + Vec3i(cutimg.at<Vec3b>(srow, scol - 1))) / 2;
			if (scol + 1 < cutimg.cols)
				cutimg.at<Vec3b>(srow, scol + 1) =
				(Vec3i(cutimg.at<Vec3b>(srow, scol)) + Vec3i(cutimg.at<Vec3b>(srow, scol + 1))) / 2;

			for (int j = scol; j < cutimg.cols - 1; j++) {
				cutimg.at<Vec3b>(srow, j) = cutimg.at<Vec3b>(srow, j + 1);
			}
		}
		cutimg = cutimg.colRange(0, cutimg.cols - 1);
	}

	Mat addSeam(const Mat& origin, int addlen)
	{
		Mat cutimg = origin.clone();

		for (int n = 0; n < addlen; n++) {
			Mat enemat = gradop(cutimg);
			seamseq.push_back(get_seam(enemat, cutimg));
			const Seam& seam = seamseq.back();
			// delete seam
			for (int i = 0; i < cutimg.rows; i++) {
				const int& scol = seam[i].col; //seam column
				const int& srow = seam[i].row;
				for (int j = scol; j < cutimg.cols - 1; j++) {
					cutimg.at<Vec3b>(srow, j) = cutimg.at<Vec3b>(srow, j + 1);
				}
			}
			cutimg = cutimg.colRange(0, cutimg.cols - 1);

			printf("%d lines cut\n", n + 1);
		}
		cout << "cut finished\n";

		Mat isdelete = Mat::zeros(origin.size(), CV_8U);
		Mat retimg;
		cv::copyMakeBorder(origin, retimg, 0, 0, 0, addlen, cv::BORDER_CONSTANT);

		// 把addlen条seam加回来, 记录下被删过的点
		for (int n = addlen - 1; n >= 0; n--) {
			const Seam& seam = seamseq[n];
			int cols = cutimg.cols + addlen - n;
			for (SeamPoint p : seam) {
				// 移动像素, 为删去的像素提供空间 // 没必要,只要记录被删过的像素即可
				// 并且移动delete标记
				for (int j = cols - 1; j > p.col; j--) {
					isdelete.at<uchar>(p.row, j) = isdelete.at<uchar>(p.row, j - 1);
				}
				isdelete.at<uchar>(p.row, p.col) = true;
			}
			printf("line %d add\n", n + 1);
		}

		// 把记录下的点再加一遍
		for (int i = 0; i < origin.rows; i++) {
			int retp = retimg.cols - 1;
			int originp = origin.cols - 1;
			// ret pointer 指向retimg的列
			for (; originp >= 0; originp--) {
				retimg.at<Vec3b>(i, retp) = retimg.at<Vec3b>(i, originp);
				retp--;
				if (isdelete.at<uchar>(i, originp) == true) {
					retimg.at<Vec3b>(i, retp) = retimg.at<Vec3b>(i, originp);
					retp--;
				}
			}
			CV_Assert(originp == -1 && retp == -1);
		}

		cout << "add seam finished\n";

		return retimg;
	}

	Mat bicut(const Mat& origin, int wlen, int hlen) {
		int tot_cutlen = wlen + hlen;
		Mat cutimg = origin.clone();
		int wcuttime = 0, hcuttime = 0;

		for (int n = 0; n < tot_cutlen; n++) {
			Mat vert_ene = gradop(cutimg); //w*h matrix, dtype: int
			Mat hori_ene = gradop(cutimg.t()); //h*w matrix
			Seam vert_seam = get_seam(vert_ene, origin);
			Seam hori_seam = get_seam(hori_ene, origin.t()); // 行列颠倒

			// caculate average seam energy
			int vert_minene = vert_ene.at<int>(vert_seam[0].cvpoint());
			int hori_minene = hori_ene.at<int>(hori_seam[0].cvpoint());
			double vert_aveE = vert_ene.at<int>(vert_seam[0].cvpoint()) / double(vert_seam.size());
			double hori_aveE = hori_ene.at<int>(hori_seam[0].cvpoint()) / double(hori_seam.size());

			Mat tmp;
			Seam* seam = nullptr;
			bool cutwidth;
			assert(wcuttime < wlen || hcuttime < hlen);
			if ((vert_aveE < hori_aveE && wcuttime < wlen) || (vert_aveE >= hori_aveE && hcuttime == hlen)) {
				tmp = cutimg;
				seam = &vert_seam;
				wcuttime++;
				cutwidth = true;
				printf("n: %d, width\n", n);
			} else {
				tmp = cutimg.t();
				seam = &hori_seam;
				hcuttime++;
				cutwidth = false;
				printf("n: %d, height\n", n);
			}

			// cut image
			for (const SeamPoint& point: *seam) {
				const int& scol = point.col; //seam column
				const int& srow = point.row;
				for (int j = scol; j < tmp.cols - 1; j++) {
					tmp.at<Vec3b>(srow, j) = tmp.at<Vec3b>(srow, j + 1);
				}
			}
			if(cutwidth)
				cutimg = cutimg.colRange(0, cutimg.cols-1);
			else
				cutimg = cutimg.rowRange(0, cutimg.rows-1);
		}

		return cutimg;
	}
public:
	WorkStation(Mat img, GradOperator gradoperator) :
		origin(img), gradop(gradoperator)
	{}

	void showSeams(bool iswidth, string path="") {
		Mat originimg = origin.clone();
		if(!iswidth)
			originimg = originimg.t();

		Mat isdelete = Mat::zeros(originimg.size(), CV_8U);
		int addlen = seamseq.size();

		// 把addlen条seam加回来, 记录下被删过的点
		for (int n = addlen - 1; n >= 0; n--) {
			const Seam& seam = seamseq[n];
			int cols = originimg.cols - n;
			for (SeamPoint p : seam) {
				// 移动像素, 为删去的像素提供空间 // 没必要,只要记录被删过的像素即可
				// 并且移动delete标记
				for (int j = cols - 1; j > p.col; j--) {
					isdelete.at<uchar>(p.row, j) = isdelete.at<uchar>(p.row, j - 1);
				}
				isdelete.at<uchar>(p.row, p.col) = true;
			}
			//printf("line %d add\n", n + 1);
		}

		Mat show = originimg;
		for (int i = 0; i<show.rows; i++)
			for (int j = 0; j < show.cols; j++) {
				if (isdelete.at<uchar>(i, j))
					show.at<Vec3b>(i, j) = Vec3b(0, 0, 255);
			}
		
		if(!iswidth)
			show = show.t();
		imshow("red seam", show);
		if(path!="")
			imwrite(path, show);
		waitKey();
	}

	Mat bidirection_cut(int width, int height) {
		seamseq.clear();
		Mat ret = origin.clone();
		assert(width<origin.cols && height<origin.rows);
		
		return bicut(ret, origin.cols-width, origin.rows-height);
	}

	Mat cut(int len, bool cutwidth = true)
	{
		seamseq.clear();
		int cutlen;
		Mat ret = origin.clone();
		if (cutwidth) {
			cutlen = origin.cols - len;
		} else {
			cutlen = origin.rows - len;
			ret = origin.t();
		}
		assert(len<origin.cols);

		for (int i = 1; i <= cutlen; i++) {
			cut1seam(ret);
			printf("%d lines cut\n", i);
		}
		if (cutwidth)
			return ret;
		else
			return ret.t();
	}

	Mat enlarge(int len, bool addwidth = true)
	{
		seamseq.clear();
		// addlen: 裁剪/放大的像素数
		int addlen;
		Mat ret = origin;
		if (addwidth) {
			addlen = len - origin.cols;
		} else {
			addlen = len - origin.rows;
			ret = origin.t();
		}
		assert(len>origin.cols);
		ret = addSeam(ret, addlen);

		if (addwidth)
			return ret;
		else
			return ret.t();
	}
};
