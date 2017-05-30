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
	bool cutwidth = true;
	while (1) {
		cout << "which side do you want to cut, height or width? <h/w>\n";
		string side;
		cin >> side;
		if (side == "w") {
			cutwidth = true;
			break;
		}
		else if (side == "h") {
			cutwidth = false;
			break;
		}
		else
			cout << "invalid, input again\n";
	}

	int len;
	while (1) {
		cout << "input the length you want to cut to\n";
		cin >> len;
		if(cutwidth==true && len>=origin_img.cols ||
			cutwidth==false && len>=origin_img.rows)
			cout<<"invalid length\n";
		else {
			break;
		}
	}

	const string winname = "hello";
	namedWindow(winname, CV_WINDOW_AUTOSIZE);

	Mat res_img = scCut(origin_img, len, cutwidth);

	imshow(winname, res_img);
	imwrite("images/result.png", res_img);

	waitKey();
}
