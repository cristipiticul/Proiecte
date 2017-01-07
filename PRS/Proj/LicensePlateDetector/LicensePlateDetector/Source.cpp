#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <windows.h>
#include <tchar.h> 
#include <stdio.h>
#include <strsafe.h>
#pragma comment(lib, "User32.lib")

// EDGE detection configurations
#define WITH_ABS
const int EDGE_PERCENTAGE = 90; // how much % of the image pixels are edge pixels (roughly) -> for binarizing the vertical edges image

// BANDS detection configurations
const int MIN_NUMBER_OF_EDGE_PIXELS_FOR_PLATE = 10;
const double VARIANCE_THRESHOLD_MIN = 20.0;

// REGION LIMITS IMPROVEMENT configurations
const int MAX_VERTICAL_LINE_DX = 6;
const int VERTICAL_LINE_MIN_LENGTH = 10;

// FILTERING REGIONS configurations
const double ASPECT_RATIO_MIN = 0.15;
const double ASPECT_RATIO_MAX = 0.35;
const int WIDTH_MIN = 50;
const int WIDTH_MAX = 300;
const int HEIGHT_MIN = 10;
const int HEIGHT_MAX = 50;

using namespace cv;
using namespace std;

void forEachImageFile(const TCHAR* folder, void(*callback)(const String));
Mat toGrayscale(Mat input);
Mat stretchContrast(Mat input);
Mat detectVerticalEdges(Mat input);

typedef struct {
    int numberOfEdgePixels;
    double meanX;
    double varianceX;
} RowInfo;
void computeRowInfo(Mat edgesImage, int rowIndex, RowInfo *result);

typedef struct {
    int start;
    int end;
} Band;
vector<Band> findBands(Mat edges, RowInfo* rowsInfo);

typedef struct {
    int startX;
    int startY;
    int endX;
    int endY;
} Region;
vector<Region> limitBands(vector<Band> bands, RowInfo* rowsInfo, int cols);

vector<Region> improveRegions(Mat edges, vector<Region> regions);

vector<Region> improveRegions(vector<Region> regions);

vector<Region> filterRegions(vector<Region> regions);

void DisplayErrorBox(LPTSTR lpszFunction);

void detectLicensePlates(const String filename)
{
    Mat image;
    image = imread(filename, IMREAD_COLOR);

    if (!image.data)
    {
        printf("Could not open or find the image %s\n", filename.c_str());
        return;
    }

    namedWindow("Display window", WINDOW_AUTOSIZE);
    imshow("Display window", image);
    waitKey(0);

    Mat grayscale = toGrayscale(image);
    imshow("Display window", grayscale);
    waitKey(0);

    Mat stretchedContrast = stretchContrast(grayscale);
    imshow("Display window", stretchedContrast);
    waitKey(0);

    Mat edges = detectVerticalEdges(stretchedContrast);
    imshow("Display window", edges);
    waitKey(0);

    Mat bandsImage = image.clone();
    RowInfo *rowsInfo = new RowInfo[edges.rows];
    for (int i = 0; i < edges.rows; i++)
    {
        computeRowInfo(edges, i, &rowsInfo[i]);
    }
    vector<Band> bands = findBands(edges, rowsInfo);
    printf("number of bands: %d\n", (int)bands.size());
    for (int i = 0; i < bands.size(); i++)
    {
        printf("%d %d\n", bands[i].start, bands[i].end);
        rectangle(bandsImage, Rect(0, bands[i].start, bandsImage.cols, bands[i].end - bands[i].start), Scalar(0, 255, 0));
    }
    imshow("Display window", bandsImage);
    waitKey(0);

    Mat regionsImage = image.clone();
    vector<Region> regions = limitBands(bands, rowsInfo, image.cols);
    for (int i = 0; i < regions.size(); i++)
    {
        printf("Region: (%3d, %3d) (%3d, %3d)\n", regions[i].startX, regions[i].startY, regions[i].endX, regions[i].endY);
        rectangle(regionsImage, Rect(Point(regions[i].startX, regions[i].startY), Point(regions[i].endX, regions[i].endY)), Scalar(0, 255, 0));
    }
    imshow("Display window", regionsImage);
    waitKey(0);

    Mat improvedRegionsImage = image.clone();
    vector<Region> improvedRegions = improveRegions(edges, regions);
    for (int i = 0; i < improvedRegions.size(); i++)
    {
        printf("Region: (%3d, %3d) (%3d, %3d)\n", improvedRegions[i].startX, improvedRegions[i].startY, improvedRegions[i].endX, improvedRegions[i].endY);
        rectangle(improvedRegionsImage, Rect(Point(improvedRegions[i].startX, improvedRegions[i].startY), Point(improvedRegions[i].endX, improvedRegions[i].endY)), Scalar(0, 255, 0));
    }
    imshow("Display window", improvedRegionsImage);
    waitKey(0);

    Mat filteredRegionsImage = image.clone();
    vector<Region> filteredRegions = filterRegions(improvedRegions);
    for (int i = 0; i < filteredRegions.size(); i++)
    {
        rectangle(filteredRegionsImage, Rect(Point(filteredRegions[i].startX, filteredRegions[i].startY), Point(filteredRegions[i].endX, filteredRegions[i].endY)), Scalar(0, 255, 0));
    }
    imshow("Display window", filteredRegionsImage);
    waitKey(0);

    delete[] rowsInfo;
}

