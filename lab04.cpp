#include "stdafx.h"
#include "common.h"
#include "lab04.h"
#include "utils.h"

Mat_<Vec3b> getFilteredImage(const Mat_<Vec3b>& srcImg, const Vec3b& filter, const Vec3b& maskColor, const Vec3b& bgColor = Vec3b(255, 255, 255)) {
	Mat_<Vec3b>& outputImg = srcImg.clone();

	for (int r = 0; r < srcImg.rows; r++) {
		for (int c = 0; c < srcImg.cols; c++) {
			if (srcImg(r, c) == filter)
				outputImg(r, c) = maskColor;
			else
				outputImg(r, c) = bgColor;
		}
	}

	return outputImg;
}

int calcArea(const Mat_<Vec3b>& img, const Vec3b& filter) {
	int area = 0;

	for (int r = 0; r < img.rows; r++) {
		for (int c = 0; c < img.cols; c++) {
			if (img(r, c) != filter) continue;
			area++;
		}
	}

	return area;
}

Point calcCenter(const Mat_<Vec3b>& img, const Vec3b& filter, const int area) {
	int sumR = 0;
	int sumC = 0;

	for (int r = 0; r < img.rows; r++) {
		for (int c = 0; c < img.cols; c++) {
			if (img(r, c) != filter) continue;
			sumR += r;
			sumC += c;
		}
	}

	return Point(sumC / area, sumR / area);
}

double calcElongationAxis(const Mat_<Vec3b>& img, const Vec3b& filter, const Point& center) {
	double numerator = 0;
double denominator = 0;

for (int r = 0; r < img.rows; r++) {
	for (int j = 0; j < img.cols; j++) {
		if (img(r, j) != filter) continue;

		double rowDiff = 1.0 * r - center.y;
		double colDiff = 1.0 * j - center.x;

		numerator += rowDiff * colDiff;
		denominator += pow(colDiff, 2) - pow(rowDiff, 2);
	}
}

return atan2(2 * numerator, denominator) / 2;
}

std::vector<Point> neighbours2 = { {-1, 0}, {0, -1} };
std::vector<Point> neighbours4 = { {-1, 0}, {-1, -1}, {0, -1}, {1, -1} };
bool isBorder(const Mat_<Vec3b>& img, const Point& p, const Vec3b& background = Vec3b(255, 255, 255)) {
	for (auto& point : neighbours4) {
		auto p1 = point + p;
		auto p2 = mirror(point) + p;

		if (isInside(img, p1) && img(p1) == background) return true;
		if (isInside(img, p2) && img(p2) == background) return true;
	}

	return false;
}

int calcPerimeter(const Mat_<Vec3b>& img, const Vec3b& filter, const Vec3b& background = Vec3b(255, 255, 255)) {
	int perimeter = 0;

	for (int r = 0; r < img.rows; r++) {
		for (int c = 0; c < img.cols; c++) {
			if (img(r, c) != filter) continue;
			if (!isBorder(img, Point(c, r), background)) continue;

			perimeter++;
		}
	}

	return perimeter; // * PI / 4;
}

double calcThinnesRatio(int area, int perimeter) {
	return 4 * PI * area / pow(perimeter, 2);
}

double calcAspectRatio(const Mat_<Vec3b>& img, const Vec3b& filter) {
	bool initialised = false;
	int rmin, rmax;
	int cmin, cmax;

	for (int r = 0; r < img.rows; r++) {
		for (int c = 0; c < img.cols; c++) {
			if (img(r, c) != filter) continue;
			if (!initialised) {
				initialised = true;
				rmin = rmax = r;
				cmin = cmax = c;
				continue;
			}

			rmin = min(rmin, r);
			rmax = max(rmax, r);
			cmin = min(cmin, c);
			cmax = max(cmax, c);
		}
	}

	std::vector<Point> points = { {cmin, rmin}, {cmax, rmin}, {cmax, rmax}, {cmin, rmax} };
	for (int i = 0; i < points.size(); i++) {
		line(img, points[i], points[(i + 1) % points.size()], Vec3b(255, 0, 255));
	}

	return 1.0 * (cmax - cmin + 1) / (rmax - rmin + 1);
}

void drawCenter(const Mat_<Vec3b>& img, const Point& center, int scale = 10, const Vec3b& color = Vec3b(0, 215, 255)) {
	for (auto& point : neighbours2) {
		auto p1 = point * scale + center;
		auto p2 = mirror(point) * scale + center;

		line(img, p1, p2, color);
	}
}

