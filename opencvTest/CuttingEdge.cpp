#include "CuttingEdge.h"


CCuttingEdge::CCuttingEdge(string fileName)
{
	this->fileName = fileName;
	out_fileName = outputFileName(fileName, "_cut.jpg");
}


CCuttingEdge::~CCuttingEdge()
{
}

//3. �ü��������ͼ��ȥ���ڱߣ�ֻ�������õĲ���
void CCuttingEdge::clipOriginImage()
{
	cout << "Cutting edge..." << endl;
	int* startPoint = getLeftTopPoint(fileName);
	int* endPoint = getRightBottomPoint(fileName);
	int startx = startPoint[0] - 15;
	int starty = startPoint[1] - 180;//���Ͻǵ������ϲ���Ҫ����һЩ���أ���Ϊ��һ��֤������ҲҪʶ��
	int endx = endPoint[0] + 15;
	int endy = endPoint[1] + 40;//֤����������ܳ���ӡˢƫ�Ƶ���������Զ�ü�һ����
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