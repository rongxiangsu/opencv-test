#include "CuttingEdge.h"


CCuttingEdge::CCuttingEdge(string fileName)
{
	this->fileName = fileName;
	out_fileName = outputFileName(fileName, "_cut.jpg");
}


CCuttingEdge::~CCuttingEdge()
{
}

//3. 裁剪矫正后的图像，去除黑边，只保留有用的部分
void CCuttingEdge::clipOriginImage()
{
	cout << "Cutting edge..." << endl;
	int* startPoint = getLeftTopPoint(fileName);
	int* endPoint = getRightBottomPoint(fileName);
	int startx = startPoint[0] - 15;
	int starty = startPoint[1] - 180;//左上角点再往上部分要保留一些像素，因为有一串证件编码也要识别
	int endx = endPoint[0] + 15;
	int endy = endPoint[1] + 40;//证件最下面可能出现印刷偏移的情况，所以多裁剪一部分
	//cout << startPoint[0] << "," << startPoint[1] << endl << endPoint[0] << "," << endPoint[1] << endl;
	Range R1;
	R1.start = starty;
	R1.end = endy;
	Range R2;
	R2.start = startx;
	R2.end = endx;
	Mat img = imread(fileName);
	Mat mask = Mat::Mat(img, R1, R2);
	imwrite(out_fileName, mask);
	return;
}