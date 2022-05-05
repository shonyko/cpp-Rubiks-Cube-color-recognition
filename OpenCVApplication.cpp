// OpenCVApplication.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//#include "common.h"
#include "utils.h"


using namespace std;
using namespace cv;

//320x240
#define WIDTH 640
#define HEIGHT 480

#define FRAME_DIVISION_FACTOR 2.5
const int contour_len = cvRound(min(HEIGHT, WIDTH) / FRAME_DIVISION_FACTOR);
const int contour_offsetX = cvRound((WIDTH - contour_len) / 2.0);
const int contour_offsetY = cvRound((HEIGHT - contour_len) / 2.0);
const int contour_division = cvRound(contour_len / 3.0);

const Point up_left(contour_offsetX, contour_offsetY);
const Point down_right(contour_offsetX + contour_len, contour_offsetY + contour_len);

const int scan_offset = cvRound(25.0 / 100 * contour_division);
const Point scan_up_left_offset(scan_offset, scan_offset);
const Point scan_down_right_offset(contour_division - scan_offset, contour_division - scan_offset);

const Mat open_kernel = getStructuringElement(MORPH_RECT, { 7, 7 });
const Mat close_kernel = getStructuringElement(MORPH_RECT, { 5, 5 });

vector<pair<Vec3b, pair<Vec3b, Vec3b>>> colors = {
	{ { 255, 255, 255 }, { { 0, 0, 80 }, { 180, 40, 255 } } },	// WHITE
	{ { 0, 255, 255 }, { { 17, 19, 39 }, {40, 255, 255} } },	// YELLOW
	{ { 0, 140, 255 }, { { 10, 20, 0 }, {30, 70, 255} } },		// ORANGE
	{ { 0, 0, 255 }, { { 0, 16, 35 }, {10, 255, 255} } },		// RED1
	{ { 0, 0, 255 }, { { 129, 43, 35 }, {180, 255, 255} } },	// RED2
	{ { 255, 0, 0 }, { { 90, 70, 20 }, {125, 255, 255} } },		// BLUE
	{ { 0, 255, 0 }, { { 41, 15, 5 }, {95, 255, 255} } },		// GREEN
};

void drawCubeFrame(Mat& img, const Vec3b& color = Vec3b(50, 205, 50), const int& thickness = 2, const bool& showScanArea = false) {
	rectangle(img, up_left, down_right, color, thickness);

	for (int i = 1; i < 3; i++) {
		line(img, { up_left.x + i * contour_division, up_left.y }, { up_left.x + i * contour_division, down_right.y }, color, thickness);
		line(img, { up_left.x, up_left.y + i * contour_division }, { down_right.x, up_left.y + i * contour_division }, color, thickness);
	}

	if (!showScanArea) return;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			auto p = up_left + contour_division * Point(j, i);
			auto up_left = p + scan_up_left_offset;
			auto down_right = p + scan_down_right_offset;
			rectangle(img, up_left, down_right, { 255, 255, 255 }, thickness);
		}
	}
}

void calcHistogram(int hist[][256], const Mat_<Vec3b>& img) {
	for (int i = 0; i < 256; i++) {
		for (int c = 0; c < 3; c++) {
			hist[c][i] = 0;
		}
	}
	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			for (int c = 0; c < 3; c++) {
				hist[c][img(i, j)[c]]++;
			}
		}
	}
}

void calcFDP(float fdp[][256], const Mat_<Vec3b>& img) {
	int hist[3][256];
	calcHistogram(hist, img);
	int M = img.rows * img.cols;
	for (int i = 0; i < 256; i++) {
		for (int c = 0; c < 3; c++) {
			fdp[c][i] = 1.0 * hist[c][i] / M;
		}
	}
}

void calcFDPC(float fdpc[][256], float fdp[][256]) {
	for (int i = 0; i < 256; i++) {
		for (int c = 0; c < 3; c++) {
			fdpc[c][i] = 0;
			for (int j = 0; j <= i; j++) {
				fdpc[c][i] += fdp[c][j];
			}
		}
	}
}

Mat BGR_eq(const Mat_<Vec3b>& img) {
	Mat_<Vec3b> result(img.rows, img.cols);
	float fdp[3][256];
	calcFDP(fdp, img);
	float fdpc[3][256];
	calcFDPC(fdpc, fdp);

	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			Point p(j, i);
			for (int c = 0; c < 3; c++) {
				result(p)[c] = 255 * fdpc[c][img(p)[c]];
			}
		}
	}

	return result;
}

