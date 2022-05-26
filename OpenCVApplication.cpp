// OpenCVApplication.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//#include "common.h"
#include "utils.h"
#include <stack>


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
const int cube_frame_width = down_right.x - up_left.x + 1;
const int cube_frame_height = down_right.y - up_left.y + 1;

const int scan_offset = cvRound(25.0 / 100 * contour_division);
const Point scan_up_left_offset(scan_offset, scan_offset);
const Point scan_down_right_offset(contour_division - scan_offset, contour_division - scan_offset);

vector<pair<Vec3b, pair<Vec3b, Vec3b>>> colors = {
	{ { 255, 255, 255 }, { { 0, 0, 80 }, { 180, 40, 255 } } },	// WHITE
	{ { 255, 255, 255 }, { { 160, 195, 80 }, { 209, 255, 255 } } },	// WHITE
	{ { 255, 255, 255 }, { { 0, 0, 200 }, { 180, 50, 255 } } },	// WHITE
	{ { 0, 255, 255 }, { { 17, 19, 39 }, {40, 255, 255} } },	// YELLOW
	//{ { 0, 140, 255 }, { { 10, 20, 0 }, {30, 70, 255} } },		// ORANGE
	{ { 0, 140, 255 }, { { 2, 27, 75 }, {12, 255, 255} } },		// ORANGE
	{ { 0, 140, 255 }, { { 3, 90, 200 }, {8, 150, 255} } },		// ORANGE
	{ { 255, 0, 0 }, { { 90, 70, 14 }, {125, 255, 255} } },		// BLUE
	{ { 0, 255, 0 }, { { 41, 15, 1 }, {95, 255, 255} } },		// GREEN
	{ { 0, 0, 255 }, { { 0	, 43, 35 }, {5, 255, 255} } },		// RED1
	{ { 0, 0, 255 }, { { 129, 76, 35 }, {180, 255, 255} } },	// RED2
};

#define CLR_WHITE 0
#define CLR_YELLOW 1
#define CLR_ORANGE 2
#define CLR_BLUE 3
#define CLR_GREEN 4
#define CLR_RED 5
vector<pair<Vec3b, Vec3b>> calibrated_colors = {
	{ { 255, 255, 255 }, { 0, 0, 0 } },	// WHITE
	{ { 0, 255, 255 }, { 0, 0, 0 } },	// YELLOW
	{ { 0, 140, 255 }, { 0, 0, 0 } },		// ORANGE
	{ { 255, 0, 0 }, { 0, 0, 0 } },		// BLUE
	{ { 0, 255, 0 }, { 0, 0, 0 } },		// GREEN
	{ { 0, 0, 255 }, { 0, 0, 0 } },		// RED
};

Mat_<Vec3b> drawCubeFrame(Mat& img, const Vec3b& color = Vec3b(50, 205, 50), const int& thickness = 2, const bool& showScanArea = false) {
	Mat_<Vec3b> res = img.clone();
	rectangle(res, up_left, down_right, color, thickness);

	for (int i = 1; i < 3; i++) {
		line(res, { up_left.x + i * contour_division, up_left.y }, { up_left.x + i * contour_division, down_right.y }, color, thickness);
		line(res, { up_left.x, up_left.y + i * contour_division }, { down_right.x, up_left.y + i * contour_division }, color, thickness);
	}

	if (!showScanArea) return res;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			auto p = up_left + contour_division * Point(j, i);
			auto up_left = p + scan_up_left_offset;
			auto down_right = p + scan_down_right_offset;
			rectangle(res, up_left, down_right, { 255, 255, 255 }, thickness);
		}
	}

	return res;
}

void fillRectangle(Mat_<Vec3b>& img, Point up_left, Point down_right, const Vec3b& color, const int& thickness = 2) {
	Point offset{ 1, 1 };
	while (up_left != down_right) {
		rectangle(img, up_left, down_right, color, thickness);
		up_left += offset;
		down_right -= offset;
	}
}

