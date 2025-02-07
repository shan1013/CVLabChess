#include <opencv2/imgcodecs.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/core/utils/logger.hpp>
#include <iostream>
#include <opencv2/imgproc/types_c.h>



using namespace cv;
using namespace std;


//ID depending on system, default is 0
#define WEBCAM_ID 1
#define EPSILON 0.001
const int EPSILON_LINES = 10;
const int EPSILON_BRIGHTNESS = 50;
vector<vector<string>> chessFields = {
{"A8", "Black Rook"}, {"B8", "Black Knight"}, {"C8", "Black Bishop"}, {"D8", "Black Queen"}, {"E8", "Black King"}, {"F8", "Black Bishop"}, {"G8", "Black Knight"}, {"H8", "Black Rook"},
{"A7", "Black Pawn"}, {"B7", "Black Pawn"}, {"C7", "Black Pawn"}, {"D7", "Black Pawn"}, {"E7", "Black Pawn"}, {"F7", "Black Pawn"}, {"G7", "Black Pawn"}, {"H7", "Black Pawn"},
{"A6", "Empty"}, {"B6", "Empty"}, {"C6", "Empty"}, {"D6", "Empty"}, {"E6", "Empty"}, {"F6", "Empty"}, {"G6", "Empty"}, {"H6", "Empty"},
{"A5", "Empty"}, {"B5", "Empty"}, {"C5", "Empty"}, {"D5", "Empty"}, {"E5", "Empty"}, {"F5", "Empty"}, {"G5", "Empty"}, {"H5", "Empty"},
{"A4", "Empty"}, {"B4", "Empty"}, {"C4", "Empty"}, {"D4", "Empty"}, {"E4", "Empty"}, {"F4", "Empty"}, {"G4", "Empty"}, {"H4", "Empty"},
{"A3", "Empty"}, {"B3", "Empty"}, {"C3", "Empty"}, {"D3", "Empty"}, {"E3", "Empty"}, {"F3", "Empty"}, {"G3", "Empty"}, {"H3", "Empty"},
{"A2", "White Pawn"}, {"B2", "White Pawn"}, {"C2", "White Pawn"}, {"D2", "White Pawn"}, {"E2", "White Pawn"}, {"F2", "White Pawn"}, {"G2", "White Pawn"}, {"H2", "White Pawn"},
{"A1", "White Rook"}, {"B1", "White Knight"}, {"C1", "White Bishop"}, {"D1", "White Queen"}, {"E1", "White King"}, {"F1", "White Bishop"}, {"G1", "White Knight"}, {"H1", "White Rook"}
};

const string WINDOW = "Setting up the Board";
const float WIDTH = 500;
const float HEIGHT = 500;
double IMG_RATIO = 16 / 9;

int threshMinCanny = 10;
int threshMaxCanny = 100;
int threshMinArea = 1000;
int threshMaxArea = 2000;

int sliderMinCanny = 50;
int sliderMaxCanny = 145;
int sliderMinArea = 1000;
int sliderMaxArea = 2000;

//Functions
void recognizeBoard(Mat img);
Mat cannyBoard(Mat img);
Mat contourBoard(Mat img);
void drawLines(Mat img, vector<vector<Point>>& horizontalLines, vector<vector<Point>>& verticalLines);
void getIntersections(const vector<vector<Point>> horizontalLines, const vector<vector<Point>> verticalLines, vector<Point>& intersections);
void drawIntersections(Mat img, const vector<Point> intersections, Scalar color = Scalar(0, 0, 255));
void getBoardFields(vector<Point> intersections, vector<Point>& boardFields);
void getFieldCornerPoints(const vector<Point> intersections, vector<Point>& topLeftPoints, vector<Point>& bottomRightPoints);
void colorField(const Mat board_img, vector<Point>& topLeftPoints, vector<Point>& bottomRightPoints, int index, Scalar color, Mat& mask);
void getMeanFieldColors(const Mat first_img, const Mat second_img, vector<Point>& topLeftPoints, vector<Point>& bottomRightPoints, vector<Point>& meanColors);
Mat makeCanvas(std::vector<cv::Mat>& vecMat, int windowHeight, int nRows);
void getHoughLines(const Mat img, vector<vector<Point>>& horizontalLines, vector<vector<Point>>& verticalLines);
void sortLines(const vector<vector<Point>> src, vector<vector<Point>>& dst);
Mat warpBoard(Mat img, vector<Point> points, float width, float height);
vector<Point> getMaxRect(Mat img);
void drawRect(Mat img, vector<Point> points);