Mat HSV_eq(const Mat_<Vec3b>& img) {
	Mat_<Vec3b> result(img.rows, img.cols);
	float fdp[3][256];
	calcFDP(fdp, img);
	float fdpc[3][256];
	calcFDPC(fdpc, fdp);

	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			Point p(j, i);
			for (int c = 0; c < 3; c++) {
				//result(p)[c] = 255 * fdpc[c][img(p)[c]];
				if (c == 0)
					result(p)[c] = 255 * fdpc[c][img(p)[c]];
				else
					result(p)[c] = img(p)[c];
			}
		}
	}

	return result;
}

int checkFileExists(char* fname) {
	struct stat buffer;
	return (stat(fname, &buffer) == 0);
}

void saveData(char* fname, int hmin = 255, int smin = 255, int vmin = 255, int hmax = 0, int smax = 0, int vmax = 0, double avgh = 0, double avgs = 0, double avgv = 0) {
	char saveName[MAX_PATH];
	for (int i = 0; ; i++) {
		sprintf(saveName, "%s(%d).txt", fname, i);
		if (!checkFileExists(saveName)) break;
	}

	printf("%s\n", saveName);
	FILE* f = fopen(saveName, "w");
	fprintf(f, "hmin = %*d hmax = %*d avgh = %f\n", 3, hmin, 3, hmax, avgh);
	fprintf(f, "smin = %*d smax = %*d avgs = %f\n", 3, smin, 3, smax, avgs);
	fprintf(f, "vmin = %*d vmax = %*d avgv = %f\n", 3, vmin, 3, vmax, avgv);
	fclose(f);
}

void fillRectangle(Mat_<Vec3b>& img, Point up_left, Point down_right, const Vec3b& color, const int& thickness = 2) {
	Point offset{ 1, 1 };
	while (up_left != down_right) {
		rectangle(img, up_left, down_right, color, thickness);
		up_left += offset;
		down_right -= offset;
	}
}

