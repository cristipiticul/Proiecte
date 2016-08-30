#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

#define PYRAMID_LEVELS 3
#define WINDOW_X 2
#define WINDOW_Y 2
#define K_STEPS 5
#define ACCURACY_THRESHOLD 1

Mat smoothingKernel(5, 5, CV_64F);
Mat dxKernel(3, 3, CV_64F);
Mat dyKernel(3, 3, CV_64F);

void prepareSmoothingKernel() {
	Mat kernel_row(1, 5, CV_64F);
	kernel_row.at<double>(Point(0, 0)) = 1.0 / 16.0;
	kernel_row.at<double>(Point(1, 0)) = 1.0 / 4.0;
	kernel_row.at<double>(Point(2, 0)) = 3.0 / 8.0;
	kernel_row.at<double>(Point(3, 0)) = 1.0 / 4.0;
	kernel_row.at<double>(Point(4, 0)) = 1.0 / 16.0;
	Mat kernel_col = kernel_row.t();
	smoothingKernel = kernel_col * kernel_row;
}


void prepareDerivativeKernels()
{
	dxKernel.at<double>(Point(0, 0)) = -1;
	dxKernel.at<double>(Point(0, 1)) = -2;
	dxKernel.at<double>(Point(0, 2)) = -1;

	dxKernel.at<double>(Point(1, 0)) = 0;
	dxKernel.at<double>(Point(1, 1)) = 0;
	dxKernel.at<double>(Point(1, 2)) = 0;

	dxKernel.at<double>(Point(2, 0)) = 1;
	dxKernel.at<double>(Point(2, 1)) = 2;
	dxKernel.at<double>(Point(2, 2)) = 1;

	dyKernel.at<double>(Point(0, 0)) = 1;
	dyKernel.at<double>(Point(1, 0)) = 2;
	dyKernel.at<double>(Point(2, 0)) = 1;

	dyKernel.at<double>(Point(0, 1)) = 0;
	dyKernel.at<double>(Point(1, 1)) = 0;
	dyKernel.at<double>(Point(2, 1)) = 0;

	dyKernel.at<double>(Point(0, 2)) = -1;
	dyKernel.at<double>(Point(1, 2)) = -2;
	dyKernel.at<double>(Point(2, 2)) = -1;
}

void prepareKernels()
{
	prepareSmoothingKernel();
	prepareDerivativeKernels();
}

Mat* computeOneLevelPyramid(Mat* image)
{
	Mat* result = new Mat(image->rows / 2, image->cols / 2, CV_64FC1);
	Mat tmp(image->rows, image->cols, CV_64FC1);
	filter2D(*image, tmp, -1, smoothingKernel, Point(-1, -1), 0.0, BORDER_REPLICATE);
	for (int i = 0; i < image->rows / 2; i++) {
		for (int j = 0; j < image->cols / 2; j++) {
			result->at<double>(i, j) = tmp.at<double>(2 * i, 2 * j);
		}
	}
	return result;
}

Mat** computePyramids(Mat image)
{
	Mat** result = new Mat*[PYRAMID_LEVELS + 1];
	result[0] = new Mat(image);
	for (int i = 1; i <= PYRAMID_LEVELS; i++) {
		result[i] = computeOneLevelPyramid(result[i - 1]);
	}
	return result;
}

Mat computeXDerivative(Mat image)
{
	Mat result;
	filter2D(image, result, -1, dxKernel, Point(-1, -1), 0.0, BORDER_REPLICATE);
	return result;
}

Mat computeYDerivative(Mat image)
{
	Mat result;
	filter2D(image, result, -1, dyKernel, Point(-1, -1), 0.0, BORDER_REPLICATE);
	return result;
}

void deallocatePyramids(Mat** pyramids)
{
	for (int i = 0; i < PYRAMID_LEVELS; i++) {
		delete pyramids[i];
	}
	//delete[] pyramids; It crashes here.... WTF?
}

