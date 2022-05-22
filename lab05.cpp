#include "stdafx.h"
#include "common.h"
#include "lab05.h"
#include "utils.h"
#include <queue>
#include <vector>
#include <random>
#include <map>

bool isBackground(uchar pixel) {
	return pixel != 0;
}

bool hasLabel(int pixel) {
	return pixel != 0;
}

std::vector<Point> vec4 =	{ {-1, 0}, {0, -1}, {1, 0}, {0, 1} };
std::vector<Point> vec8 =	{ 
								{-1, 0}, {0, -1}, {1, 0}, {0, 1},
								{-1, -1}, {-1, 1}, {1, -1}, {1, 1}
							};

Mat_<Vec3b> colorlabels(const Mat_<int>& labels, const Vec3b& bgColor = Vec3b(255, 255, 255)) {
	Mat_<Vec3b> colorImg(labels.rows, labels.cols);

	std::default_random_engine gen;
	std::uniform_int_distribution<int> d(0, 255);

	std::map<int, Vec3b> colors;
	for (int i = 0; i < labels.rows; i++) {
		for (int j = 0; j < labels.cols; j++) {
			Point p(j, i);
			if (!hasLabel(labels(p))) {
				colorImg(p) = bgColor;
				continue;
			}

			int label = labels(p);
			if (!colors.count(label)) {
				colors.emplace(label, Vec3b(d(gen), d(gen), d(gen)));
			}

			colorImg(p) = colors.at(label);
		}
	}

	return colorImg;
}

Mat_<int> BFS_label(const Mat_<uchar>& img, const std::vector<Point>& vec) {
	Mat_<int> labels(img.rows, img.cols);
	labels.setTo(0);

	int label = 0;
	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			Point p(j, i);
			if (isBackground(img(p))) continue;
			if (hasLabel(labels(p))) continue;

			label++;
			labels(p) = label;
			std::queue<Point> q;
			q.push(p);

			while (!q.empty()) {
				auto point = q.front(); q.pop();
				for (auto& offset : vec) {
					auto neighbor = point + offset;
					if (!isInside(img, neighbor)) continue;
					if (isBackground(img(neighbor))) continue;
					if (hasLabel(labels(neighbor))) continue;

					labels(neighbor) = label;
					q.push(neighbor);
				}
			}
		}
	}

	return labels;
}

Mat_<int> twoway_label(const Mat_<uchar>& img, const std::vector<Point>& vec, const bool partial = false) {
	Mat_<int> labels(img.rows, img.cols);
	labels.setTo(0);

	std::vector<std::vector<int>> edges;
	int label = 0;
	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			Point p(j, i);
			if (isBackground(img(p))) continue;
			if (hasLabel(labels(p))) continue;

			std::vector<int> L;
			for (auto& offset : vec) {
				auto neighbor = p + offset;
				if (!isInside(img, neighbor)) continue;
				if (!hasLabel(labels(neighbor))) continue;
				L.push_back(labels(neighbor));
			}

			if (L.size() == 0) {
				label++;
				labels(p) = label;
				edges.resize(label + 1);
				continue;
			}

			auto x = *std::min_element(L.begin(), L.end());
			labels(p) = x;
			for (auto& label : L) {
				if (label == x) continue;
				edges[label].push_back(x);
				edges[x].push_back(label);
			}
		}
	}

	if (partial) return labels;

	int newlabel = 0;
	std::vector<int> newlabels(label + 1);
	for (int i = 1; i <= label; i++) {
		if (hasLabel(newlabels[i])) continue;

		newlabel++;
		newlabels[i] = newlabel;

		std::queue<int> q;
		q.push(i);
		while (!q.empty()) {
			auto label = q.front(); q.pop();
			for (auto& neighbor : edges[label]) {
				if (hasLabel(newlabels[neighbor])) continue;
				newlabels[neighbor] = newlabel;
				q.push(neighbor);
			}
		}
	}

	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			Point p(j, i);
			labels(p) = newlabels[labels(p)];
		}
	}

	return labels;
}

void ex2() {
	char fname[MAX_PATH];
	if (openFileDlg(fname)) {
		Mat_<Vec3b> img = imread(fname, IMREAD_COLOR);
		auto bwImg = RGB2BlackWhite(img);
		auto labels = BFS_label(bwImg, vec8);
		auto outputImg = colorlabels(labels);
		
		imshow("original image", img);
		imshow("output image", outputImg);
		waitKey(0);
	}
}

void ex3() {
	char fname[MAX_PATH];
	if (openFileDlg(fname)) {
		Mat_<Vec3b> img = imread(fname, IMREAD_COLOR);
		auto bwImg = RGB2BlackWhite(img);

		auto labels = twoway_label(bwImg, vec8);
		auto outputImg = colorlabels(labels);

		auto partial = twoway_label(bwImg, vec8, true);
		auto partialOutputImg = colorlabels(partial);

		imshow("original image", img);
		imshow("partial output image", partialOutputImg);
		imshow("output image", outputImg);
		waitKey(0);
	}
}

void addLab05ToMenu(std::vector<MenuItem>& menuList) {
	menuList.push_back(createMenuItem("ex2", ex2));
	menuList.push_back(createMenuItem("ex3", ex3));
}