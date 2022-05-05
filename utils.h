#pragma once

#ifndef UTILS_H
#define UTILS_H

#include "common.h"

bool isInside(const Mat& img, int r, int c);
bool isInside(const Mat& img, const Point& p);
int clamp(int minval, int val, int maxval);
Point mirror(const Point& point);
Point rotate(const Point& point, double angle);
Mat_<uchar> RGB2Gray(const Mat_<Vec3b>& img);
Mat_<uchar> Gray2BlackWhite(const Mat_<uchar>& img, int threshold);
Mat_<uchar> RGB2BlackWhite(const Mat_<Vec3b>& img, const int threshold = 128);
void calcHistogram(int* hist, const Mat_<uchar>& img);
Mat_<uchar> Gray2Negative(const Mat_<uchar>& img);
void showHistogram(const std::string& name, int* hist, const int  hist_cols, const int hist_height);
int minHistValue(int* hist, int size = 256);
int maxHistValue(int* hist, int size = 256);
int minHistIntensity(int* hist, int size = 256);
int maxHistIntensity(int* hist, int size = 256);
void calcFDP(float* fdp, Mat_<uchar> img);
void calcFDPC(float* fdpc, float* fdp, int size = 256);

#endif // !UTILS_H