static void on_trackbar_min_canny(int, void*)
{
	threshMinCanny = sliderMinCanny; //set the global variable to the current slidervalue;
}

static void on_trackbar_max_canny(int, void*)
{
	threshMaxCanny = sliderMaxCanny; //set the global variable to the current slidervalue;
}

static void on_trackbar_min_area(int, void*)
{
	threshMinCanny = sliderMinArea; //set the global variable to the current slidervalue;
}

static void on_trackbar_max_area(int, void*)
{
	threshMaxCanny = sliderMaxArea; //set the global variable to the current slidervalue;
}

int main() {
	cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_SILENT);


	VideoCapture cap(WEBCAM_ID);
	namedWindow(WINDOW);
	Mat img, img_scanned, img_static_resized, img_full_static_resized, img_static_warped, img_full_static_warped, img_static_cannyed, img_static_houghed, img_static_intersected;
	vector<vector<Point>> horizontalLines, verticalLines;
	vector<Point> intersections, boardFields, topLeftPoints, bottomRightPoints;
	vector<Point> meanColors;
	bool whitesTurn = true;

	//Mat img_board = imread("Ressources/chessboard_table_lamp_empty.png");
	//Mat img_full_board = imread("Ressources/chessboard_table_lamp_pawns.png");
	Mat img_board = imread("Ressources/game/Schachspiel_empty.jpg");
	//Mat img_full_board =  imread("Ressources/game/WIN_20220114_15_24_47_Pro.jpg");
	cout << "img width: " << img_board.cols << " img height: " << img_board.rows << endl;
	IMG_RATIO = img_board.cols / img_board.rows;
	cout << IMG_RATIO << "->" << cvRound(400 * IMG_RATIO) << endl;
	resize(img_board, img_static_resized, { 600, 400 });
	//resize(img_full_board, img_full_static_resized, { 600, 400 });
	cout << "img width: " << img_static_resized.cols << " img height: " << img_static_resized.rows << endl;
	vector<Point> maxRect = getMaxRect(img_static_resized);
	img_static_warped = warpBoard(img_static_resized, maxRect, 500, 500);
	//img_full_static_warped = warpBoard(img_full_static_resized, maxRect, 500, 500);
	img_static_cannyed = cannyBoard(img_static_warped);
	getHoughLines(img_static_cannyed, horizontalLines, verticalLines);
	
	getIntersections(horizontalLines, verticalLines, intersections);
	//drawIntersections(img_static_warped, intersections);
	getBoardFields(intersections, boardFields);
	
	
	getFieldCornerPoints(intersections, topLeftPoints, bottomRightPoints);

	

	drawLines(img_static_warped, horizontalLines, verticalLines);
	//drawIntersections(img_static_warped, topLeftPoints);
	drawIntersections(img_static_warped, boardFields);
	//drawIntersections(img_full_static_warped, boardFields);
	drawRect(img_static_resized, maxRect);
	imshow("static keypoints", img_static_resized);
	//imshow("static cannyed", img_static_cannyed);
	imshow("static warped", img_static_warped);
	//imshow("full static warped", img_full_static_warped);
	//imshow("static houghed", img_static_houghed);

	Mat colored_move_img;
	img_full_static_warped.copyTo(colored_move_img);


	vector<Mat> move_images = {};
	Mat first_img, second_img, first_img_resized, second_img_resized, first_img_warped, second_img_warped;
	for (int imageIndex = 0; imageIndex < 20; imageIndex++) {
		Mat second_img_colored;
		first_img = imread("Ressources/game/Chessgame_" + to_string(imageIndex) + ".jpg");
		second_img = imread("Ressources/game/Chessgame_" + to_string(imageIndex + 1) + ".jpg");
		resize(first_img, first_img_resized, { 600, 400 });
		resize(second_img, second_img_resized, { 600, 400 });
		first_img_warped = warpBoard(first_img_resized, maxRect, 500, 500);
		second_img_warped = warpBoard(second_img_resized, maxRect, 500, 500);
		

		getMeanFieldColors(first_img_warped, second_img_warped, topLeftPoints, bottomRightPoints, meanColors);

		//cout << "Mean Colors" << meanColors << endl;

		second_img_warped.copyTo(second_img_colored);
		if (meanColors.size() == 2) {
			if (chessFields[meanColors[0].x][1] != "Empty" && chessFields[meanColors[1].x][1] == "Empty") {
				arrowedLine(second_img_colored, boardFields[meanColors[0].x], boardFields[meanColors[1].x], Scalar(0, 250, 250), 3);
				cout << chessFields[meanColors[0].x][0] << " moves to " << chessFields[meanColors[1].x][0] << endl;
				chessFields[meanColors[1].x][1] = chessFields[meanColors[0].x][1];
				chessFields[meanColors[0].x][1] = "Empty";
			}
			else if (chessFields[meanColors[0].x][1] == "Empty" && chessFields[meanColors[1].x][1] != "Empty") {
				arrowedLine(second_img_colored, boardFields[meanColors[1].x], boardFields[meanColors[0].x], Scalar(0, 250, 250), 3);
				cout << chessFields[meanColors[1].x][0] << " moves to " << chessFields[meanColors[0].x][0] << endl;
				chessFields[meanColors[0].x][1] = chessFields[meanColors[1].x][1];
				chessFields[meanColors[1].x][1] = "Empty";
			}
			else if (chessFields[meanColors[0].x][1] != "Empty" && chessFields[meanColors[1].x][1] != "Empty") {
				if (whitesTurn && chessFields[meanColors[1].x][1].compare(0, 5, "White")) {
					arrowedLine(second_img_colored, boardFields[meanColors[0].x], boardFields[meanColors[1].x], Scalar(0, 250, 250), 3);
					cout << chessFields[meanColors[0].x][0] << " moves to " << chessFields[meanColors[1].x][0] << endl;
					chessFields[meanColors[1].x][1] = chessFields[meanColors[0].x][1];
					chessFields[meanColors[0].x][1] = "Empty";
				}
				else if (!whitesTurn && chessFields[meanColors[1].x][1].compare(0, 5, "Black")) {
					arrowedLine(second_img_colored, boardFields[meanColors[0].x], boardFields[meanColors[1].x], Scalar(0, 250, 250), 3);
					cout << chessFields[meanColors[0].x][0] << " moves to " << chessFields[meanColors[1].x][0] << endl;
					chessFields[meanColors[1].x][1] = chessFields[meanColors[0].x][1];
					chessFields[meanColors[0].x][1] = "Empty";
				}
				else {
					arrowedLine(second_img_colored, boardFields[meanColors[1].x], boardFields[meanColors[0].x], Scalar(0, 250, 250), 3);
					cout << chessFields[meanColors[1].x][0] << " moves to " << chessFields[meanColors[0].x][0] << endl;
					chessFields[meanColors[0].x][1] = chessFields[meanColors[1].x][1];
					chessFields[meanColors[1].x][1] = "Empty";
				}
			}
			else {
			}
		}
		else if (meanColors.size() == 4) {
			cout << "Castles" << endl;
		}

			/*
			if (whitesTurn && (meanColors[i].y < meanColors[i + 1].y) || !whitesTurn && (meanColors[i + 1].y < meanColors[i].y)) {
				arrowedLine(second_img_colored, boardFields[meanColors[i].x], boardFields[meanColors[i + 1].x], Scalar(0, 250, 250), 3);
				cout << chessFields[meanColors[i].x] << " moves to " << chessFields[meanColors[i + 1].x] << endl;
			}
			else {
				arrowedLine(second_img_colored, boardFields[meanColors[i + 1].x], boardFields[meanColors[i].x], Scalar(0, 250, 250), 3);
				cout << chessFields[meanColors[i + 1].x] << " moves to " << chessFields[meanColors[i].x] << endl;
			}*/
		whitesTurn = !whitesTurn;


		move_images.emplace_back(second_img_colored);
		//imshow("first image nr" + to_string(imageIndex), second_img_colored);
	}
	
	imshow("show moves", makeCanvas(move_images, 1000, 4));

	


	createTrackbar("Min Canny", WINDOW, &sliderMinCanny, 250, on_trackbar_min_canny);
	createTrackbar("Max Canny", WINDOW, &sliderMaxCanny, 250, on_trackbar_max_canny);
	createTrackbar("Min Area", WINDOW, &sliderMinArea, 2500, on_trackbar_min_area);
	createTrackbar("Max Area", WINDOW, &sliderMaxArea, 2500, on_trackbar_max_area);
	//TODO make Buttons with callbacks createButton("Recalculate Board", )

	while (waitKey(30) != 'q') {
		//cap.read(img);
		//img_scanned = contourBoard(cannyBoard(img));
		//imshow("Image", img);
		//imshow(WINDOW, img_scanned);
		waitKey(1);
	}

	return 0;
}