float hsv_distance(Vec3b a, Vec3b b) {
	auto dh = min(abs(a[0] - b[0]), 360 - abs(a[0] - b[0])) / 180.0;
	auto ds = abs(a[1] - b[1]);
	auto dv = abs(a[2] - b[2]) / 255.0;

	return sqrt(pow(dh, 2) + pow(ds, 2) + pow(dv, 2));
}

Mat_<Vec3b> drawOutput(const Mat_<Vec3b>& img, const vector<pair<Point, Point>>& seekZones) {
	Mat_<Vec3b> output(HEIGHT, WIDTH, Vec3b(127, 127, 127));
	Mat_<Vec3b> hsv;

	cvtColor(img, hsv, COLOR_BGR2HSV);

	float dist[3][3];
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			dist[i][j] = 1000;
		}
	}

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			auto& zone = seekZones[i * 3 + j];
			auto len = min(abs(zone.second.x - zone.first.x), abs(zone.second.y - zone.second.y));
			//auto scan_offset = cvRound(80.0 / 100 * len);
			auto scan_offset = 10;
			auto scan_up_left = zone.first + Point(scan_offset, scan_offset);
			auto scan_down_right = zone.second - Point(scan_offset, scan_offset);

			float h, s, v; h = s = v = 0;
			int cnt = 1;
			for (int k = scan_up_left.y; k < scan_down_right.y; k++) {
				for (int l = scan_up_left.x; l < scan_down_right.x; l++) {
					if (!isInside(hsv, Point(l, k))) continue;
					h += hsv(k, l)[0];
					s += hsv(k, l)[1];
					v += hsv(k, l)[2];
					cnt++;
				}
			}

			h /= cnt; s /= cnt; v /= cnt;
			Vec3b average_hsv(h, s, v);
			for (auto& color : calibrated_colors) {
				//float h, s, v; h = s = v = 0;
				//h += (color.second.first[0] + color.second.second[0]) / 2;
				//s += (color.second.first[1] + color.second.second[1]) / 2;
				//v += (color.second.first[2] + color.second.second[2]) / 2;

				//Vec3b m_hsv(h, s, v);

				auto d = hsv_distance(average_hsv, color.second);
				if (d < dist[i][j]) {
					dist[i][j] = d;
					auto p = up_left + contour_division * Point(j, i);
					auto up_left = p + scan_up_left_offset;
					auto down_right = p + scan_down_right_offset;
					fillRectangle(output, up_left, down_right, color.first);
				}
			}
		}
	}

	//for (auto& color : colors) {
	//	auto& finalColor = color.first;
	//	auto& lower = color.second.first;
	//	auto& upper = color.second.second;

	//	Mat_<uchar> mask;
	//	inRange(hsv, lower, upper, mask);
	//	const Mat open_kernel = getStructuringElement(MORPH_RECT, { 7, 7 });
	//	const Mat close_kernel = getStructuringElement(MORPH_RECT, { 5, 5 });
	//	morphologyEx(mask, mask, MORPH_CLOSE, close_kernel);
	//	morphologyEx(mask, mask, MORPH_OPEN, open_kernel);

	//	
	//}

	return output;
}

void calibrateColor(const Mat_<Vec3b>& img, const vector<pair<Point, Point>>& seekZones, const int colorId) {
	Mat_<Vec3b> hsv;
	cvtColor(img, hsv, COLOR_BGR2HSV);

	float h, s, v; h = s = v = 0;
	int cnt = 1;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			auto& zone = seekZones[i * 3 + j];
			auto len = min(abs(zone.second.x - zone.first.x), abs(zone.second.y - zone.second.y));
			auto scan_offset = 10;
			auto scan_up_left = zone.first + Point(scan_offset, scan_offset);
			auto scan_down_right = zone.second - Point(scan_offset, scan_offset);
			
			for (int k = scan_up_left.y; k < scan_down_right.y; k++) {
				for (int l = scan_up_left.x; l < scan_down_right.x; l++) {
					if (!isInside(hsv, Point(l, k))) continue;
					h += hsv(k, l)[0];
					s += hsv(k, l)[1];
					v += hsv(k, l)[2];
					cnt++;
				}
			}
		}
	}
	h /= cnt; s /= cnt; v /= cnt;
	Vec3b average_hsv(h, s, v);

	calibrated_colors[colorId].second = average_hsv;
}

