// OpenCVApplication.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "common.h"


using namespace std;
using namespace cv;

int main() {
	VideoCapture cap(0);
	if (!cap.isOpened()) {
		printf("Cannot open camera");
		return -1;
	}

	auto resolution = Size((int)cap.get(CAP_PROP_FRAME_WIDTH), 
		(int)cap.get(CAP_PROP_FRAME_HEIGHT));

	Mat frame;

	const char* WIN_WEBCAM = "Webcam";
	namedWindow(WIN_WEBCAM, WINDOW_AUTOSIZE);
	moveWindow(WIN_WEBCAM, 0, 0);

	const char* WIN_SNAP = "Snap";
	namedWindow(WIN_SNAP, WINDOW_AUTOSIZE);
	moveWindow(WIN_SNAP, resolution.width + 10, 0);

	char c;

	while (true) {
		cap >> frame;
		if (frame.empty()) {
			printf("End of video.\n");
			break;
		}

		imshow(WIN_WEBCAM, frame);

		c = waitKey(10);
		// ESC
		if (c == 27) {
			printf("Exiting capture...\n");
			break;
		}
		if (c == 's') {
			imshow(WIN_SNAP, frame);
		}
	}

	printf("Hello World!");
	return 0;
}