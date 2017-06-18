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
#include "energy.h"
using std::cout;
using std::endl;
using std::cin;
using std::string;
using namespace cv;

#define DEBUG

void getarg(bool& cutwidth, int& cutlen, std::istream& in, Mat& img)
{
	while (1) {
		cout << "which side do you want to cut, height or width? <h/w>\n";
		string side;
		in >> side;
		if (side == "w") {
			cutwidth = true;
			break;
		} else if (side == "h") {
			cutwidth = false;
			break;
		} else
			cout << "invalid, input again\n";
	}

	while (1) {
		cout << "input the length you want to cut to\n";
		in >> cutlen;
		break;
		/*if (cutwidth == true && cutlen >= img.cols ||
			cutwidth == false && cutlen >= img.rows)
			cout << "invalid length\n";
		else {
			break;
		}*/
	}
}

int main()
{
	string imgname;

#ifdef DEBUG
	std::ifstream fin("imgname.txt");
	auto& uin = fin;
#else
	auto& uin = cin;
#endif

	uin>>imgname;
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
	int len;
	getarg(cutwidth, len, uin, origin_img);

	const string winname = "hello";
	namedWindow(winname, CV_WINDOW_AUTOSIZE);

	Mat res_img = scCut(origin_img, sobel_energy, len, cutwidth);

	imshow(winname, res_img);
	imwrite("images/result.png", res_img);

	waitKey();
}
