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

void get_scinfo(bool& cutwidth, int& cutlen, bool& isadd, std::istream& in, Mat& img)
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

	cout << "input the length you want to cut to\n";
	in >> cutlen;
	if (cutwidth == true && cutlen >= img.cols ||
		cutwidth == false && cutlen >= img.rows) {
		cout << "enlarge image\n";
		isadd = true;
	} else {
		cout << "cut image\n";
		isadd = false;
	}
}

GradOperator getop(std::istream& in)
{
	string tmp;
	in>>tmp;
	while (1) {
		cout<<tmp<<endl;
		if (tmp == "e1")
			return e1;
		else if (tmp == "sobel")
			return sobel_energy;
		else if (tmp == "laplace")
			return laplace_energy;
		else if (tmp == "scharr")
			return scharr_energy;
		else
			cout << "invalid input";
	}
}

Mat getimg(std::istream& in)
{
	string imgname;
	in >> imgname;
	cout << imgname << "|\n";

	Mat origin_img = imread(imgname, CV_LOAD_IMAGE_COLOR);
	assert(origin_img.type() == CV_8UC3);
	if (origin_img.data == nullptr) {
		cout << "no such image";
		system("pause");
		throw 1;
	}
	cout << "image size: " << origin_img.size() << endl;
	return origin_img;
}

int main(int argc, char** argv)
{
#ifdef DEBUG
	std::ifstream fin("imgname.txt");
	auto& uin = fin;
#else
	auto& uin = cin;
#endif

	// get args
	GradOperator gradop = getop(uin);
	Mat origin_img = getimg(uin);

	enum Cuttype {HEIGHT, WIDTH, BOTH};
	Cuttype cuttype;
	bool isadd;
	int len, len2;
	//get_scinfo(cutwidth, len, isadd, uin, origin_img);

	while (1) {
		cout << "which side do you want to cut, height or width or both? <h/w/hw/wh>\n";
		string side;
		uin >> side;
		if (side == "w") {
			cuttype = WIDTH;
			break;
		} else if (side == "h") {
			cuttype = HEIGHT;
			break;
		} else if (side == "wh" || side == "hw") {
			cuttype = BOTH;
			break;
		} else
			cout << "invalid, input again\n";
	}

	cout << "input the length you want to cut to\n";
	uin >> len;
	if (cuttype == BOTH) {
		uin >> len2;
	}
	if (cuttype == WIDTH && len >= origin_img.cols ||
		cuttype == HEIGHT && len >= origin_img.rows) {
		cout << "enlarge image\n";
		isadd = true;
	} else if (cuttype != BOTH) {
		cout << "cut image\n";
		isadd = false;
	} else {
		cout<<"bidirection cut\n";
		isadd = false;
	}


	WorkStation ws = WorkStation(origin_img, gradop);

	Mat res_img;
	string seampath = "";
	if(argc==3)
		seampath = argv[2];

	if (isadd) {
		res_img = ws.enlarge(len, cuttype);
		ws.showSeams(cuttype, seampath);
	} else if(cuttype != BOTH) {
		res_img = ws.cut(len, cuttype);
		ws.showSeams(cuttype, seampath);
	} else {
		cout<<len<<' '<<len2<<endl;
		//system("pause");
		res_img = ws.bidirection_cut(len, len2);
	}
	const string winname = "hello";
	imshow(winname, res_img);
	string storepath = "images/result.png";
	if(argc==3)
		storepath = argv[1];
	imwrite(storepath, res_img);

	waitKey();
}
