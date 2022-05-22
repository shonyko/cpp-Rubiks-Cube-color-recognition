#include "stdafx.h"
#include "common.h"
#include "lab08.h"
#include "utils.h"

float calc_media(const Mat_<uchar>& img) {
	float avg = 0;

	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			avg += img(i, j);
		}
	}

	auto M = img.rows * img.cols;
	avg /= M;

	return avg;
}

float calc_deviatia(const Mat_<uchar>& img, float media) {
	float avg = 0;

	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			avg += pow(img(i, j) - media, 2);
		}
	}

	auto M = img.rows * img.cols;
	avg /= M;

	return sqrt(avg);
}

void calc_histograma_cumulativa(int* histc, int* hist, int size = 256) {
	for (int i = 0; i < size; i++) {
		histc[i] = 0;
		for (int j = 0; j <= i; j++) {
			histc[i] += hist[j];
		}
	}
}

Mat_<uchar> binarizare_automata(const Mat_<uchar>& img, float err = 0.003) {
	Mat_<uchar> result(img.rows, img.cols, (uchar)0);

	int hist[256];
	calcHistogram(hist, img);

	int imin = minHistIntensity(hist);
	int imax = maxHistIntensity(hist);

	auto T = 1.0 * (imin + imax) / 2;

	int G1, G2;
	int N1, N2;
	G1 = G2 = 0;
	N1 = N2 = 0;

	double lastT;
	do {
		for (int i = imin; i <= T; i++) {
			G1 += i * hist[i];
			N1 += hist[i];
		}
		for (int i = T + 1; i <= imax; i++) {
			G2 += i * hist[i];
			N2 += hist[i];
		}

		auto mg1 = 1.0 * G1 / N1;
		auto mg2 = 1.0 * G2 / N2;

		lastT = T;
		T = 1.0 * (mg1 + mg2) / 2;
	} while (abs(T - lastT) <= err);

	printf("Threshold: %f\n", T);

	return Gray2BlackWhite(img, T);
}

Mat_<uchar> latire_ingustare_histograma(const Mat_<uchar>& img, int g_in_min, int g_in_max, int g_out_min, int g_out_max) {
	Mat_<uchar> result(img.rows, img.cols, (uchar)0);
	
	auto factor = 1.0 * (g_out_max - g_out_min) / (g_in_max - g_in_min);
	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			Point p(j, i);
			result(p) = g_out_min + (img(p) - g_in_min) * factor;
		}
	}

	return result;
}

Mat_<uchar> corectie_gamma(const Mat_<uchar>& img, float gamma) {
	Mat_<uchar> result(img.rows, img.cols, (uchar)0);
	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			Point p(j, i);
			auto fraction = 1.0 * img(p) / 255;
			result(p) = 255 * pow(fraction, gamma);
		}
	}
	return result;
}

Mat_<uchar> modificare_luminozitate(const Mat_<uchar>& img, int offset) {
	Mat_<uchar> result(img.rows, img.cols, (uchar)0);
	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			Point p(j, i);
			auto new_val = img(p) + offset;
			result(p) = clamp(0, new_val, 255);
		}
	}
	return result;
}

Mat_<uchar> egalizare_histograma(const Mat_<uchar>& img) {
	Mat_<uchar> result(img.rows, img.cols, (uchar)0);
	float fdp[256];
	calcFDP(fdp, img);
	float fdpc[256];
	calcFDPC(fdpc, fdp);

	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			Point p(j, i);
			result(p) = 255 * fdpc[img(p)];
		}
	}

	return result;
}

void l8_ex1() {
	char fname[MAX_PATH];
	if (openFileDlg(fname)) {
		Mat_<Vec3b> img = imread(fname, IMREAD_COLOR);
		auto grayImg = RGB2Gray(img);

		auto media = calc_media(grayImg);
		printf("Media: %f\n", media);

		auto deviatia = calc_deviatia(grayImg, media);
		printf("Deviatia: %f\n", deviatia);

		int hist[256];
		calcHistogram(hist, grayImg);
		showHistogram("hist", hist, 256, 256);


		int histc[256];
		calc_histograma_cumulativa(histc, hist);
		showHistogram("histc", histc, 256, 256);
		
		waitKey();
	}
}

