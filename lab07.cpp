#include "stdafx.h"
#include "common.h"
#include "lab07.h"
#include "utils.h"

Mat_<uchar> expand(const Mat_<uchar>& img, const Mat_<uchar>& kernel, const Point& origin = Point(1, 1), const uchar& color = 0, const uchar& bgColor = 255) {
	Mat_<uchar> output(img.rows, img.cols, bgColor);

	auto offsetX = origin.x;
	auto offsetY = origin.y;
	for (int i = offsetY; i < img.rows - (kernel.rows - 1 - origin.y); i++) {
		for (int j = offsetX; j < img.cols - (kernel.cols - 1 - origin.x); j++) {
			Point p(j, i);
			if (img(p) == bgColor) continue;
			for (int r = 0; r < kernel.rows; r++) {
				for (int c = 0; c < kernel.cols; c++) {
					Point target = p + Point(c, r);
					output(target) = color;
				}
			}
		}
	}

	return output;
}

Mat_<uchar> erode(const Mat_<uchar>& img, const Mat_<uchar>& kernel, const Point& origin = Point(1, 1), const uchar& color = 0, const uchar& bgColor = 255) {
	Mat_<uchar> output(img.rows, img.cols, bgColor);

	auto offsetX = kernel.cols / 2;
	auto offsetY = kernel.rows / 2;
	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			Point p(j, i);
			bool ok = true;
			for (int r = 0; ok && r < kernel.rows; r++) {
				for (int c = 0; ok && c < kernel.cols; c++) {
					Point kernel_offset = Point(c, r) - origin;
					Point target = p + kernel_offset;
					if (!isInside(img, target)) continue;
					if (kernel(r, c) == bgColor) continue;
					ok &= img(target) == kernel(r, c);
				}
			}
			if (!ok) continue;
			output(p) = color;
		}
	}

	return output;
}

void dilatare() {
	char fname[MAX_PATH];
	if (openFileDlg(fname)) {
		Mat_<Vec3b> img = imread(fname, IMREAD_COLOR);
		auto bwImg = RGB2BlackWhite(img);

		Mat_<uchar> neighb4_kernel(3, 3, (uchar)0);
		neighb4_kernel(0, 0) = 255;
		neighb4_kernel(0, 2) = 255;
		neighb4_kernel(2, 0) = 255;
		neighb4_kernel(2, 2) = 255;
		auto expandedImg = expand(bwImg, neighb4_kernel);

		imshow("original image", bwImg);
		imshow("expanded image", expandedImg);
		waitKey(0);
	}
}

void eroziune() {
	char fname[MAX_PATH];
	if (openFileDlg(fname)) {
		Mat_<Vec3b> img = imread(fname, IMREAD_COLOR);
		auto bwImg = RGB2BlackWhite(img);

		Mat_<uchar> neighb4_kernel(3, 3, (uchar)0);
		neighb4_kernel(0, 0) = 255;
		neighb4_kernel(0, 2) = 255;
		neighb4_kernel(2, 0) = 255;
		neighb4_kernel(2, 2) = 255;
		std::cout << neighb4_kernel << std::endl;
		auto erodedImg = erode(bwImg, neighb4_kernel);

		imshow("original image", bwImg);
		imshow("eroded image", erodedImg);
		waitKey(0);
	}
}

void addLab07ToMenu(std::vector<MenuItem>& menuList) {
	menuList.push_back(createMenuItem("dilatare", dilatare));
	menuList.push_back(createMenuItem("eroziune", eroziune));
}