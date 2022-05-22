#include "stdafx.h"
#include "common.h"
#include "lab06.h"
#include "utils.h"
#include <fstream>

std::vector<Point> neighb_offset = { {1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, {1, 1} };

std::vector<int> get_contour(const Mat_<uchar>& img, const Point& start) {
	std::vector<int> dirs;
	int dir = 7;
	Point last(-1, -1);
	Point curr = start;

	Point second(-1, -1);
	bool found = false;

	while(curr != second) {
		if (dir % 2 == 0) {
			dir = (dir + 7) % 8;
		} else {
			dir = (dir + 6) % 8;
		}

		for(int i = 0; i < 8; i++) {
			auto new_dir = (dir + i) % 8;
			Point p = curr + neighb_offset[new_dir];
			if (!isInside(img, p)) continue;
			if (img(p) != img(curr)) continue;
			if (!found && last == start) {
				second = curr;
				found = true;
			}
			/*std::cout << "===========================" << std::endl;
			std::cout << "curr: " << curr << std::endl;
			std::cout << "dir: " << new_dir << std::endl;
			std::cout << "offset: " << neighb_offset[dir] << std::endl;
			std::cout << "p: " << p << std::endl;
			std::cout << "last: " << last << std::endl;*/
			dirs.push_back(new_dir);
			dir = new_dir;
			last = curr;
			curr = p;
			break;
		}
	}

	dirs.pop_back();
	dirs.pop_back();
	return dirs;
}

int wraparound(int val, int min, int max) {
	if (val < min) {
		auto diff = abs(min - val);
		return max - diff;
	}
	if (val >= max) {
		auto diff = abs(val - max);
		return min + diff;
	}
	return val;
}

std::vector<int> get_derivative(const std::vector<int>& contour) {
	std::vector<int> derivative;
	for (int i = 0; i < contour.size(); i++) {
		auto behind = wraparound(i - 1, 0, contour.size());

		auto d = wraparound(contour[i] - contour[behind], 0, 8);
		derivative.push_back(d);
	}

	return derivative;
}

Point get_first_pixel(const Mat_<uchar>& img) {
	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			Point p(j, i);
			if (img(p) == 0) return p;
		}
	}
}

void draw_contour(Mat_<uchar>& img, const Point& start, const std::vector<int>& contour, const uchar& fillColor = 255) {
	auto curr = start;
	for (auto& dir : contour) {
		curr += neighb_offset[dir];
		img(curr) = fillColor;
	}
}

void ex6_1() {
	char fname[MAX_PATH];
	if (openFileDlg(fname)) {
		Mat_<Vec3b> img = imread(fname, IMREAD_COLOR);
		auto bwImg = RGB2BlackWhite(img);

		Mat_<uchar> conturImg(img.rows, img.cols);
		conturImg.setTo(0);

		auto start = get_first_pixel(bwImg);
		auto contur = get_contour(bwImg, start);
		draw_contour(conturImg, start, contur);

		imshow("imagine originala", img);
		imshow("contur", conturImg);
		waitKey(0);
	}
}

void ex6_2() {
	char fname[MAX_PATH];
	if (openFileDlg(fname)) {
		Mat_<Vec3b> img = imread(fname, IMREAD_COLOR);
		auto bwImg = RGB2BlackWhite(img);

		auto start = get_first_pixel(bwImg);
		auto cod = get_contour(bwImg, start);
		auto derivata = get_derivative(cod);

		char outname[MAX_PATH];
		sprintf(outname, "%s.cod.derivata.txt", fname);
		std::ofstream fout(outname);
		fout << "Cod inlantuit:" << std::endl;
		for (auto& n : cod) {
			fout << n << " ";
		}
		fout << std::endl << "Derivata:" << std::endl;
		for (auto& n : derivata) {
			fout << n << " ";
		}
		fout.close();
	}
}

void ex6_3() {
	char fname[MAX_PATH];
	char inname[MAX_PATH];
	if (openFileDlg(fname) && openFileDlg(inname)) {
		Mat_<uchar> img = imread(fname, IMREAD_GRAYSCALE);
		
		std::ifstream in(inname);
		int i, j;
		int cnt;
		in >> i >> j >> cnt;
		std::vector<int> contur;
		for (int i = 0; i < cnt; i++) {
			int x;
			in >> x;
			contur.push_back(x);
		}
		in.close();

		draw_contour(img, { j, i }, contur);

		imshow("imagine", img);
		waitKey(0);
	}
}

void addLab06ToMenu(std::vector<MenuItem>& menuList) {
	menuList.push_back(createMenuItem("ex1", ex6_1));
	menuList.push_back(createMenuItem("ex2", ex6_2));
	menuList.push_back(createMenuItem("ex3", ex6_3));
}