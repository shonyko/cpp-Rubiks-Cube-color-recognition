#include "stdafx.h"
#include "utils.h"

bool isInside(const Mat& img, int r, int c) {
	bool ok = true;

	ok &= (r >= 0 && r < img.rows);
	ok &= (c >= 0 && c < img.cols);

	return ok;
}
bool isInside(const Mat& img, const Point& p) {
	return isInside(img, p.y, p.x);
}

int clamp(int minval, int val, int maxval) {
	return min(max(minval, val), maxval);
}

Point mirror(const Point& point) {
	return Point(-point.x, -point.y);
}
Point rotate(const Point& point, double angle) {
	int x, y;

	auto sinus = std::sin(angle);
	auto cosinus = std::cos(angle);

	x = point.x * cosinus - point.y * sinus;
	y = point.x * sinus + point.y * cosinus;

	return Point(x, y);
}

Mat_<uchar> RGB2Gray(const Mat_<Vec3b>& img) {
	Mat_<uchar> result(img.rows, img.cols);

	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			auto r = img(i, j)[2];
			auto g = img(i, j)[1];
			auto b = img(i, j)[0];
			//result(i, j) = 1/3.0 * r + 1/3.0 * g + 1/3.0 * b;
			result(i, j) = 0.299 * r + 0.587 * g + 0.114 * b;
		}
	}

	return result;
}

Mat_<uchar> Gray2BlackWhite(const Mat_<uchar>& img, int threshold) {
	Mat_<uchar> result(img.rows, img.cols);

	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			uchar value = (img(i, j) >= threshold) * 255;
			result(i, j) = value;
		}
	}

	return result;
}

Mat_<uchar> RGB2BlackWhite(const Mat_<Vec3b>& img, const int threshold) {
	printf("%d\n", threshold);
	auto grayImg = RGB2Gray(img);
	auto bwImg = Gray2BlackWhite(grayImg, threshold);

	return bwImg;
}

Mat_<uchar> Gray2Negative(const Mat_<uchar>& img) {
	Mat_<uchar> negativeImg(img.rows, img.cols, (uchar)0);

	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			negativeImg(i, j) = 255 - img(i, j);
		}
	}

	return negativeImg;
}

void calcHistogram(int* hist, const Mat_<uchar>& img) {
	for (int i = 0; i < 256; i++) hist[i] = 0;
	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			hist[img(i, j)]++;
		}
	}
}

void showHistogram(const std::string& name, int* hist, const int  hist_cols, const int hist_height) {
	Mat imgHist(hist_height, hist_cols, CV_8UC3, CV_RGB(255, 255, 255)); // constructs a white image

	//computes histogram maximum
	int max_hist = 0;
	for (int i = 0; i < hist_cols; i++)
		if (hist[i] > max_hist)
			max_hist = hist[i];
	double scale = 1.0;
	scale = (double)hist_height / max_hist;
	int baseline = hist_height - 1;

	for (int x = 0; x < hist_cols; x++) {
		Point p1 = Point(x, baseline);
		Point p2 = Point(x, baseline - cvRound(hist[x] * scale));
		line(imgHist, p1, p2, CV_RGB(255, 0, 255)); // histogram bins colored in magenta
	}

	imshow(name, imgHist);
}
int minHistValue(int* hist, int size) {
	int mini = hist[0];
	for (int i = 1; i < size; i++) {
		mini = min(mini, hist[i]);
	}
	return mini;
}
int maxHistValue(int* hist, int size) {
	int maxi = hist[0];
	for (int i = 1; i < size; i++) {
		maxi = max(maxi, hist[i]);
	}
	return maxi;
}
int minHistIntensity(int* hist, int size) {
	int mini = hist[0];
	int intensity = 0;
	for (int i = 1; i < size; i++) {
		if (hist[i] < mini) {
			mini = hist[i];
			intensity = i;
		}
	}
	return intensity;
}
int maxHistIntensity(int* hist, int size) {
	int maxi = hist[0];
	int intensity = 0;
	for (int i = 1; i < size; i++) {
		if (hist[i] > maxi) {
			maxi = hist[i];
			intensity = i;
		}
	}
	return intensity;
}

void calcFDP(float* fdp, Mat_<uchar> img) {
	int hist[256];
	calcHistogram(hist, img);
	int M = img.rows * img.cols;
	for (int i = 0; i < 256; i++) {
		fdp[i] = 1.0 * hist[i] / M;
	}
}
void calcFDPC(float* fdpc, float* fdp, int size) {
	for (int i = 0; i < size; i++) {
		fdpc[i] = 0;
		for (int j = 0; j <= i; j++) {
			fdpc[i] += fdp[j];
		}
	}
}