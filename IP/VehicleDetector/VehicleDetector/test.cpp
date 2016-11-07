/*
	Vehicle detector from color image stream.
	References:
		https://pdfs.semanticscholar.org/1488/249a5895851f6d46dd6b60843ce1c93f49ce.pdf (computing optical flow from color images)
		http://image.diku.dk/imagecanon/material/HornSchunckOptical_Flow.pdf (section 7 for computing derivatives)
*/

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <queue>

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

void showHistogram(Mat src) {
	Mat dst;

	/// Separate the image in 3 places ( B, G and R )
	vector<Mat> bgr_planes;
	split(src, bgr_planes);

	/// Establish the number of bins
	int histSize = 256;

	/// Set the ranges ( for B,G,R) )
	float range[] = { 0, 256 };
	const float* histRange = { range };

	bool uniform = true; bool accumulate = false;

	Mat b_hist, g_hist, r_hist;

	/// Compute the histograms:
	calcHist(&bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate);
	calcHist(&bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate);
	calcHist(&bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate);

	// Draw the histograms for B, G and R
	int hist_w = 512; int hist_h = 400;
	int bin_w = cvRound((double)hist_w / histSize);

	Mat histImage(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));

	/// Normalize the result to [ 0, histImage.rows ]
	normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
	normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
	normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());

	/// Draw for each channel
	for (int i = 1; i < histSize; i++)
	{
		line(histImage, Point(bin_w*(i - 1), hist_h - cvRound(b_hist.at<float>(i - 1))),
			Point(bin_w*(i), hist_h - cvRound(b_hist.at<float>(i))),
			Scalar(255, 0, 0), 2, 8, 0);
		line(histImage, Point(bin_w*(i - 1), hist_h - cvRound(g_hist.at<float>(i - 1))),
			Point(bin_w*(i), hist_h - cvRound(g_hist.at<float>(i))),
			Scalar(0, 255, 0), 2, 8, 0);
		line(histImage, Point(bin_w*(i - 1), hist_h - cvRound(r_hist.at<float>(i - 1))),
			Point(bin_w*(i), hist_h - cvRound(r_hist.at<float>(i))),
			Scalar(0, 0, 255), 2, 8, 0);
	}

	/// Display
	namedWindow("calcHist Demo", CV_WINDOW_AUTOSIZE);
	imshow("calcHist Demo", histImage);

	waitKey(0);
}

void showGrayscaleOpticalFlow(Mat image_prev, Mat image_next)
{
	namedWindow("Display window", WINDOW_AUTOSIZE); // Create a window for display
	Mat channels_prev[3];
	Mat channels_next[3];
	split(image_prev, channels_prev);
	split(image_next, channels_next);
	Mat channels_prev_filtered[3];
	Mat channels_next_filtered[3];
	Mat channels_prev_edges[3];
	Mat channels_next_edges[3];
	Mat edges(image_prev.rows, image_prev.cols, CV_8U);
	for (int channel = 0; channel < 3; channel++) {
		blur(channels_prev[channel], channels_prev_filtered[channel], Size(3, 3));
		blur(channels_next[channel], channels_next_filtered[channel], Size(3, 3));

		const int threshold = 50;
		const int ratio = 3;
		Canny(channels_prev_filtered[channel], channels_prev_edges[channel], threshold, threshold * ratio);
		Canny(channels_next_filtered[channel], channels_next_edges[channel], threshold, threshold * ratio);
	}
	for (int i = 0; i < image_prev.rows; i++) {
		for (int j = 0; j < image_prev.cols; j++) {
			bool is_on_edge = true;
			for (int channel = 0; channel < 3; channel++) {
				if (channels_prev_edges[channel].at<unsigned char>(i, j) < 200) {
					is_on_edge = false;
					break;
				}
			}
			if (is_on_edge) {
				edges.at<unsigned char>(i, j) = 255;
			}
			else {
				edges.at<unsigned char>(i, j) = 0;
			}
		}
	}

	std::vector<cv::Point2f> features_prev;
	std::vector<cv::Point2f> features_next;
	std::vector<unsigned char> status;
	std::vector<float> err;


	//cv::goodFeaturesToTrack(
	//	image_prev_edges_filtered, features_prev, 100, 0.01, 0.5, Mat(), 3, 0, 0.04
	//);
	int pixel_nr = 0;
	for (int i = 0; i < image_prev.rows; i++) {
		for (int j = 0; j < image_prev.cols; j++) {
			if (edges.at<unsigned char>(i, j) == 255) {
				if (pixel_nr == 10) {
					features_prev.push_back(Point2i(i, j));
					pixel_nr = 0;
				}
				else {
					pixel_nr++;
				}
			}
		}
	}
	/*
	cv::calcOpticalFlowPyrLK(
		edges, image_next_edges_filtered, features_prev, features_next, status, err
	);

	for (int i = 0; i < features_prev.size(); i++) {
		if (status[i] == 1) {
			cv::line(image_prev_edges, features_prev[i], features_next[i], cv::Scalar(255, 255, 0));
			cv::line(image_next_edges, features_prev[i], features_next[i], cv::Scalar(255, 255, 0));
		}
	}

	imshow("Display window", image_prev_edges);
	waitKey(0);
	*/
	imshow("Display window", edges);
	waitKey(0);
}