Mat toGrayscale(Mat input)
{
    Mat result(input.rows, input.cols, CV_8UC1);
    for (int i = 0; i < input.rows; i++)
    {
        for (int j = 0; j < input.cols; j++)
        {
            Vec3b color = input.at<Vec3b>(i, j);
            result.at<uchar>(i, j) = (uchar)(((int)color[2] * 59 + (int)color[1] * 30 + (int)color[0] * 11) / 100);
        }
    }
    return result;
}

int* computeHistogram(Mat image)
{
    int *hist = new int[256];
    memset(hist, 0, sizeof(int[256]));

    for (int i = 0; i < image.rows; i++)
    {
        for (int j = 0; j < image.cols; j++)
        {
            hist[image.at<uchar>(i, j)]++;
        }
    }

    return hist;
}

Mat stretchContrast(Mat input)
{
    Mat result(input.rows, input.cols, CV_8UC1);
    uchar newColor[256];
    int* hist = computeHistogram(input);

    int pixelsSoFar = 0;
    for (int i = 0; i < 256; i++)
    {
        pixelsSoFar += hist[i];
        newColor[i] = pixelsSoFar * 255 / (input.rows * input.cols);
    }
    delete[] hist;

    for (int i = 0; i < input.rows; i++)
    {
        for (int j = 0; j < input.cols; j++)
        {
            result.at<uchar>(i, j) = newColor[input.at<uchar>(i, j)];
        }
    }

    return result;
}

uchar normalize(int value)
{
    if (value > 255) return 255;
    if (value < 0) return 0;
    return value;
}

Mat detectVerticalEdges(Mat input)
{
    Mat result(input.rows, input.cols, CV_8UC1);

    for (int i = 0; i < input.rows; i++)
    {
        for (int j = 0; j < input.cols; j++)
        {
            if (i == 0 || j == 0 || i == input.rows - 1 || j == input.cols - 1)
            {
                result.at<uchar>(i, j) = 0;
                continue;
            }
            // sobel operator
            result.at<uchar>(i, j) = normalize(
#ifdef WITH_ABS
                    abs(
#endif
                -((int)input.at<uchar>(i - 1, j - 1))
                - 2 * input.at<uchar>(i, j - 1)
                - input.at<uchar>(i + 1, j - 1)
                + input.at<uchar>(i - 1, j + 1)
                + 2 * input.at<uchar>(i, j + 1)
                + input.at<uchar>(i + 1, j + 1)
#ifdef WITH_ABS
                    )
#endif
            );
        }
    }

    int* hist = computeHistogram(input);
    int pixelsSoFar = 0;
    int threshold = 0;
    for (int i = 0; i < 256; i++)
    {
        pixelsSoFar += hist[i];
        if (pixelsSoFar > (input.rows * input.cols) * EDGE_PERCENTAGE / 100)
        {
            threshold = i;
            break;
        }
    }
    delete[] hist;
    printf("threshold: %d\n", threshold);

    for (int i = 0; i < result.rows; i++)
    {
        for (int j = 0; j < result.cols; j++)
        {
            result.at<uchar>(i, j) = (result.at<uchar>(i, j) < threshold ? 0 : 255);
        }
    }

    return result;
}