int main() {
	VideoCapture cap(0);
	if (!cap.isOpened()) {
		printf("Cannot open camera");
		return -1;
	}

	cap.set(CAP_PROP_FRAME_WIDTH, WIDTH);
	cap.set(CAP_PROP_FRAME_HEIGHT, HEIGHT);

	auto resolution = Size((int)cap.get(CAP_PROP_FRAME_WIDTH), 
		(int)cap.get(CAP_PROP_FRAME_HEIGHT));

	//const char* WIN_WEBCAM = "Webcam";
	//namedWindow(WIN_WEBCAM, WINDOW_AUTOSIZE);
	//moveWindow(WIN_WEBCAM, 0, 0);

	//const char* WIN_SNAP = "Snap";
	//namedWindow(WIN_SNAP, WINDOW_AUTOSIZE);
	//moveWindow(WIN_SNAP, resolution.width + 10, 0);

	const char* WIN_MARKED_WEBCAM = "Marked Webcam";
	namedWindow(WIN_MARKED_WEBCAM, WINDOW_AUTOSIZE);
	moveWindow(WIN_MARKED_WEBCAM, 0, 0);

	//const char* WIN_HSV = "HSV Eq";
	//namedWindow(WIN_HSV, WINDOW_AUTOSIZE);
	//moveWindow(WIN_HSV, 0, resolution.height + 10);

	//const char* WIN_BGR = "BGR Eq";
	//namedWindow(WIN_BGR, WINDOW_AUTOSIZE);
	//moveWindow(WIN_BGR, resolution.width + 10, resolution.height + 10);

	const char* WIN_OUT = "out";
	namedWindow(WIN_OUT, WINDOW_AUTOSIZE);
	moveWindow(WIN_OUT, resolution.width + 10, 0);

	const char* WIN_MASK = "Mask";
	namedWindow(WIN_MASK, WINDOW_AUTOSIZE);
	moveWindow(WIN_MASK, resolution.width + 10, resolution.height + 10);

	const char* WIN_SCROLLBARS = "scrollbars";
	namedWindow(WIN_SCROLLBARS, WINDOW_AUTOSIZE);
	createTrackbar("HMin", WIN_SCROLLBARS, 0, 179);
	createTrackbar("SMin", WIN_SCROLLBARS, 0, 255);
	createTrackbar("VMin", WIN_SCROLLBARS, 0, 255);
	createTrackbar("HMax", WIN_SCROLLBARS, 0, 179);
	createTrackbar("SMax", WIN_SCROLLBARS, 0, 255);
	createTrackbar("VMax", WIN_SCROLLBARS, 0, 255);

	Mat_<Vec3b> frame, frame_clone, hsv;
	Mat_<Vec3b> output(HEIGHT, WIDTH);
	Mat_<uchar> aux_mask, mask(HEIGHT, WIDTH);
	char c;
	double squares[3][3];
	while (true) {
		cap >> frame;
		if (frame.empty()) {
			printf("End of video.\n");
			break;
		}

		flip(frame, frame, 1);
		//imshow(WIN_WEBCAM, frame);
		frame_clone = frame.clone();
		drawCubeFrame(frame_clone, { 50, 205, 50 }, 2, true);
		imshow(WIN_MARKED_WEBCAM, frame_clone);

		c = waitKey(10);
		// ESC
		if (c == 27) {
			printf("Exiting capture...\n");
			break;
		}
		if (c == 's') {
			//imshow(WIN_SNAP, frame);
			int hmin = 255, smin = 255, vmin = 255;
			int hmax = 0, smax = 0, vmax = 0;
			double avgh = 0, avgs = 0, avgv = 0;
			int cnt = 0;
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					auto p = up_left + contour_division * Point(j, i);
					auto up_left = p + scan_up_left_offset;
					auto down_right = p + scan_down_right_offset;

					for (int k = up_left.y; k < down_right.y; k++) {
						for (int l = up_left.x; l < down_right.x; l++) {
							auto h = frame(k, l)[0];
							auto s = frame(k, l)[1];
							auto v = frame(k, l)[2];

							hmin = min(hmin, h); hmax = max(hmax, h);
							smin = min(smin, s); smax = max(smax, s);
							vmin = min(vmin, v); vmax = max(vmax, v);

							avgh += h;
							avgs += s;
							avgv += v;

							cnt++;
						}
					}
				}
			}
			avgh /= cnt;
			avgs /= cnt;
			avgv /= cnt;

			saveData("D:/Faculta/An3/sem2/PI/lab/OpenCVApplication-VS2019_OCV451_basic/data/orange", hmin, smin, vmin, hmax, smax, vmax, avgh, avgs, avgv);
		}

		cvtColor(frame, hsv, COLOR_BGR2HSV);

		auto whiteL = Vec3b(getTrackbarPos("HMin", "scrollbars"), getTrackbarPos("SMin", "scrollbars"), getTrackbarPos("VMin", "scrollbars"));
		auto whiteU = Vec3b(getTrackbarPos("HMax", "scrollbars"), getTrackbarPos("SMax", "scrollbars"), getTrackbarPos("VMax", "scrollbars"));
		mask.setTo((uchar)0);
		output.setTo(Vec3b{ 127, 127, 127 });
		drawCubeFrame(output, { 0, 0, 0 });

		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				squares[i][j] = 0;
			}
		}

		//colors.push_back({{}})
		for (auto& p : colors) {
			auto color = p.first;
			auto lower = p.second.first;
			auto upper = p.second.second;
			inRange(hsv, lower, upper, aux_mask);
			morphologyEx(aux_mask, aux_mask, MORPH_CLOSE, close_kernel);
			morphologyEx(aux_mask, aux_mask, MORPH_OPEN, open_kernel, { -1, -1 }, 5);

			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					auto p = up_left + contour_division * Point(j, i);
					auto up_left = p + scan_up_left_offset;
					auto down_right = p + scan_down_right_offset;

					int sum = 0;
					int cnt = 0;
					for (int k = up_left.y; k < down_right.y; k++) {
						for (int l = up_left.x; l < down_right.x; l++) {
							sum += aux_mask(k, l) == 255;
							cnt++;
						}
					}
					auto avg = 1.0 * sum / cnt;
					if (cvRound(avg) < 1) continue;
					if (avg < squares[i][j]) continue;

					squares[i][j] = avg;
					fillRectangle(output, up_left, down_right, color);
				}
			}

			mask |= aux_mask;
		}
		imshow(WIN_OUT, output);
		imshow(WIN_MASK, mask);

		//auto bgrImg = BGR_eq(frame);
		//imshow(WIN_BGR, bgrImg);

		//hsv = HSV_eq(hsv);
		//cvtColor(hsv, frame, COLOR_HSV2BGR);
		//imshow(WIN_HSV, frame);
	}

	printf("Hello World!");
	return 0;
}