vector<Point> getMaxRect(Mat img) {
	Mat cannyed;

	cannyed = cannyBoard(img);

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	
	findContours(cannyed, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	vector<vector<Point>> contourPoly(contours.size());
	vector<Rect> boundaryRect(contours.size());
	Point originPoint(0, 0);

	int maxArea = 0;
	vector<Point> maxRect;

	for (int i = 0; i < contours.size(); i++) {
		int area = contourArea(contours[i]);

		if (area > 500) {
			float perimeter = arcLength(contours[i], true);
			approxPolyDP(contours[i], contourPoly[i], 0.02 * perimeter, true);
			if (area > maxArea && contourPoly[i].size() == 4) {
				maxRect = contourPoly[i];
				maxArea = area;
			}
			
		}
	}

	//reorder result points
	vector<Point> result;
	vector<int> coordinateSums, coordinateDiffs;

	for (int i = 0; i < 4; i++) {
		coordinateSums.push_back(maxRect[i].x + maxRect[i].y);
		coordinateDiffs.push_back(maxRect[i].x - maxRect[i].y);
	}

	result.push_back(maxRect[min_element(coordinateSums.begin(), coordinateSums.end()) - coordinateSums.begin()]);
	result.push_back(maxRect[max_element(coordinateDiffs.begin(), coordinateDiffs.end()) - coordinateDiffs.begin()]);
	result.push_back(maxRect[max_element(coordinateSums.begin(), coordinateSums.end()) - coordinateSums.begin()]);
	result.push_back(maxRect[min_element(coordinateDiffs.begin(), coordinateDiffs.end()) - coordinateDiffs.begin()]);

	return result;
}

void drawRect(Mat img, vector<Point> points) {
	vector<vector<Point>> maxRectArray = { points };
	drawContours(img, maxRectArray, 0, Scalar(0, 255, 0), 2);

	Scalar red = Scalar(0, 0, 255);

	for (int i = 0; i < points.size(); i++) {
		circle(img, points[i], 5, red, FILLED);
		putText(img, to_string(i), points[i], FONT_HERSHEY_PLAIN, 2, red, 2);
	}
}

void drawLines(Mat img, vector<vector<Point>>& horizontalLines, vector<vector<Point>>& verticalLines) {

	for (int i = 0; i < horizontalLines.size(); i++) {
		line(img, horizontalLines[i][0], horizontalLines[i][1], Scalar(0, 255, 0), 1);
	}

	for (int i = 0; i < verticalLines.size(); i++) {
		line(img, verticalLines[i][0], verticalLines[i][1], Scalar(0, 255, 0), 1);
	}
}

void getIntersections(const vector<vector<Point>> horizontalLines, const vector<vector<Point>> verticalLines, vector<Point>& intersections) {
	intersections = {};
	for (int vertical = 0; vertical < verticalLines.size(); vertical++) {
		for (int horizontal = 0; horizontal < horizontalLines.size(); horizontal++) {
			int x = horizontalLines[horizontal][0].x;
			int y = verticalLines[vertical][1].y;
			intersections.push_back({ x,y });
		}
	}
}

void drawIntersections(Mat img, const vector<Point> intersections, Scalar color) {

	for (int i = 0; i < intersections.size(); i++) {
		//cout << "Intersection " << i << ": " << intersections[i] << endl;
		circle(img, intersections[i], 5, color, FILLED);
		putText(img, to_string(i), intersections[i], FONT_HERSHEY_PLAIN, 1, color, 1);
	}
}

void getBoardFields(vector<Point> intersections, vector<Point>& boardFields) {
	int offset = sqrt(intersections.size()) + 1;
	Point topLeftPoint;
	Point bottomRightPoint;
	boardFields = {};
	int counter = 0;
	for (int i = 0; i < intersections.size() - offset; i++) {
		if (counter < offset - 2) {
			topLeftPoint = intersections[i];
			bottomRightPoint = intersections[i + offset];
			boardFields.push_back({ topLeftPoint.x + cvRound((bottomRightPoint.x - topLeftPoint.x) / 2), topLeftPoint.y + cvRound((bottomRightPoint.y - topLeftPoint.y) / 2) });
			counter++;
		}
		else {
			counter = 0;
		}
	}
}

void getFieldCornerPoints(const vector<Point> intersections, vector<Point>& topLeftPoints, vector<Point>& bottomRightPoints) {
	int offset = sqrt(intersections.size()) + 1;
	topLeftPoints = {};
	bottomRightPoints = {};
	int counter = 0;
	for (int i = 0; i < intersections.size() - offset; i++) {
		if (counter < offset - 2) {
			topLeftPoints.push_back(intersections[i]);
			bottomRightPoints.push_back(intersections[i + offset]);
			counter++;
		}
		else {
			counter = 0;
		}
	}
}

void colorField(const Mat board_img, vector<Point>& topLeftPoints, vector<Point>& bottomRightPoints, int index, Scalar color, Mat& mask) {
	double alpha = 0.3;

	board_img.copyTo(mask);

	rectangle(mask, Rect(topLeftPoints[index], bottomRightPoints[index]), color, -1);
	addWeighted(mask, alpha, board_img, 1 - alpha, 0, board_img);
}

void getMeanFieldColors(const Mat first_img, const Mat second_img, vector<Point>& topLeftPoints, vector<Point>& bottomRightPoints, vector<Point>& meanColors) {

	meanColors = {};
	Mat colored_second_img;
	second_img.copyTo(colored_second_img);
	Mat first_img_gray;
	Mat second_img_gray;
	cvtColor(first_img, first_img_gray, COLOR_BGR2GRAY);
	cvtColor(second_img, second_img_gray, COLOR_BGR2GRAY);
	int pixelWidth;
	int pixelHeight;
	Point topLeft;
	Point bottomRight;
	int fieldSizeX;
	int fieldSizeY;
	int differenceField;
	int differencePixel;
	Mat diff;
	Mat thresh;
	absdiff(first_img_gray, second_img_gray, diff);
	threshold(diff, thresh, 10, 255, 0);
	imshow("static source", first_img);
	imshow("static diff", diff);
	imshow("static thresh", thresh);

	/*
	for (int i = 0; i < topLeftPoints.size(); i++) {
		topLeft = topLeftPoints[i];
		bottomRight = bottomRightPoints[i];
		//cout << "Field Nr." << i << " Coords: (" << topLeft << "," << bottomRight << ")" << endl;
		differenceColor = 0;
		
		
		for (int y = topLeft.y + EPSILON_BRIGHTNESS; y < bottomRight.y - EPSILON_BRIGHTNESS; y++) {
			for (int x = topLeft.x + EPSILON_BRIGHTNESS; x < bottomRight.x - EPSILON_BRIGHTNESS; x++) {
				differenceColor += (int)abs(second_img_gray.at<uchar>(x, y) - first_img_gray.at<uchar>(x, y));
				//cout << "Field (" << x << "," << y << ") Color Difference:" << differenceColor << endl;
			}
		}
		differenceColor /= (bottomRight.x - topLeft.x) * (bottomRight.y - topLeft.y);
		meanColors.push_back(differenceColor);
		if (abs(differenceColor) > EPSILON_BRIGHTNESS) {
			cout << "Field Nr." << i << " Color Difference:" << differenceColor << endl;
		}
	}*/

	for (int i = 0; i < topLeftPoints.size(); i++) {
		topLeft = topLeftPoints[i];
		bottomRight = bottomRightPoints[i];
		//cout << "Field Nr." << i << " Coords: (" << topLeft << "," << bottomRight << ")" << endl;
		differenceField = 0;


		for (int y = topLeft.y; y < bottomRight.y; y++) {
			for (int x = topLeft.x; x < bottomRight.x; x++) {
				differencePixel = second_img_gray.at<uchar>(y, x) - first_img_gray.at<uchar>(y, x);
				if (abs(differencePixel) > EPSILON_BRIGHTNESS) {
					differenceField += abs(differencePixel); //thresh.at<uchar>(y,x);
				}
				
				//cout << "Field (" << x << "," << y << ") Color Difference:" << differenceColor << endl;
			}
		}
		differenceField /= (bottomRight.x - topLeft.x) * (bottomRight.y - topLeft.y);
		
		if (abs(differenceField) > 0) {
			meanColors.push_back(Point(i, differenceField));
			//cout << "Field Nr." << i << " Color Difference:" << differenceField << endl;
		}
	}
	
}

Mat warpBoard(Mat img, vector<Point> points, float width, float height) {
	Mat warped;
	Point2f src[4] = {points[0], points[1], points[2], points[3]};
	Point2f dst[4] = { {0.0f, 0.0f}, {width, 0.0f}, {width, height}, {0.0f, height} };

	Mat matrix = getPerspectiveTransform(src, dst);
	warpPerspective(img, warped, matrix, Point(width, height));

	return warped;
}

//Draw blobs on the board
void recognizeBoard(const Mat img) {
	SimpleBlobDetector detector;
	vector<KeyPoint> keypoints;
	Mat img_with_keypoints;

	//Detect blobs
	detector.detect(img, keypoints);

	//Draw detected blobs as green circles
	drawKeypoints(img, keypoints, img_with_keypoints, Scalar(0, 255, 0), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

	//Show blobs
	imshow("keypoints", img_with_keypoints);
	waitKey(0);
}

Mat cannyBoard(const Mat img) {
	Mat gray, blur, cannyed, dilated, eroded;
	const Mat kernel = getStructuringElement(MORPH_RECT, Size(5, 5));
	

	cvtColor(img, gray, COLOR_BGR2GRAY);
	GaussianBlur(gray, blur, Size(7, 7), 0, 0);
	Canny(blur, cannyed, threshMinCanny, threshMaxCanny);
	dilate(cannyed, dilated, kernel);
	erode(dilated, eroded, kernel);

	return eroded;
}

Mat contourBoard(const Mat img) {
	Mat contoured;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	findContours(img, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
	contoured = Mat::zeros(img.size(), CV_8UC3);
	for (size_t i = 0; i < contours.size(); i++)
	{
		int area = contourArea(contours[i]);
		//cout << area << endl;
		Scalar color = Scalar(255, 0, 0);
		if (area > threshMinArea && area < threshMaxArea) {
			drawContours(contoured, contours, (int)i, color, 2, LINE_8, hierarchy, 0);
		}
	}

	return contoured;
}

void getHoughLines(const Mat img, vector<vector<Point>> &horizontalLines, vector<vector<Point>> &verticalLines) {
	vector<Vec2f> lines;
	horizontalLines = {};
	verticalLines = {};

	HoughLines(img, lines, 1, CV_PI / 180, 120, 0, 0);

	Mat houghed(img.size(), CV_8UC1, Scalar(0, 0, 0));

	for (size_t i = 0; i < lines.size(); i++) {
		float rho = lines[i][0], theta = lines[i][1];
		//cout << "Rho:" << rho << endl;
		//cout << "Theta:" << theta << endl;
		//cout << "PI/2:" << CV_PI / 2 << endl;

		// check which houghLines have 0� or 180� angle
		if (theta < EPSILON) {
			Point pt1, pt2;
			double a = cos(theta), b = sin(theta);
			double x0 = a * rho, y0 = b * rho;
			pt1.x = cvRound(x0 + 1000 * (-b));
			pt1.y = 0;
			pt2.x = pt1.x;
			pt2.y = WIDTH;
			//cout << "Horizontal:" << pt1 << "," << pt2 << endl;
			horizontalLines.push_back({ pt1, pt2 });
		}
		else if (theta < EPSILON || abs(theta - CV_PI / 2) < EPSILON) {
			Point pt1, pt2;
			double a = cos(theta), b = sin(theta);
			double x0 = a * rho, y0 = b * rho;
			pt1.x = 0;
			pt1.y = cvRound(y0 + 1000 * (a));
			pt2.x = HEIGHT;
			pt2.y = pt1.y;
			//cout << "Vertical:" << pt1 << "," << pt2 << endl;
			verticalLines.push_back({ pt1, pt2 });
		}
	}

	//sort lines
	sort(horizontalLines.begin(), horizontalLines.end(), [](const vector<Point> a, const vector<Point> b) { return a[0].x < b[0].x; });
	sort(verticalLines.begin(), verticalLines.end(), [](const vector<Point> a, const vector<Point> b) { return a[0].y < b[0].y; });

	//remove close horizontal lines
	if (horizontalLines[0][0].x < EPSILON_LINES) {
		horizontalLines.erase(horizontalLines.begin());
	}
	if ((WIDTH - horizontalLines[horizontalLines.size() - 1][0].x) < EPSILON_LINES) {
		horizontalLines.pop_back();
	}
	for (int i = 0; i < horizontalLines.size() - 1; i++) {
		if ((horizontalLines[i + 1][0].x - horizontalLines[i][0].x) < EPSILON_LINES) {
			horizontalLines.erase(horizontalLines.begin() + i);
		}
	}

	//remove close vertical lines
	if (verticalLines[0][0].y < EPSILON_LINES) {
		verticalLines.erase(verticalLines.begin());
	}
	if ((HEIGHT - verticalLines[horizontalLines.size() - 1][0].y) < EPSILON_LINES) {
		verticalLines.pop_back();
	}
	for (int i = 0; i < verticalLines.size() - 1; i++) {
		if ((verticalLines[i + 1][0].y - verticalLines[i][0].y) < EPSILON_LINES) {
			verticalLines.erase(verticalLines.begin() + i);
		}
	}
}

Mat makeCanvas(std::vector<cv::Mat>& vecMat, int windowHeight, int nRows) {
	int N = vecMat.size();
	nRows = nRows > N ? N : nRows;
	int edgeThickness = 10;
	int imagesPerRow = ceil(double(N) / nRows);
	int resizeHeight = floor(2.0 * ((floor(double(windowHeight - edgeThickness) / nRows)) / 2.0)) - edgeThickness;
	int maxRowLength = 0;

	std::vector<int> resizeWidth;
	for (int i = 0; i < N;) {
		int thisRowLen = 0;
		for (int k = 0; k < imagesPerRow; k++) {
			double aspectRatio = double(vecMat[i].cols) / vecMat[i].rows;
			int temp = int(ceil(resizeHeight * aspectRatio));
			resizeWidth.push_back(temp);
			thisRowLen += temp;
			if (++i == N) break;
		}
		if ((thisRowLen + edgeThickness * (imagesPerRow + 1)) > maxRowLength) {
			maxRowLength = thisRowLen + edgeThickness * (imagesPerRow + 1);
		}
	}
	int windowWidth = maxRowLength;
	cv::Mat canvasImage(windowHeight, windowWidth, CV_8UC3, Scalar(0, 0, 0));

	for (int k = 0, i = 0; i < nRows; i++) {
		int y = i * resizeHeight + (i + 1) * edgeThickness;
		int x_end = edgeThickness;
		for (int j = 0; j < imagesPerRow && k < N; k++, j++) {
			int x = x_end;
			cv::Rect roi(x, y, resizeWidth[k], resizeHeight);
			cv::Size s = canvasImage(roi).size();
			// change the number of channels to three
			cv::Mat target_ROI(s, CV_8UC3);
			if (vecMat[k].channels() != canvasImage.channels()) {
				if (vecMat[k].channels() == 1) {
					cv::cvtColor(vecMat[k], target_ROI, CV_GRAY2BGR);
				}
			}
			else {
				vecMat[k].copyTo(target_ROI);
			}
			cv::resize(target_ROI, target_ROI, s);
			if (target_ROI.type() != canvasImage.type()) {
				target_ROI.convertTo(target_ROI, canvasImage.type());
			}
			target_ROI.copyTo(canvasImage(roi));
			x_end += resizeWidth[k] + edgeThickness;
		}
	}
	return canvasImage;
}