void computeRowInfo(Mat edgesImage, int rowIndex, RowInfo *result)
{
    result->numberOfEdgePixels = 0;
    result->meanX = 0.0;
    result->varianceX = 0.0;
    for (int j = 0; j < edgesImage.cols; j++)
    {
        if (edgesImage.at<uchar>(rowIndex, j) == 255)
        {
            result->numberOfEdgePixels++;
            result->meanX += j;
        }
    }
    if (result->numberOfEdgePixels == 0)
    {
        result->meanX = 0.0;
        return;
    }
    result->meanX /= result->numberOfEdgePixels;
    for (int j = 0; j < edgesImage.cols; j++)
    {
        if (edgesImage.at<uchar>(rowIndex, j) == 255)
        {
            result->varianceX += ((j - result->meanX) * (j - result->meanX));
        }
    }
    result->varianceX = sqrt(result->varianceX) / result->numberOfEdgePixels;
}

vector<Band> findBands(Mat edges, RowInfo* rowsInfo)
{
    vector<Band> bands;
    for (int i = 0; i < edges.rows; i++)
    {
        if (rowsInfo[i].numberOfEdgePixels >= MIN_NUMBER_OF_EDGE_PIXELS_FOR_PLATE && rowsInfo[i].varianceX >= VARIANCE_THRESHOLD_MIN)
        {
            int bandEnd;
            for (bandEnd = i + 1; bandEnd < edges.rows; bandEnd++)
            {
                if (rowsInfo[bandEnd].numberOfEdgePixels < MIN_NUMBER_OF_EDGE_PIXELS_FOR_PLATE || rowsInfo[i].varianceX < VARIANCE_THRESHOLD_MIN)
                {
                    break;
                }
            }
            bandEnd--;

            Band b;
            b.start = i;
            b.end = bandEnd;
            bands.push_back(b);

            i = bandEnd;
        }
    }
    return bands;
}

vector<Region> limitBands(vector<Band> bands, RowInfo* rowsInfo, int cols)
{
    vector<Region> result;
    for (int i = 0; i < bands.size(); i++)
    {
        // for each region, find mean_min, mean_max and variance_max
        Region region;
        region.startY = bands[i].start;
        region.endY = bands[i].end;
        double meanMin = rowsInfo[bands[i].start].meanX;
        double meanMax = rowsInfo[bands[i].start].meanX;
        double varianceMax = rowsInfo[bands[i].start].varianceX;
        for (int row = bands[i].start + 1; row < bands[i].end; row++)
        {
            if (rowsInfo[row].meanX > meanMax)
            {
                meanMax = rowsInfo[row].meanX;
            }
            if (rowsInfo[row].meanX < meanMin)
            {
                meanMin = rowsInfo[row].meanX;
            }
            if (rowsInfo[row].varianceX > varianceMax)
            {
                varianceMax = rowsInfo[row].varianceX;
            }
        }
        // the limits on the X axis will be mean_min - variance_max and mean_max + variance_max
        region.startX = (int)(meanMin - varianceMax + 0.5);
        if (region.startX < 0)
        {
            region.startX = 0;
        }
        region.endX = (int)(meanMax + varianceMax + 0.5);
        if (region.endX > cols)
        {
            region.endX = cols;
        }
        result.push_back(region);
    }
    return result;
}

// Find the left-most and right-most proeminent vertical line
vector<Region> improveRegions(Mat edges, vector<Region> regions)
{
    vector<Region> result;
    for (int i = 0; i < regions.size(); i++)
    {
        int colMin = regions[i].endX;
        int colMax = regions[i].startX;
        int rowMin = regions[i].endY;
        int rowMax = regions[i].startY;
        for (int row = regions[i].startY; row < regions[i].endY; row++)
        {
            for (int col = regions[i].startX; col < regions[i].endX; col++)
            {
                if (edges.at<uchar>(row, col) == 255)
                {
                    //find the biggest vertical line starting with (row, col)
                    int row2 = row + 1;
                    int col2 = col;
                    for (; row2 < regions[i].endY; row2++)
                    {
                        if (edges.at<uchar>(row2, col2) == 255)
                        {
                        }
                        else if (edges.at<uchar>(row2, col2 - 1) == 255 && abs(col2 - 1 - col) < MAX_VERTICAL_LINE_DX)
                        {
                            col2 = col2 - 1;
                        }
                        else if (edges.at<uchar>(row2, col2 + 1) == 255 && abs(col2 + 1 - col) < MAX_VERTICAL_LINE_DX)
                        {
                            col2 = col2 + 1;
                        }
                        else
                        {
                            row2--;
                            break;
                        }
                    }
                    if (row2 - row > VERTICAL_LINE_MIN_LENGTH)
                    {
                        if (col < colMin)
                        {
                            colMin = col;
                        }
                        if (col > colMax)
                        {
                            colMax = col;
                        }
                        if (row < rowMin)
                        {
                            rowMin = row;
                        }
                        if (row2 > rowMax)
                        {
                            rowMax = row2;
                        }
                    }
                }
            }
        }
        if (rowMin < rowMax && colMin < colMax) // vertical edge found
        {
            Region r;
            r.startX = colMin;
            r.endX = colMax;
            r.startY = rowMin;
            r.endY = rowMax;
            result.push_back(r);
        }
    }
    return result;
}

