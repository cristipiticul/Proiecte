#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

Vec3f convolute(Mat m1, Mat m2)
{
	Vec3f res(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < m1.rows; i++) {
		for (int j = 0; j < m1.cols; j++) {
			auto& pixel1 = m1.at<Vec3f>(i, j);
			auto& pixel2 = m2.at<int>(i, j);
			for (int k = 0; k < 3; k++) {
				res[k] += pixel1[k] * pixel2;
			}
		}
	}
	return res;
}

int main(int argc, char** argv)
{
	Mat image_prev;
	Mat image_prev_gray;
	image_prev = imread("0000\\000000.png", CV_LOAD_IMAGE_COLOR);
	Mat image_next;
	Mat image_next_gray;
	image_next = imread("0000\\000001.png", CV_LOAD_IMAGE_COLOR);

	if (image_prev.empty() || image_next.empty()) // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}

	cvtColor(image_prev, image_prev_gray, CV_BGR2GRAY);
	cvtColor(image_next, image_next_gray, CV_BGR2GRAY);

	std::vector<cv::Point2f> features_prev;
	std::vector<cv::Point2f> features_next;
	std::vector<unsigned char> status;
	std::vector<float> err;
	
	cv::goodFeaturesToTrack(
		image_prev_gray, features_prev, 1000, 0.01, 0.5, Mat(), 3, 0, 0.04
	);

	cv::calcOpticalFlowPyrLK(
		image_prev_gray, image_next_gray, features_prev, features_next, status, err
	);

	Mat dx = Mat_<Vec3f>(Size(image_prev.rows, image_prev.cols));
	Mat dy = Mat_<Vec3f>(Size(image_prev.rows, image_prev.cols));
	Mat dt = Mat_<Vec3f>(Size(image_prev.rows, image_prev.cols));

	for (int i = 1; i < image_next.rows - 1 && i < image_prev.rows - 1; i++) {
		for (int j = 1; j < image_next.cols - 1 && j < image_prev.cols - 1; j++) {
			Mat neighb_prev = image_prev.rowRange(Range(i - 1, i + 1)).colRange(Range(j - 1, j + 1));
			Mat neighb_next = image_next.rowRange(Range(i - 1, i + 1)).colRange(Range(j - 1, j + 1));
			dx = convolute();
		}
	}

	for (int i = 0; i < features_prev.size(); i++) {
		if (status[i] == 1) {
			cv::line(image_prev, features_prev[i], features_next[i], cv::Scalar(0, 255, 0));
		}
	}

	namedWindow("Display window", WINDOW_AUTOSIZE); // Create a window for display.
	
	int nr_frame = 0;
	while (true) {
		if (nr_frame == 1) {
			imshow("Display window", image_next);
		}
		else {
			imshow("Display window", image_prev);
		}

		nr_frame = 1 - nr_frame;
		waitKey(0); // Wait for a keystroke in the window
	}
	return 0;
}