int image_nr = 0;
char image_name[20];
void computeDenseOpticalFlow(Mat image_prev, Mat image_next)
{
	sprintf(image_name, "output_images\\input_%i.png", image_nr);
	imwrite(image_name, image_prev);
	Mat image_prev_red;
	extractChannel(image_prev, image_prev_red, 2);
	Mat image_prev_green;
	extractChannel(image_prev, image_prev_green, 1);
	Mat image_prev_blue;
	extractChannel(image_prev, image_prev_blue, 0);
	imwrite("output_images\\red.png", image_prev_red);
	imwrite("output_images\\green.png", image_prev_green);
	imwrite("output_images\\blue.png", image_prev_blue);

	Mat image_next_red;
	extractChannel(image_next, image_next_red, 2);
	Mat image_next_green;
	extractChannel(image_next, image_next_green, 1);
	Mat image_next_blue;
	extractChannel(image_next, image_next_blue, 0);

	Mat optical_flow_red;
	calcOpticalFlowFarneback(image_prev_red, image_next_red, optical_flow_red, 0.5, 3, 10, 3, 5, 1.2, 0);

	Mat optical_flow_green;
	calcOpticalFlowFarneback(image_prev_green, image_next_green, optical_flow_green, 0.5, 3, 10, 3, 5, 1.2, 0);

	Mat optical_flow_blue;
	calcOpticalFlowFarneback(image_prev_blue, image_next_blue, optical_flow_blue, 0.5, 3, 10, 3, 5, 1.2, 0);

	Mat red_flow_img(image_prev.size(), CV_8UC1);
	float max_red_flow = 2348;
	float sum_flows = 0.0f;
	for (int i = 0; i < image_prev.size().height; i++) {
		for (int j = 0; j < image_prev.size().width; j++) {
			float dy = optical_flow_red.at<Point2f>(i, j).y;
			float dx = optical_flow_red.at<Point2f>(i, j).x;
			float d = dx * dx + dy * dy;
			sum_flows += d;
			if (d > max_red_flow) {
				max_red_flow = d;
			}
			int color = (int)(d * 10.0f);
			if (color > 255) {
				color = 255;
			}
			if (color < 0) {
				color = 0;
			}
			red_flow_img.at<unsigned char>(i, j) = (unsigned char)(color);
		}
	}

	Mat green_flow_img(image_prev.size(), CV_8UC1);
	float max_green_flow = 2348;
	for (int i = 0; i < image_prev.size().height; i++) {
		for (int j = 0; j < image_prev.size().width; j++) {
			float dy = optical_flow_green.at<Point2f>(i, j).y;
			float dx = optical_flow_green.at<Point2f>(i, j).x;
			float d = dx * dx + dy * dy;
			sum_flows += d;
			if (d > max_green_flow) {
				max_green_flow = d;
			}
			int color = (int)(d * 10.0f);
			if (color > 255) {
				color = 255;
			}
			if (color < 0) {
				color = 0;
			}
			green_flow_img.at<unsigned char>(i, j) = (unsigned char)(color);
		}
	}

	Mat blue_flow_img(image_prev.size(), CV_8UC1);
	float max_blue_flow = 2348;
	for (int i = 0; i < image_prev.size().height; i++) {
		for (int j = 0; j < image_prev.size().width; j++) {
			float dy = optical_flow_blue.at<Point2f>(i, j).y;
			float dx = optical_flow_blue.at<Point2f>(i, j).x;
			float d = dx * dx + dy * dy;
			sum_flows += d;
			if (d > max_blue_flow) {
				max_blue_flow = d;
			}
			int color = (int)(d * 10.0f);
			if (color > 255) {
				color = 255;
			}
			if (color < 0) {
				color = 0;
			}
			blue_flow_img.at<unsigned char>(i, j) = (unsigned char)(color);
		}
	}
	float max_flow = max(max_blue_flow, max(max_green_flow, max_red_flow));

	const float MIN_FLOW_SQUARED = 160;
	Mat image_prev_visible;
	image_prev.convertTo(image_prev_visible, CV_8U);
	Mat big_flow(image_prev.rows, image_prev.cols, CV_8U);
	for (int i = 0; i < image_prev.size().height; i++) {
		for (int j = 0; j < image_prev.size().width; j++) {
			Point2f flows[3];
			flows[0] = optical_flow_red.at<Point2f>(i, j);
			flows[1] = optical_flow_green.at<Point2f>(i, j);
			flows[2] = optical_flow_blue.at<Point2f>(i, j);
			for (int channel = 0; channel < 3; channel++) {
				if (flows[channel].x * flows[channel].x + flows[channel].y * flows[channel].y > MIN_FLOW_SQUARED) {
					big_flow.at<unsigned char>(i, j) = 255;
				} else {
					big_flow.at<unsigned char>(i, j) = 0;
				}
			}
		}
	}

	const int MIN_CAR_WIDTH = 100;
	const int MIN_CAR_HEIGHT = 60;
	const int dx[] = { -1, 0, 1, 0 };
	const int dy[] = { 0, -1, 0, 1 };
	sprintf(image_name, "output_images\\flow_%i.png", image_nr);
	imwrite(image_name, big_flow);

	for (int i = 0; i < image_prev.rows; i++) {
		for (int j = 0; j < image_prev.cols; j++) {
			if (big_flow.at<unsigned char>(i, j) == 255) {
				Rect2i boundary(j, i, 0, 0);
				queue<Point2i> q;
				q.push(Point2i(j, i));
				big_flow.at<unsigned char>(i, j) = 0;
				while (!q.empty()) {
					Point2i p = q.front();
					if (p.x < boundary.x) {
						boundary.x = p.x;
					}
					if (p.y < boundary.y) {
						boundary.y = p.y;
					}
					if (p.x > boundary.x + boundary.width) {
						boundary.width = p.x - boundary.x;
					}
					if (p.y > boundary.y + boundary.height) {
						boundary.height = p.y - boundary.y;
					}

					for (int dir = 0; dir < 4; dir++) {
						Point2i new_p(p.x + dx[dir], p.y + dy[dir]);
						if (new_p.inside(Rect2i(0, 0, image_prev.cols, image_prev.rows)) && big_flow.at<unsigned char>(new_p.y, new_p.x) == 255) {
							big_flow.at<unsigned char>(new_p.y, new_p.x) = 0;
							q.push(new_p);
						}
					}

					q.pop();
				}
				if (boundary.width >= MIN_CAR_WIDTH && boundary.height >= MIN_CAR_HEIGHT) {
					rectangle(image_prev_visible, boundary, Scalar(255, 0, 0), 2);
				}
			}
		}
	}

	int nr_frame = 0;

	sprintf(image_name, "output_images\\output_%i.png", image_nr);
	imwrite(image_name, image_prev_visible);
	image_nr++;

	imshow("Image Prev", image_prev_visible);
	imshow("Red flow img", red_flow_img);
	imshow("Green flow img", green_flow_img);
	imshow("Blue flow img", blue_flow_img);
	waitKey(0);
}