void get_hist(const Mat_<uchar>& img, int* hist) {
	memset(hist, 0, 256 * sizeof(int));
	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			hist[img(i, j)]++;
		}
	}
}

Mat_<uchar> egalizare_hist(const Mat_<uchar>& img) {
	Mat_<uchar> res(img.rows, img.cols);
	int hist[256], histc[256], g[256];

	memset(hist, 0, sizeof(hist));
	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			hist[img(i, j)]++;
		}
	}

	histc[0] = hist[0];
	for (int i = 1; i < 256; i++) {
		histc[i] = histc[i - 1] + hist[i];
	}

	for (int i = 0; i < 256; i++) {
		g[i] = 255 * (1.0 * histc[i] / histc[255]);
	}

	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			res(i, j) = g[img(i, j)];
		}
	}

	return res;
}

Mat_<Vec3b> copyInterestZone(const Mat_<Vec3b>& img) {
	Mat_<Vec3b> res(down_right.y - up_left.y + 1, down_right.x - up_left.x + 1);

	for (int i = up_left.y; i <= down_right.y; i++) {
		for (int j = up_left.x; j <= down_right.x; j++) {
			Point p(j, i);
			res(p - up_left) = img(p);
		}
	}

	return res;
}

Mat_<uchar> gamma_correct(const Mat_<uchar>& img, float gamma) {
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

vector<Vec2f> filterLines(vector<Vec2f> lines) {
	vector<Vec2f> strong_lines;

	float rho_threshold = 40;
	float theta_threshold = 14 * CV_PI / 360;

	for (auto& line : lines) {
		if (line[0] < 0) {
			line[0] *= -1;
			line[1] -= CV_PI;
		}
	}

	strong_lines.push_back(lines[0]);
	for (int i = 1; i < lines.size(); i++) {
		auto& line = lines[i];
		auto rho = line[0];
		auto theta = line[1];
		
		bool rhoClose = false;
		bool thetaClose = false;
		for (auto& strong_line : strong_lines) {
			if (abs(rho - strong_line[0]) < rho_threshold) {
				rhoClose = true;
			}
			if (abs(theta - strong_line[1]) < theta_threshold) {
				thetaClose = true;
			}
		}
		if (rhoClose && thetaClose) continue;
		strong_lines.push_back(line);
		if (strong_lines.size() >= 4) break;
	}

	return strong_lines;
}

Mat_<Vec3b> drawLines(const Mat_<Vec3b>& img, vector<Vec2f> lines) {
	Mat_<Vec3b> res = img.clone();

	printf("\n==========\n");
	for (auto& curr_line : lines) {
		auto rho = curr_line[0];
		auto theta = curr_line[1];

		printf("%f %f\n", rho, theta);

		auto a = cos(theta);
		auto b = sin(theta);
		auto x0 = a * rho;
		auto y0 = b * rho;
		int x1 = x0 + 1000 * (-b);
		int y1 = y0 + 1000 * a;
		int x2 = x0 - 1000 * (-b);
		int y2 = y0 - 1000 * a;
		line(res, {x1, y1}, {x2, y2}, Vec3b(255, 255, 0));
	}

	return res;
}

vector<Vec4i> filterLines(vector<Vec4i> lines) {
	vector<pair<float, pair<Point, Point>>> augmented_lines;
	vector<pair<float, pair<Point, Point>>> strong_augmented_lines;
	vector<Vec4i> strong_lines;

	if (lines.size() <= 0) return strong_lines;
	

	float angle_threshold = 80;
	float distance_threshold = 45;

	for (auto& line : lines) {
		auto& p1 = Point(line[0], line[1]);
		auto& p2 = Point(line[2], line[3]);
		float angle = atan2(p1.y - p2.y, p1.x - p2.x) * 180 / CV_PI;
		if (angle < 0) {
			angle += 360;
		}
		augmented_lines.push_back({ angle, { p1, p2 } });
	}

	strong_lines.push_back(lines[0]);
	strong_augmented_lines.push_back(augmented_lines[0]);
	for (int i = 1; i < lines.size(); i++) {
		auto& angle = augmented_lines[i].first;
		auto& p1 = augmented_lines[i].second.first;
		auto& p2 = augmented_lines[i].second.second;
		/*auto ac = p2c.y - p1c.y;
		auto bc = p2c.x - p1c.x;
		auto cc = p1c.x * p2c.y - p2c.x * p1c.y;*/
		//Len = Sqrt((x2 - x1) ^ 2 + (y2 - y1) ^ 2)
		//Rho = (x1 * y2 - x2 * y1) / Len
		auto len = sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
		auto rho = (p1.x * p2.y - p2.x * p1.y) / len;
		//if (rho < 0) rho *= -1;

		

		bool angle_close = false;
		bool dist_close = false;
		for (auto& strong_augmented_line : strong_augmented_lines) {
			if (abs(angle - strong_augmented_lines[0].first) < angle_threshold) {
				angle_close = true;
			}
			auto& p1 = strong_augmented_line.second.first;
			auto& p2 = strong_augmented_line.second.second;
			//auto a = p2.y - p1.y;
			//auto b = p2.x - p1.x;
			//auto c = p1.x * p2.y - p2.x * p1.y;
			//if (ac == a && bc == b) {
			//	auto d = abs(c - cc) / sqrt(a * a + b * b);
			//	if (d <= distance_threshold) dist_close = true;
			//} else {
			//	dist_close = true;
			//}
			auto len = sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
			auto rho_curr = (p1.x * p2.y - p2.x * p1.y) / len;
			//if (rho_curr < 0) rho_curr *= -1;

			if (abs(rho_curr - rho) < distance_threshold) dist_close = true;
		}
		if (angle_close) continue;
		strong_lines.push_back(lines[i]);
		strong_augmented_lines.push_back(augmented_lines[i]);
		//if (strong_lines.size() >= 4) break;
	}

	return strong_lines;
}

Mat_<Vec3b> drawLines(const Mat_<Vec3b>& img, vector<Vec4i> lines) {
	Mat_<Vec3b> res = img.clone();

	//printf("\n==========\n");
	for (size_t i = 0; i < lines.size(); i++) {
		Vec4i l = lines[i];
		// draw the lines

		Point p1, p2;
		p1 = Point(l[0], l[1]);
		p2 = Point(l[2], l[3]);
		line(res, p1, p2, Vec3b(255, 255, 0));
	}

	return res;
}

Mat_<uchar> fillBackground(const Mat_<uchar>& img) {
	Mat_<uchar> res = img.clone();
	vector<vector<Point> > imageBorders;
	imageBorders.push_back({ Point(0, 0), Point(res.cols, 0) });
	imageBorders.push_back({ Point(res.cols, 0), Point(res.cols, res.rows) });
	imageBorders.push_back({ Point(res.cols, res.rows), Point(0, res.rows) });
	imageBorders.push_back({ Point(0, res.rows), Point(0, 0) });

	int borderThickness = 20;

	for (auto& border : imageBorders) {
		Point startPoint = border[0];
		Point endPoint	 = border[1];
		line(res, startPoint, endPoint, Vec3b(255, 255, 255), borderThickness);
	}

	int fillOffsetX = borderThickness;
	int fillOffsetY = borderThickness;
	Scalar fillTolerance = 0;
	int fillColor = 255;

	int targetCols = res.cols;
	int targetRows = res.rows;

	floodFill(res, Point(fillOffsetX, fillOffsetY), fillColor, (Rect*)0, fillTolerance, fillTolerance);
	floodFill(res, Point(fillOffsetX, targetRows - fillOffsetY), fillColor, (Rect*)0, fillTolerance, fillTolerance);
	floodFill(res, Point(targetCols - fillOffsetX, fillOffsetY), fillColor, (Rect*)0, fillTolerance, fillTolerance);
	floodFill(res, Point(targetCols - fillOffsetX, targetRows - fillOffsetY), fillColor, (Rect*)0, fillTolerance, fillTolerance);

	return res;
}

Point calcCenterOfMass(const Mat_<uchar>& img, const uchar& filter) {
	int sumR = 0;
	int sumC = 0;
	int area = 1;

	for (int r = 0; r < img.rows; r++) {
		for (int c = 0; c < img.cols; c++) {
			if (img(r, c) != filter) continue;
			sumR += r;
			sumC += c;
			area++;
		}
	}

	return Point(sumC / area, sumR / area);
}

Mat_<uchar> getShapeMask(const Mat_ <uchar>& img, const Point& start) {
	Mat_<uchar> mask(img.rows, img.cols, (uchar)0);

	vector<Point> neighb_offset = { {1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, {1, 1} };
	stack<Point> s;
	s.push(start);

	while (!s.empty()) {
		auto& p = s.top(); s.pop();
		mask(p) = 255;

		for (auto& offset : neighb_offset) {
			auto target = p + offset;
			if (!isInside(img, target)) continue;
			if (mask(target) == 255) continue;
			if (img(target) != img(start)) continue;
			s.push(target);
		}
	}

	return mask;
}

Mat rotate(const Mat& img, float angle) {
	Mat res = img.clone();
	// get the center coordinates of the image to create the 2D rotation matrix
	Point center(res.cols / 2.0, res.rows / 2.0);
	// using getRotationMatrix2D() to get the rotation matrix
	Mat rotation_matix = getRotationMatrix2D(center, angle, 1.0);

	// rotate the image using warpAffine
	warpAffine(res, res, rotation_matix, res.size());
	
	return res;
}

vector<pair<Point, Point>> getRectangles(const Point& mid_up_left, const Point& mid_down_right) {
	vector<pair<Point, Point>> rectangles;

	auto len = mid_down_right.x - mid_up_left.x;
	auto center = (mid_up_left + mid_down_right) / 2;
	auto up_left_offset = center - mid_up_left;
	auto down_right_offset = mid_down_right - center;
	
	auto border_offset = 3;

	// UP LEFT
	auto new_center = center - 2 * up_left_offset - Point(border_offset, border_offset);
	auto new_up_left = new_center - up_left_offset;
	auto new_down_right = new_center + down_right_offset;
	rectangles.push_back({ new_up_left, new_down_right });

	// UP MIDDLE
	new_center = center - 2 * Point(0, up_left_offset.y) - Point(0, border_offset);
	new_up_left = new_center - up_left_offset;
	new_down_right = new_center + down_right_offset;
	rectangles.push_back({ new_up_left, new_down_right });

	// UP RIGHT
	new_center = center - 2 * Point(-up_left_offset.x, up_left_offset.y) - Point(-border_offset, border_offset);
	new_up_left = new_center - up_left_offset;
	new_down_right = new_center + down_right_offset;
	rectangles.push_back({ new_up_left, new_down_right });

	// MID LEFT
	new_center = center - 2 * Point(up_left_offset.x, 0) - Point(border_offset, 0);
	new_up_left = new_center - up_left_offset;
	new_down_right = new_center + down_right_offset;
	rectangles.push_back({ new_up_left, new_down_right });

	// MID MID
	rectangles.push_back({ mid_up_left, mid_down_right });

	// MID RIGHT
	new_center = center - 2 * Point(-up_left_offset.x, 0) - Point(-border_offset, 0);
	new_up_left = new_center - up_left_offset;
	new_down_right = new_center + down_right_offset;
	rectangles.push_back({ new_up_left, new_down_right });

	// DOWN LEFT
	new_center = center - 2 * Point(up_left_offset.x, -up_left_offset.y) - Point(border_offset, -border_offset);
	new_up_left = new_center - up_left_offset;
	new_down_right = new_center + down_right_offset;
	rectangles.push_back({ new_up_left, new_down_right });

	// DOWN MIDDLE
	new_center = center - 2 * Point(0, -up_left_offset.y) - Point(0, -border_offset);
	new_up_left = new_center - up_left_offset;
	new_down_right = new_center + down_right_offset;
	rectangles.push_back({ new_up_left, new_down_right });

	// DOWN RIGHT
	new_center = center - 2 * Point(-up_left_offset.x, -up_left_offset.y) - Point(-border_offset, -border_offset);
	new_up_left = new_center - up_left_offset;
	new_down_right = new_center + down_right_offset;
	rectangles.push_back({ new_up_left, new_down_right });

	//auto offset = center - mid_up_left;
	//auto new_up_left = mid_up_left - 2 * offset;
	//rectangles.push_back({ new_up_left, mid_up_left - Point(10, 10) });

	//new_up_left = mid_up_left - Point(0, len);
	//rectangles.push_back({ new_up_left, Point(mid_down_right.x, mid_up_left.y) - Point(0, 7) });

	//new_up_left = Point(mid_down_right.x, mid_up_left.y) - Point(-10, len);
	//rectangles.push_back({ new_up_left, Point(mid_down_right.x + len, mid_up_left.y) + Point(5, -10) });

	return rectangles;
}

enum Showage { verbose, important, none };
Showage showType = verbose;

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

	while (true) {
		Mat_<Vec3b> frame, rotatedImg;
		cap >> frame;
		if (frame.empty()) {
			printf("End of video.\n");
			break;
		}

		flip(frame, frame, 1);
		if(showType <= important) imshow("input", drawCubeFrame(frame, { 50, 205, 50 }, 2, false));

		frame = copyInterestZone(frame);
		rotatedImg = frame.clone();
		fastNlMeansDenoising(frame, frame, 5);
		//fastNlMeansDenoising(frame, frame, 5);
		frame = gamma_correct(frame, 1.175);
		if (showType <= verbose) imshow("zona interes", frame);

		Mat_<uchar> grayImg;
		cvtColor(frame, grayImg, COLOR_BGR2GRAY);
		if (showType <= verbose) imshow("gri", grayImg);

		grayImg = egalizare_hist(grayImg);
		medianBlur(grayImg, grayImg, 5);
		if (showType <= verbose) imshow("egalizat1", grayImg);

		grayImg = egalizare_hist(grayImg);
		medianBlur(grayImg, grayImg, 5);
		if (showType <= verbose) imshow("egalizat2", grayImg);

		Mat topHat, blackHat;
		auto kernel = getStructuringElement(MORPH_ELLIPSE, Size(11, 11));
		morphologyEx(grayImg, topHat, MORPH_TOPHAT, kernel);
		morphologyEx(grayImg, blackHat, MORPH_BLACKHAT, kernel);
		grayImg = grayImg + topHat - blackHat;
		if (showType <= verbose) imshow("contrast++", grayImg);

		threshold(grayImg, grayImg, 20, 255, THRESH_BINARY);
		if (showType <= verbose) imshow("binarizat", grayImg);


		auto erode_kernel = getStructuringElement(MORPH_RECT, { 5, 5 });
		auto dilate_kernel = getStructuringElement(MORPH_RECT, { 7, 7 });
		morphologyEx(grayImg, grayImg, MORPH_ERODE, erode_kernel, { -1, -1 }, 1);
		morphologyEx(grayImg, grayImg, MORPH_DILATE, dilate_kernel, { -1, -1 }, 1);
		grayImg = fillBackground(grayImg);
		grayImg = 255 - grayImg;
		if (showType <= important) imshow("curatat", grayImg);

		Mat edges = grayImg;
		/*Canny(grayImg, edges, 75, 255);
		imshow("edges", edges);*/

		Mat_<Vec3b> drawEdges(grayImg.rows, grayImg.cols, Vec3b(0, 0, 0));

		vector<Vec4i> lines;
		// detect the lines
		//printf("================\n");
		float min_angle = 360;
		float min_length = frame.rows;
		HoughLinesP(edges, lines, 1, CV_PI / 180, 80, 20, 50);
		for (size_t i = 0; i < lines.size(); i++) {
			Vec4i l = lines[i];
			Point p1, p2;
			p1 = Point(l[0], l[1]);
			p2 = Point(l[2], l[3]);
			//calculate angle in radian,  if you need it in degrees just do angle * 180 / PI
			float angle = atan2(p1.y - p2.y, p1.x - p2.x) * 180 / CV_PI;
			if (angle < 0) angle += 360;
			min_angle = min(min_angle, angle);

			auto len = sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
			min_length = min(min_length, len);
		}
		//lines = filterLines(lines);
		drawEdges = drawLines(drawEdges, lines);
		
		//printf("%f\n", min_angle);

		auto center = calcCenterOfMass(grayImg, 255);
		//printf("%d, %d\n", center.x, center.y);
		auto& cube_mask = getShapeMask(grayImg, center);

		drawEdges(center) = Vec3b(0, 0, 255);
		if (showType <= important) imshow("linii", drawEdges);

		if (min_angle > 90) {
			float closest = 90;
			for (int a = 180; a <= 360; a += 90) {
				if (abs(a - min_angle) < abs(min_angle - closest)) closest = a;
			}
			rotatedImg = rotate(rotatedImg, min_angle - closest);
			cube_mask = rotate(cube_mask, min_angle - closest);
		}
		if (showType <= verbose) imshow("rotated", rotatedImg);
		//imshow("mask", cube_mask);

		int minx, miny; minx = miny = cube_mask.rows;
		int maxx, maxy; maxx = maxy = 0;
		for (int i = 0; i < cube_mask.rows; i++) {
			for (int j = 0; j < cube_mask.cols; j++) {
				if (cube_mask(i, j) == 0) continue;
				minx = min(minx, j);
				miny = min(miny, i);
				maxx = max(maxx, j);
				maxy = max(maxy, i);
			}
		}
		auto rectangles = getRectangles({ minx, miny }, { maxx, maxy });
		for (auto& rect : rectangles) {
			rectangle(rotatedImg, rect.first, rect.second, Vec3b(255, 255, 0));
		}

		imshow("output", drawOutput(rotatedImg, rectangles));

		Mat copy = rotatedImg.clone();

		if (showType == verbose) {
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					auto& zone = rectangles[i * 3 + j];
					auto len = min(abs(zone.second.x - zone.first.x), abs(zone.second.y - zone.second.y));
					//auto scan_offset = cvRound(80.0 / 100 * len);
					auto scan_offset = 10;
					auto scan_up_left_offset = zone.first + Point(scan_offset, scan_offset);
					auto scan_down_right_offset = zone.second - Point(scan_offset, scan_offset);

					line(rotatedImg, scan_up_left_offset, { scan_down_right_offset.x, scan_up_left_offset.y }, Vec3b(255, 255, 255));
					line(rotatedImg, scan_up_left_offset, { scan_up_left_offset.x, scan_down_right_offset.y }, Vec3b(255, 255, 255));
					line(rotatedImg, { scan_up_left_offset.x, scan_down_right_offset.y }, scan_down_right_offset, Vec3b(255, 255, 255));
					line(rotatedImg, { scan_down_right_offset.x, scan_up_left_offset.y }, scan_down_right_offset, Vec3b(255, 255, 255));
				}
			}
		}

		if (showType <= important) imshow("cube", rotatedImg);

		

		// detect lines
		//Mat_<Vec3b> test(grayImg.rows, grayImg.cols, Vec3b(0, 0, 0));
		//vector<Vec2f> lines;
		//HoughLines(edges, lines, 1, CV_PI / 180, 80, 50, 100);
		//test = drawLines(test, lines);
		//lines = filterLines(lines);
		//drawEdges = drawLines(drawEdges, lines);
		//imshow("linii", drawEdges);
		//if (showType <= verbose) imshow("liniitot", test);

		auto c = waitKey(1);
		if (c == 27) {
			printf("Exiting...\n");
			break;
		}
		else if (c == 'w') {
			calibrateColor(copy, rectangles, CLR_WHITE);
		} else if (c == 'y') {
			calibrateColor(copy, rectangles, CLR_YELLOW);
		} else if (c == 'r') {
			calibrateColor(copy, rectangles, CLR_RED);
		} else if (c == 'b') {
			calibrateColor(copy, rectangles, CLR_BLUE);
		} else if (c == 'o') {
			calibrateColor(copy, rectangles, CLR_ORANGE);
		} else if (c == 'g') {
			calibrateColor(copy, rectangles, CLR_GREEN);
		} 
	}

	printf("Hello World!");
	return 0;
}