vector<Region> filterRegions(vector<Region> regions)
{
    vector<Region> result;
    for (int i = 0; i < regions.size(); i++)
    {
        int width = regions[i].endX - regions[i].startX;
        int height = regions[i].endY - regions[i].startY;
        double aspectRatio = (double)height / width;
        if (aspectRatio >= ASPECT_RATIO_MIN && aspectRatio <= ASPECT_RATIO_MAX &&
            width >= WIDTH_MIN && width <= WIDTH_MAX &&
            height >= HEIGHT_MIN && height <= HEIGHT_MAX)
        {
            result.push_back(regions[i]);
        }
    }
    return result;
}

int main(int argc, char** argv)
{
    forEachImageFile(TEXT(".\\img"), detectLicensePlates);
    //forEachImageFile(TEXT(".\\img\\difficult_color_more_than_one"), detectLicensePlates);
    return 0;
}

void forEachImageFile(const TCHAR* folder, void(*callback)(const String))
{
    WIN32_FIND_DATA ffd;
    TCHAR szDir[MAX_PATH];
    TCHAR nextDir[MAX_PATH];
    TCHAR fileName[MAX_PATH];
    char fileNameStr[MAX_PATH];
    HANDLE hFind = INVALID_HANDLE_VALUE;
    DWORD dwError = 0;

    // Prepare string for use with FindFile functions.  First, copy the
    // string to a buffer, then append '\*' to the directory name.

    StringCchCopy(szDir, MAX_PATH, folder);
    StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

    // Find the first file in the directory.

    hFind = FindFirstFile(szDir, &ffd);

    if (INVALID_HANDLE_VALUE == hFind)
    {
        DisplayErrorBox(TEXT("FindFirstFile"));
        return;
    }

    // List all the files in the directory with some info about them.

    do
    {
        if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && _tcscmp(ffd.cFileName, TEXT(".")) != 0 && _tcscmp(ffd.cFileName, TEXT("..")) != 0)
        {
            StringCchCopy(nextDir, MAX_PATH, folder);
            StringCchCat(nextDir, MAX_PATH, TEXT("\\"));
            StringCchCat(nextDir, MAX_PATH, ffd.cFileName);
            _tprintf(TEXT("Entering dir %s\n"), nextDir);
            forEachImageFile(nextDir, callback);
        }
        else
        {
            // file. Check if it's an image
            TCHAR* extension = _tcschr(ffd.cFileName, TEXT('.'));
            if (extension != NULL && _tcscmp(extension, TEXT(".jpg")) == 0)
            {
                StringCchCopy(fileName, MAX_PATH, folder);
                StringCchCat(fileName, MAX_PATH, TEXT("\\"));
                StringCchCat(fileName, MAX_PATH, ffd.cFileName);
                wcstombs(fileNameStr, fileName, MAX_PATH);

                callback(fileNameStr);
            }
        }
    } while (FindNextFile(hFind, &ffd) != 0);

    dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES)
    {
        DisplayErrorBox(TEXT("FindFirstFile"));
    }

    FindClose(hFind);
}

void DisplayErrorBox(LPTSTR lpszFunction)
{
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, NULL);

    // Display the error message and clean up

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
    StringCchPrintf((LPTSTR)lpDisplayBuf,
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"),
        lpszFunction, dw, lpMsgBuf);
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}