vector<Point2d> computeOpticalFlow(Mat input_source, Mat input_destination, vector<Point2i> points_to_track, vector<Point2d> guess)
{
	vector<Point2d> result;
	result.resize(points_to_track.size());

	Mat source_derivative_x = computeXDerivative(input_source);
	Mat source_derivative_y = computeYDerivative(input_source);

	for (int i = 0; i < points_to_track.size(); i++) {
		int x = points_to_track[i].x;
		int y = points_to_track[i].y;
		if (x < WINDOW_X || y < WINDOW_Y || x >= input_source.cols - WINDOW_X || y >= input_source.rows - WINDOW_Y) {
			result[i] = Point2d(-1000000, -1000000);
			continue;
		}
		Point2d displacement(0.0, 0.0);
		Mat spatial_gradient(2, 2, CV_64F);
		spatial_gradient.at<double>(0, 0) = 0;
		spatial_gradient.at<double>(0, 1) = 0;
		spatial_gradient.at<double>(1, 0) = 0;
		spatial_gradient.at<double>(1, 1) = 0;
		for (int xx = x - WINDOW_X; xx <= x + WINDOW_X; xx++) {
			for (int yy = y - WINDOW_Y; yy <= y + WINDOW_Y; yy++) {
				double dx = source_derivative_x.at<double>(yy, xx);
				double dy = source_derivative_y.at<double>(yy, xx);
				spatial_gradient.at<double>(0, 0) += dx * dx;
				spatial_gradient.at<double>(0, 1) += dx * dy;
				spatial_gradient.at<double>(1, 0) += dx * dy;
				spatial_gradient.at<double>(1, 1) += dy * dy;
			}
		}
		Mat spatial_gradient_inv = spatial_gradient.inv();
		Point2d curr_guess = guess[i] * 2;
		if (curr_guess.x < -10000) {
			result[i] = curr_guess;
			continue;
		}
		Point2d optical_flow(100.0, 100.0);

		for (int k = 0; k < K_STEPS && (optical_flow.x * optical_flow.x + optical_flow.y * optical_flow.y) > ACCURACY_THRESHOLD; k++) {
			int new_x = x + (int)(curr_guess.x + displacement.x);
			int new_y = y + (int)(curr_guess.y + displacement.y);
			if (new_x < 0 || new_y < 0 || new_x >= input_source.cols || new_y >= input_source.rows) {
				displacement.x = -1000000;
				displacement.y = -1000000;
				break;
			}
			double image_difference = input_source.at<double>(y, x) - input_destination.at<double>(new_y, new_x);
			Point2d mismatch_vector(0, 0);
			for (int xx = x - WINDOW_X; xx <= x + WINDOW_X; xx++) {
				for (int yy = y - WINDOW_Y; yy <= y + WINDOW_Y; yy++) {
					mismatch_vector.x += image_difference * source_derivative_x.at<double>(y, x);
					mismatch_vector.y += image_difference * source_derivative_y.at<double>(y, x);
				}
			}
			optical_flow.x = spatial_gradient_inv.at<double>(0, 0) * mismatch_vector.x + spatial_gradient_inv.at<double>(0, 1) * mismatch_vector.y;
			optical_flow.y = spatial_gradient_inv.at<double>(1, 0) * mismatch_vector.x + spatial_gradient_inv.at<double>(1, 1) * mismatch_vector.y;
			displacement.x += optical_flow.x;
			displacement.y += optical_flow.y;
		}
		curr_guess.x += displacement.x;
		curr_guess.y += displacement.y;
		result[i] = curr_guess;
	}

	return result;
}


int main_old()
{
	prepareKernels();

	Mat image_prev;
	image_prev = imread("0000\\000395.png", CV_LOAD_IMAGE_COLOR);
	image_prev.convertTo(image_prev, CV_64FC3);
	Mat image_next;
	image_next = imread("0000\\000396.png", CV_LOAD_IMAGE_COLOR);
	image_next.convertTo(image_next, CV_64FC3);

	Mat layers_prev[3];
	split(image_prev, layers_prev);
	Mat layers_next[3];
	split(image_next, layers_next);

	Mat** image_prev_pyramids = computePyramids(layers_prev[0]);
	Mat** image_next_pyramids = computePyramids(layers_next[0]);

	vector<Point2i> points_to_track;
	Mat good_features_img;
	image_prev_pyramids[0]->convertTo(good_features_img, CV_8U);
	goodFeaturesToTrack(good_features_img, points_to_track, 1000, 0.01, 0.5, Mat(), 3, 0, 0.04);
	vector<Point2i> points_to_track_3(points_to_track.size());
	vector<Point2i> points_to_track_2(points_to_track.size());
	vector<Point2i> points_to_track_1(points_to_track.size());
	vector<Point2d> zero_guesses(points_to_track.size());
	for (int i = 0; i < points_to_track.size(); i++) {
		zero_guesses[i] = Point2d(0.0, 0.0);
		points_to_track_1[i] = points_to_track[i] / 2;
		points_to_track_2[i] = points_to_track[i] / 4;
		points_to_track_3[i] = points_to_track[i] / 8;
	}
	vector<Point2d> optical_flow_3 = computeOpticalFlow(*image_prev_pyramids[3], *image_next_pyramids[3], points_to_track_3, zero_guesses);
	vector<Point2d> optical_flow_2 = computeOpticalFlow(*image_prev_pyramids[2], *image_next_pyramids[2], points_to_track_2, optical_flow_3);
	vector<Point2d> optical_flow_1 = computeOpticalFlow(*image_prev_pyramids[1], *image_next_pyramids[1], points_to_track_1, optical_flow_2);
	vector<Point2d> optical_flow = computeOpticalFlow(*image_prev_pyramids[0], *image_next_pyramids[0], points_to_track, optical_flow_1);
	Mat drawing;
	image_prev_pyramids[0]->convertTo(drawing, CV_8U);
	Mat next_drawing;
	image_next_pyramids[0]->convertTo(next_drawing, CV_8U);
	for (int i = 0; i < optical_flow.size(); i++) {
		int y = points_to_track[i].y;
		int x = points_to_track[i].x;
		Point2d flow = optical_flow[i];
		if (flow.x * flow.x + flow.y * flow.y < 400 && flow.y < 0) {
			line(drawing, Point(x, y), Point(x + flow.x, y + flow.y), Scalar(255), 1);
			line(next_drawing, Point(x, y), Point(x + flow.x, y + flow.y), Scalar(255), 1);
		}
	}
	while (true) {
		imshow("opticalflow", drawing);
		waitKey(0);
		imshow("opticalflow", next_drawing);
		waitKey(0);
	}
	
	deallocatePyramids(image_prev_pyramids);
	deallocatePyramids(image_next_pyramids);

	/*
	Mat channels_prev[3];
	Mat channels_next[3];
	split(image_prev, channels_prev);
	split(image_next, channels_next);
	*/
}