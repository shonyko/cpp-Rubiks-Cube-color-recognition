#include "stdafx.h"
#include "common.h"
#include "lab09.h"
#include "utils.h"

Mat_<float> convolutie(const Mat_<uchar>& img, const Mat_<float>& kernel, const Point& origin = Point(1, 1)) {
	Mat_<float> result(img.rows, img.cols);

	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			Point pimg(j, i);

			float sum = 0;
			for (int u = 0; u < kernel.rows; u++) {
				for (int v = 0; v < kernel.cols; v++) {
					Point pkernel(v, u);
					auto offset = pkernel - origin;
					auto point = pimg + offset;
					if (!isInside(img, point)) continue;
					sum += kernel(pkernel) * img(point);
				}
			}

			result(pimg) = sum;
		}
	}

	return result;
}

Mat_<uchar> normalizare(const bool& trece_jos, const Mat_<float>& conv, const Mat_<float>& kernel, const Point& origin = Point(1, 1)) {
	Mat_<uchar> result(conv.rows, conv.cols);

	float sump = 0;
	float sumn = 0;
	for (int i = 0; i < kernel.rows; i++) {
		for (int j = 0; j < kernel.cols; j++) {
			auto val = kernel(i, j);
			if (val > 0) sump += val;
			else sumn += val;
		}
	}

	auto c = max(sump, -sumn);
	for (int i = 0; i < conv.rows; i++) {
		for (int j = 0; j < conv.cols; j++) {
			Point p(j, i);
			uchar val;
			if (trece_jos) {
				val = abs(conv(p)) / c;
			} else {
				val = 128 + conv(p) / (2 * c);
			}
			
			result(p) = val;
		}
	}

	return result;
}

void trece_jos() {
	char fname[MAX_PATH];
	if (openFileDlg(fname)) {
		Mat_<Vec3b> img = imread(fname, IMREAD_COLOR);
		auto grayImg = RGB2Gray(img);

		Mat_<float> kernel3x3(3, 3, 1.0f);
		Point origin3x3(1, 1);
		Mat_<float> kernel5x5(5, 5, 1.0f);
		Point origin5x5(2, 2);


		auto conv = convolutie(grayImg, kernel3x3);
		auto trece_jos3x3 = normalizare(true, conv, kernel3x3);

		conv = convolutie(grayImg, kernel5x5, origin5x5);
		auto trece_jos5x5 = normalizare(true, conv, kernel5x5, origin5x5);

		/*auto gaussianKernel = getGaussianKernel(3, 3);
		for (int i = 0; i < gaussianKernel.rows; i++) {
			for (int j = 0; j < gaussianKernel.cols; j++) {
				printf("%f ", gaussianKernel.at<double>(i, j));
			}
			printf("\n");
		}*/
		Mat_<float> gaussianKernel(3, 3, 1.0f);
		gaussianKernel(0, 1) = 2;
		gaussianKernel(1, 2) = 2;
		gaussianKernel(2, 1) = 2;
		gaussianKernel(1, 0) = 2;
		gaussianKernel(1, 1) = 4;

		conv = convolutie(grayImg, gaussianKernel, origin3x3);
		auto gaussian = normalizare(true, conv, gaussianKernel, origin3x3);

		Mat_<float> kernel1x5(1, 5, 1.0f);
		Point origin1x5(2, 0);

		conv = convolutie(grayImg, kernel1x5, origin1x5);
		auto out1x5 = normalizare(true, conv, kernel1x5, origin1x5);

		imshow("original", grayImg);
		imshow("trece_jos 3x3", trece_jos3x3);
		imshow("trece_jos 5x5", trece_jos5x5);
		imshow("gaussian", gaussian);
		imshow("1x5", out1x5);
		waitKey();
	}
}

void trece_sus() {
	char fname[MAX_PATH];
	if (openFileDlg(fname)) {
		Mat_<Vec3b> img = imread(fname, IMREAD_COLOR);
		auto grayImg = RGB2Gray(img);

		Mat_<float> low_kernel3x3(3, 3, 1.0f);
		Point origin3x3(1, 1);


		auto conv = convolutie(grayImg, low_kernel3x3);
		auto trece_jos3x3 = normalizare(true, conv, low_kernel3x3);

		Mat_<float> laplaceKernel(3, 3, -1.0f);
		laplaceKernel(origin3x3) = 8;

		conv = convolutie(grayImg, laplaceKernel);
		auto trece_sus_laplace_orig = normalizare(false, conv, laplaceKernel);

		conv = convolutie(trece_jos3x3, laplaceKernel);
		auto trece_sus_laplace_filtrat = normalizare(false, conv, laplaceKernel);

		Mat_<float> high_kernel3x3(3, 3, -1.0f);
		high_kernel3x3(origin3x3) = 9;

		conv = convolutie(grayImg, high_kernel3x3);
		auto trece_sus = normalizare(false, conv, high_kernel3x3);

		imshow("original", grayImg);
		imshow("laplace orig", trece_sus_laplace_orig);
		imshow("laplace filtrat", trece_sus_laplace_filtrat);
		imshow("high pass", trece_sus);
		waitKey();
	}
}

void addLab09ToMenu(std::vector<MenuItem>& menuList) {
	menuList.push_back(createMenuItem("trece jos", trece_jos));
	menuList.push_back(createMenuItem("trece sus", trece_sus));
}