// Compute X derivative using 2x2 pixel matrices
Vec3f computeXDerivative(Mat neighb_prev, Mat neighb_next)
{
	Vec3f result;
	auto& pixel000 = neighb_prev.at<Vec3b>(0, 0);
	auto& pixel010 = neighb_prev.at<Vec3b>(0, 1);
	auto& pixel100 = neighb_prev.at<Vec3b>(1, 0);
	auto& pixel110 = neighb_prev.at<Vec3b>(1, 1);
	auto& pixel001 = neighb_next.at<Vec3b>(0, 0);
	auto& pixel011 = neighb_next.at<Vec3b>(0, 1);
	auto& pixel101 = neighb_next.at<Vec3b>(1, 0);
	auto& pixel111 = neighb_next.at<Vec3b>(1, 1);
	for (int i = 0; i < 3; i++) {
		result[i] = 1.0f / 4 * (pixel010[i] - pixel000[i] + pixel110[i] - pixel100[i] + pixel011[i] - pixel001[i] + pixel111[i] - pixel101[i]);
	}
	return result;
}

// Compute Y derivative using 2x2 pixel matrices
Vec3f computeYDerivative(Mat neighb_prev, Mat neighb_next)
{
	Vec3f result;
	auto& pixel000 = neighb_prev.at<Vec3b>(0, 0);
	auto& pixel010 = neighb_prev.at<Vec3b>(0, 1);
	auto& pixel100 = neighb_prev.at<Vec3b>(1, 0);
	auto& pixel110 = neighb_prev.at<Vec3b>(1, 1);
	auto& pixel001 = neighb_next.at<Vec3b>(0, 0);
	auto& pixel011 = neighb_next.at<Vec3b>(0, 1);
	auto& pixel101 = neighb_next.at<Vec3b>(1, 0);
	auto& pixel111 = neighb_next.at<Vec3b>(1, 1);
	for (int i = 0; i < 3; i++) {
		result[i] = 1.0f / 4 * (pixel100[i] - pixel000[i] + pixel101[i] - pixel001[i] + pixel110[i] - pixel010[i] + pixel111[i] - pixel011[i]);
	}
	return result;
}