void drawElongationAxis(const Mat_<Vec3b>& img, const Point& center, double angle, const Vec3b& color = Vec3b(0, 255, 0)) {
	auto initialPoint = Point(2 * img.rows, 0);
	auto rotatedPoint = rotate(initialPoint, angle);
	auto mirrorPoint = mirror(rotatedPoint);

	auto p1 = rotatedPoint + center;
	auto p2 = mirrorPoint + center;

	line(img, p1, p2, color);
}

Mat_<Vec3b> carveObject(const Mat_<Vec3b>& img, const Vec3b& filter, const Vec3b& background = Vec3b(255, 255, 255)) {
	Mat_<Vec3b>& carvedImg = img.clone();
	for (int r = 0; r < img.rows; r++) {
		for (int c = 0; c < img.cols; c++) {
			if (img(r, c) != filter) continue;
			if (isBorder(img, Point(c, r), background)) continue;
			carvedImg(r, c) = background;
		}
	}

	return carvedImg;
}

int h_proj(const Mat_<Vec3b>& img, const Vec3b& filter, int row) {
	int cnt = 0;

	for (int c = 0; c < img.cols; c++) {
		if (img(row, c) != filter) continue;
		cnt++;
	}

	return cnt;
}

int v_proj(const Mat_<Vec3b>& img, const Vec3b& filter, int col) {
	int cnt = 0;

	for (int r = 0; r < img.rows; r++) {
		if (img(r, col) != filter) continue;
		cnt++;
	}

	return cnt;
}

void drawProjection(const Mat_<Vec3b>& img, const Vec3b& filter, const Vec3b& color = Vec3b(255, 0, 255), const Vec3b& background = Vec3b(255, 255, 255)) {
	Mat_<Vec3b> hImg(img.rows, img.cols); hImg.setTo(background);
	Mat_<Vec3b> vImg(img.rows, img.cols); vImg.setTo(background);

	for (int r = 0; r < img.rows; r++) {
		int pixels = h_proj(img, filter, r);
		for (int c = 0; c < pixels; c++) {
			hImg(r, c) = color;
		}
	}

	for (int c = 0; c < img.cols; c++) {
		int pixels = v_proj(img, filter, c);
		for (int r = 0; r < pixels; r++) {
			vImg(r, c) = color;
		}
	}

	imshow("h_proj", hImg);
	imshow("v_proj", vImg);
}

void onMouse(int event, int x, int y, int flags, void* param) {
	if (event != EVENT_LBUTTONDOWN) return;

	const Vec3b bgColor = Vec3b(255, 255, 255);

	Mat_<Vec3b>& img = *((Mat_<Vec3b>*) param);
	Vec3b filter = img(y, x);
	Vec3b& mask = Vec3b(0, 0, 0);

	if (filter == bgColor) {
		printf("Fundal! Try again!\n");
		return;
	}

	Mat_<Vec3b>& filteredImg = getFilteredImage(img, filter, mask);

	printf("\n\n");

	int area = calcArea(filteredImg, mask);
	printf("Aria: %d\n", area);

	Point& center = calcCenter(filteredImg, mask, area);
	printf("Centru de greutate: (%d, %d)\n", center.x, center.y);

	double elongationAngle = calcElongationAxis(filteredImg, mask, center);
	printf("Unghiul de alungire: %f\n", elongationAngle);

	int perimeter = calcPerimeter(filteredImg, mask);
	printf("Perimetru: %d\n", perimeter);

	double thinnesRatio = calcThinnesRatio(area, perimeter);
	printf("Unghiul de alungire: %f\n", thinnesRatio);

	drawProjection(filteredImg, mask, filter);

	double aspectRatio = calcAspectRatio(filteredImg, mask);
	printf("Elongatia: %f\n", aspectRatio);

	auto outputImage = carveObject(filteredImg, mask);
	drawElongationAxis(outputImage, center, elongationAngle);
	drawCenter(outputImage, center);
	imshow("output", outputImage);
}

void ex1() {
	char fname[MAX_PATH];
	if (openFileDlg(fname)) {
		Mat_<Vec3b> img = imread(fname, IMREAD_COLOR);

		imshow("image", img);
		setMouseCallback("image", onMouse, &img);
		waitKey(0);
	}
}

void addLab04ToMenu(std::vector<MenuItem>& menuList) {
	menuList.push_back(createMenuItem("ex1", ex1));
}
