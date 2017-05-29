#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <fstream>
#include <cassert>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "sc.h"
using std::cout;
using std::endl;
using std::cin;
using std::string;
using namespace cv;

int main()
{
	string imgname;
	std::ifstream fin("imgname.txt");
	fin>>imgname;
	cout<<imgname<<"|\n";

	Mat origin_img = imread(imgname, CV_LOAD_IMAGE_COLOR);
	assert(origin_img.type()==CV_8UC3);

	if (origin_img.data == nullptr) {
		cout<<"no such image";
		system("pause");
		return 0;
	}

	cout<<"image size: "<<origin_img.size()<<endl;
	cout<<"input the width you want to cut\n";
	int w;
	cin>>w;

	const string winname = "hello";
	namedWindow(winname, CV_WINDOW_AUTOSIZE);

	Mat res_img = scCut(origin_img, w);

	imshow(winname, res_img);

	waitKey();
}