// Compute T derivative using 2x2 pixel matrices
Vec3f computeTDerivative(Mat neighb_prev, Mat neighb_next)
{
	Vec3f result;
	auto& pixel000 = neighb_prev.at<Vec3b>(0, 0);
	auto& pixel010 = neighb_prev.at<Vec3b>(0, 1);
	auto& pixel100 = neighb_prev.at<Vec3b>(1, 0);
	auto& pixel110 = neighb_prev.at<Vec3b>(1, 1);
	auto& pixel001 = neighb_next.at<Vec3b>(0, 0);
	auto& pixel011 = neighb_next.at<Vec3b>(0, 1);
	auto& pixel101 = neighb_next.at<Vec3b>(1, 0);
	auto& pixel111 = neighb_next.at<Vec3b>(1, 1);
	for (int i = 0; i < 3; i++) {
		result[i] = 1.0f / 4 * (pixel001[i] - pixel000[i] + pixel011[i] - pixel010[i] + pixel101[i] - pixel100[i] + pixel111[i] - pixel110[i]);
	}
	return result;
}

inline bool is_inside(int i, int j, Size size)
{
	return i >= 0 && j >= 0 && i < size.height && j < size.width;
}

int di[] = { 0, 0, 1, 0, -1 };
int dj[] = { 0, 1, 0, -1, 0 };
int num_dirs = 5;

inline int difference(Mat img1, int i1, int j1, Mat img2, int i2, int j2)
{
	int delta = 0;
	for (int dir = 0; dir < num_dirs; dir++) {
		int new_i1 = i1 + di[dir];
		int new_j1 = j1 + dj[dir];
		int new_i2 = i2 + di[dir];
		int new_j2 = j2 + dj[dir];
		if (is_inside(new_i1, new_j1, img1.size()) && is_inside(new_i2, new_j2, img2.size())) {
			for (int channel = 0; channel < 3; channel++) {
				delta += abs(img1.at<Vec3b>(new_i1, new_j1)[channel] - img2.at<Vec3b>(new_i2, new_j2)[channel]);
			}
		}
	}
	return delta;
}

