#include "stdafx.h"
#include "common.h"
#include "lab10.h"
#include "utils.h"

Mat_<float> center(const Mat_ <float>& src) {
	Mat_<float> dst(src.rows, src.cols);

	for (int i = 0; i < src.rows; i++) {
		for (int j = 0; j < src.cols; j++) {
			Point p(j, i);
			dst(p) = (i + j) & 1 ? -src(p) : src(p);
		}
	}

	return dst;
}

float getMax(const Mat_<float>& src) {
	float maxval = src(0, 0);

	for (int i = 0; i < src.rows; i++) {
		for (int j = 0; j < src.cols; j++) {
			maxval = max(maxval, src(i, j));
		}
	}

	return maxval;
}

Mat_<float> multiply(const Mat_<float>& a, const Mat_<float>& b) {
	Mat_<float> res(a.rows, a.cols);

	for (int i = 0; i < a.rows; i++) {
		for (int j = 0; j < a.cols; j++) {
			res(i, j) = a(i, j) * b(i, j);
		}
	}

	return res;
}

Mat_<float> wave_filter(const Mat_<float>& mag, const Mat_<float>& phi, bool sinFlag = true) {
	Mat_<float> res(mag.rows, mag.cols);

	for (int i = 0; i < mag.rows; i++) {
		for (int j = 0; j < mag.cols; j++) {
			res(i, j) = 0;
			res(i, j) += sin(mag(i, j)) * phi(i, j) * sinFlag;
			res(i, j) += cos(mag(i, j)) * phi(i, j) * !sinFlag;
		}
	}

	return res;
}

Mat_<uchar> fourier(const Mat_<uchar>& src, const Mat_<float>& kernel) {
	Mat_<float> srcf;
	src.convertTo(srcf, CV_32FC1);
	auto srcfc = center(srcf);

	Mat fourier;
	dft(srcfc, fourier, DFT_COMPLEX_OUTPUT);

	Mat channels[] = { Mat_<float>::zeros(src.size()), Mat_<float>::zeros(src.size()) };
	split(fourier, channels); // channels[0] = Re(DFT(I)), channels[1] = Im(DFT(I))

	Mat mag, phi;
	magnitude(channels[0], channels[1], mag);
	phase(channels[0], channels[1], phi);


	//aici afișați imaginile cu fazele și magnitudinile
	imshow("magnitudine", mag);
	imshow("faza", phi);


	//aici inserați operații de filtrare aplicate pe coeficienții Fourier
	Mat_<float> magp = multiply(mag, kernel);

	//memorați partea reală în channels[0] și partea imaginară în channels[1]
	channels[0] = wave_filter(magp, phi, false);
	channels[1] = wave_filter(magp, phi);

	Mat dstf, dstfc;
	merge(channels, 2, fourier);
	dft(fourier, dstf, DFT_INVERSE | DFT_REAL_OUTPUT | DFT_SCALE);

	dstfc = center(dstf);

	Mat_<uchar> dst;
	normalize(dstfc, dst, 0, 255, NORM_MINMAX, CV_8UC1);

	return dst;
}

Mat_<uchar> gauss_trece_jos(const Mat_<uchar>& src, float A = 20) {
	Mat_<float> kernel(src.rows, src.cols);

	auto halfH = 1.0 * src.rows / 2;
	auto halfW = 1.0 * src.cols / 2;
	auto A2 = pow(A, 2);

	for (int i = 0; i < src.rows; i++) {
		for (int j = 0; j < src.cols; j++) {
			kernel(i, j) = exp(-(pow(halfH - i, 2) + pow(halfW - j, 2)) / A2);
		}
	}

	return fourier(src, kernel);
}

Mat_<uchar> gauss_trece_sus(const Mat_<uchar>& src, float A = 20) {
	Mat_<float> kernel(src.rows, src.cols);

	auto halfH = 1.0 * src.rows / 2;
	auto halfW = 1.0 * src.cols / 2;
	auto A2 = pow(A, 2);

	for (int i = 0; i < src.rows; i++) {
		for (int j = 0; j < src.cols; j++) {
			kernel(i, j) = 1 - exp(-(pow(halfH - i, 2) + pow(halfW - j, 2)) / A2);
		}
	}

	return fourier(src, kernel);
}

void l10_ex3() {
	char fname[MAX_PATH];
	if (openFileDlg(fname)) {
		Mat_<Vec3b> img = imread(fname, IMREAD_COLOR);
		auto grayImg = RGB2Gray(img);

		//Mat_<float> maglog(mag.rows, mag.cols);
		//for (int i = 0; i < mag.rows; i++) {
		//	for (int j = 0; j < mag.cols; j++) {
		//		maglog(i, j) = log(mag.at<float>(i, j) + 1);
		//	}
		//}
		//float logmax = getMax(maglog);
		//imshow("logaritm", maglog / logmax);

		//float halfH = 1.0 * mag.rows / 2;
		//float halfW = 1.0 * mag.cols / 2;
		//float R = 20;
		//float R2 = pow(R, 2);
		//Mat_<float> magjos(mag.rows, mag.cols);
		//for (int i = 0; i < mag.rows; i++) {
		//	for (int j = 0; j < mag.cols; j++) {
		//		magjos(i, j) = pow(halfH - i, 2) + pow(halfW - j, 2) <= R2;
		//	}
		//}
		//float josmax = getMax(magjos);
		//imshow("jos", magjos / josmax);

		//Mat_<float> magsus(mag.rows, mag.cols);
		//for (int i = 0; i < mag.rows; i++) {
		//	for (int j = 0; j < mag.cols; j++) {
		//		magsus(i, j) = pow(halfH - i, 2) + pow(halfW - j, 2) > R2;
		//	}
		//}
		//float susmax = getMax(magsus);
		//imshow("sus", magsus / susmax);

		//Mat_<float> magjosgauss(mag.rows, mag.cols);
		//for (int i = 0; i < mag.rows; i++) {
		//	for (int j = 0; j < mag.cols; j++) {
		//		magjosgauss(i, j) = log10(1 + mag.at<float>(i, j) * exp(-(pow(halfH - i, 2) + pow(halfW - j, 2)) / R2));
		//		//magjosgauss(i, j) = exp(-(pow(halfH - i, 2) + pow(halfW - j, 2)) / R2);
		//	}
		//}
		//float josgaussmax = getMax(magjosgauss);
		//imshow("jos gauss", magjosgauss / josgaussmax);

		//Mat_<float> magsusgauss(mag.rows, mag.cols);
		//for (int i = 0; i < mag.rows; i++) {
		//	for (int j = 0; j < mag.cols; j++) {
		//		magsusgauss(i, j) = log10(1 + mag.at<float>(i, j) * (1 - exp(-(pow(halfH - i, 2) + pow(halfW - j, 2)) / R2)));
		//	}
		//}
		//float susgaussmax = getMax(magjosgauss);
		//imshow("sus gauss", magsusgauss / susgaussmax);

		auto gauss_jos = gauss_trece_jos(grayImg);
		auto gauss_sus = gauss_trece_sus(grayImg);

		imshow("original", grayImg);
		imshow("gauss jos", gauss_jos);
		imshow("gauss sus", gauss_sus);
		waitKey();
	}
}

void addLab10ToMenu(std::vector<MenuItem>& menuList) {
	//menuList.push_back(createMenuItem("trece jos", trece_jos));
	menuList.push_back(createMenuItem("ex3", l10_ex3));
}