void l8_ex2() {
	char fname[MAX_PATH];
	if (openFileDlg(fname)) {
		Mat_<Vec3b> img = imread(fname, IMREAD_COLOR);
		auto grayImg = RGB2Gray(img);

		auto bwImg = binarizare_automata(grayImg);

		imshow("original", grayImg);
		imshow("binarizat", bwImg);
		waitKey();
	}
}

void l8_ex3() {
	char fname[MAX_PATH];
	if (openFileDlg(fname)) {
		Mat_<Vec3b> img = imread(fname, IMREAD_COLOR);
		auto grayImg = RGB2Gray(img);

		// Negativ
		auto negativeImg = Gray2Negative(grayImg);

		// Latire / ingustare histograma
		int g_in_min = -1, g_in_max;
		int g_out_min, g_out_max;
		int hist[256];
		calcHistogram(hist, grayImg);
		for (int i = 0; i < 256; i++) {
			if (hist[i] == 0) continue;
			if (g_in_min == -1) g_in_min = i;
			g_in_max = i;
		}
		printf("g_out_min: "); scanf("%d", &g_out_min);
		printf("g_out_max: "); scanf("%d", &g_out_max);
		printf("g_in_min: %d\n g_in_max: %d\n", g_in_min, g_in_max);
		printf("g_out_min: %d\n g_out_max: %d\n", g_out_min, g_out_max);
		auto latire_ingustare_img = latire_ingustare_histograma(grayImg, g_in_min, g_in_max, g_out_min, g_out_max);

		// Corectie gamma
		float gamma;
		printf("gamma: "); scanf("%f", &gamma);
		auto gammaImg = corectie_gamma(grayImg, gamma);

		// Modificarea luminozitatii
		int offset;
		printf("offset luminozitate: "); scanf("%d", &offset);
		auto luminozitateImg = modificare_luminozitate(grayImg, offset);

		calcHistogram(hist, grayImg);
		imshow("original", grayImg);
		showHistogram("original hist", hist, 256, 256);

		calcHistogram(hist, negativeImg);
		imshow("negative", negativeImg);
		showHistogram("negative hist", hist, 256, 256);

		calcHistogram(hist, latire_ingustare_img);
		imshow("latire / ingustare", latire_ingustare_img);
		showHistogram("latire / ingustare hist", hist, 256, 256);
		
		calcHistogram(hist, gammaImg);
		imshow("corectie gamma", gammaImg);
		showHistogram("corectie gamma hist", hist, 256, 256);
		
		calcHistogram(hist, luminozitateImg);
		imshow("luminozitate", luminozitateImg);
		showHistogram("luminozitate hist", hist, 256, 256);
		waitKey();
	}
}

void l8_ex4() {
	char fname[MAX_PATH];
	if (openFileDlg(fname)) {
		Mat_<Vec3b> img = imread(fname, IMREAD_COLOR);
		auto grayImg = RGB2Gray(img);

		auto egalizatImg = egalizare_histograma(grayImg);

		int hist[256];
		calcHistogram(hist, grayImg);
		imshow("original", grayImg);
		showHistogram("original hist", hist, 256, 256);

		calcHistogram(hist, egalizatImg);
		imshow("egalizata", egalizatImg);
		showHistogram("egalizata hist", hist, 256, 256);
		waitKey();
	}
}

void addLab08ToMenu(std::vector<MenuItem>& menuList) {
	menuList.push_back(createMenuItem("ex1", l8_ex1));
	menuList.push_back(createMenuItem("ex2", l8_ex2));
	menuList.push_back(createMenuItem("ex3", l8_ex3));
	menuList.push_back(createMenuItem("ex4", l8_ex4));
}