int main232(int argc, char** argv)
{
	VideoCapture capture;
	Mat image_prev;
	Mat image_next;
	capture.set(CV_CAP_PROP_FOURCC, CV_FOURCC('A', 'V', 'C', '1'));
	capture.open("FroggerHighway.mp4");
	if (!capture.isOpened()) {
		cerr << "Failed to open video file!\n" << endl;
		system("Pause");
		return 1;
	}
	capture.set(CAP_PROP_POS_FRAMES, 50);
	capture >> image_prev;
	resize(image_prev, image_prev, image_prev.size() / 2);
	while (true) {
		capture >> image_next;
		resize(image_next, image_next, image_next.size() / 2);
		computeDenseOpticalFlow(image_prev, image_next);
		image_next.copyTo(image_prev);
	}
	/*
	Mat image_prev;
	image_prev = imread("0000\\000000.png", CV_LOAD_IMAGE_COLOR);
	image_prev.convertTo(image_prev, CV_64FC3);
	Mat image_next;
	image_next = imread("0000\\000001.png", CV_LOAD_IMAGE_COLOR);
	image_next.convertTo(image_next, CV_64FC3);
	int image_nr = 2;
	char image_name_buf[20];

	while (!image_next.empty()) {
		computeDenseOpticalFlow(image_prev, image_next);
		image_prev = image_next;
		sprintf(image_name_buf, "0000\\%06d.png", image_nr);
		image_next = imread(image_name_buf, CV_LOAD_IMAGE_COLOR);
		image_nr++;
	}

	if (image_prev.empty() || image_next.empty()) // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}
	*/


	// showHistogram(image_prev);
	//showHistogram(image_next);
	/*
	cv::Mat lab_image;
	cv::cvtColor(image_prev, lab_image, CV_BGR2Lab);

	// Extract the L channel
	std::vector<cv::Mat> lab_planes(3);
	cv::split(lab_image, lab_planes);  // now we have the L image in lab_planes[0]

									   // apply the CLAHE algorithm to the L channel
	cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
	clahe->setClipLimit(4);
	cv::Mat dst;
	clahe->apply(lab_planes[0], dst);

	// Merge the the color planes back into an Lab image
	dst.copyTo(lab_planes[0]);
	cv::merge(lab_planes, lab_image);

	// convert back to RGB
	cv::Mat image_clahe;
	cv::cvtColor(lab_image, image_clahe, CV_Lab2BGR);
	imshow("image", image_prev);
	imshow("image_clahe", image_clahe);
	waitKey();
	return 0;

	const int THRESH = 1000;
	for (int i = 0; i < image_prev.size().height; i++) {
		for (int j = 0; j < image_prev.size().width; j++) {
			bool is_different = true;
			for (int k = 0; k < num_dirs; k++) {
				if (difference(image_prev, i, j, image_prev, i + 1, j) < THRESH) {
					is_different = false;
					break;
				}
			}
			if (is_different) {

				image_prev.at<Vec3b>(i, j)[0] = 0;
				image_prev.at<Vec3b>(i, j)[1] = 0;
				image_prev.at<Vec3b>(i, j)[2] = 255;
			}
		}
	}

	imshow("image", image_prev);
	imshow("image_next", image_next);
	waitKey();
	*/
/*
	cv::Mat hsv;
	cv::cvtColor(image_prev, hsv, CV_BGR2HSV);

	vector<Mat> channels;
	split(hsv, channels);

	imshow("value", channels[2]);
	waitKey();*/

	return 0;

	// showGrayscaleOpticalFlow(image_prev, image_next);
	// return 0;

	Mat dx = Mat_<Vec3f>(Size(image_prev.cols, image_prev.rows));
	Mat dy = Mat_<Vec3f>(Size(image_prev.cols, image_prev.rows));
	Mat dt = Mat_<Vec3f>(Size(image_prev.cols, image_prev.rows));

	for (int i = 0; i < image_next.rows - 1 && i < image_prev.rows - 1; i++) {
		for (int j = 0; j < image_next.cols - 1 && j < image_prev.cols - 1; j++) {
			Mat neighb_prev = image_prev.rowRange(Range(i, i + 2)).colRange(Range(j, j + 2));
			Mat neighb_next = image_next.rowRange(Range(i, i + 2)).colRange(Range(j, j + 2));
			dx.at<Vec3f>(i, j) = computeXDerivative(neighb_prev, neighb_next);
			dy.at<Vec3f>(i, j) = computeYDerivative(neighb_prev, neighb_next);
			dt.at<Vec3f>(i, j) = computeTDerivative(neighb_prev, neighb_next);
		}
	}

	const int neighb_dx[] = { 0, 1, 1, 0 };// , -1, -1, -1, 0, 1 };
	const int neighb_dy[] = { 0, 0, 1, 1 };// , 1, 0, -1, -1, -1 };
	const int num_dirs = 4;

	for (int i = 1; i < image_next.rows - 1 && i < image_prev.rows - 1; i += 100) {
		for (int j = 1; j < image_next.cols - 1 && j < image_prev.cols - 1; j += 100) {
			Mat A = Mat_<float>(num_dirs * 3, 2);
			cout << i << '\n';
			for (int channel = 0; channel < 3; channel++) {
				for (int dir = 0; dir < num_dirs; dir++) {
					A.at<float>(channel * num_dirs + dir, 0) = dx.at<Vec3f>(i + neighb_dy[dir], j + neighb_dx[dir])[channel];
					A.at<float>(channel * num_dirs + dir, 1) = dy.at<Vec3f>(i + neighb_dy[dir], j + neighb_dx[dir])[channel];
				}
			}
			Mat b = Mat_<float>(num_dirs * 3, 1);
			for (int channel = 0; channel < 3; channel++) {
				for (int dir = 0; dir < num_dirs; dir++) {
					b.at<float>(channel * 4 + dir, 0) = dt.at<Vec3f>(i + neighb_dy[dir], j + neighb_dx[dir])[channel];
				}
			}
			Mat v = (A.t() * A).inv() * (A.t() * b);
			cout << (A * v + b) << '\n';
			cv::line(image_prev, cv::Point(i, j), cv::Point(i + (int)v.at<float>(1, 0), j + (int)v.at<float>(0, 0)), cv::Scalar(0, 255, 0));
			cv::line(image_next, cv::Point(i, j), cv::Point(i + (int)v.at<float>(1, 0), j + (int)v.at<float>(0, 0)), cv::Scalar(0, 255, 0));
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