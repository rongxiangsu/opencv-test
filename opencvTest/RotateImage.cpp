#include "RotateImage.h"


CRotateImage::CRotateImage(string fileName)
{
	this->fileName = fileName;
	out_fileName = outputFileName(fileName, "_rotate.jpg");
}


CRotateImage::~CRotateImage()
{
}

IplImage* CRotateImage::rotateImage(double degree)
{
	IplImage* img = cvLoadImage(this->fileName.c_str());
	double a = sin(degree), b = cos(degree);
	int width = img->width, height = img->height;
	int width_rotate = int(height * fabs(a) + width * fabs(b));
	int height_rotate = int(width * fabs(a) + height * fabs(b));
	IplImage* img_rotate = cvCreateImage(cvSize(width_rotate, height_rotate), img->depth, img->nChannels);
	cvZero(img_rotate);
	int tempLength = sqrt((double)width * width + (double)height *height) + 10;
	int tempX = (tempLength + 1) / 2 - width / 2;
	int tempY = (tempLength + 1) / 2 - height / 2;
	IplImage* temp = cvCreateImage(cvSize(tempLength, tempLength), img->depth, img->nChannels);
	cvZero(temp);
	cvSetImageROI(temp, cvRect(tempX, tempY, width, height));
	cvCopy(img, temp, NULL);
	cvResetImageROI(temp);
	float m[6];
	int w = temp->width;
	int h = temp->height;
	m[0] = b;
	m[1] = a;
	m[3] = -m[1];
	m[4] = m[0];
	m[2] = w * 0.5f;
	m[5] = h * 0.5f;
	CvMat M = cvMat(2, 3, CV_32F, m);
	cvGetQuadrangleSubPix(temp, img_rotate, &M);
	cvReleaseImage(&temp);
	cvReleaseImage(&img);
	return img_rotate;
}

//ÇãÐ±½ÃÕý
void CRotateImage::correctImage()
{
	cout << "Correcting image..." << endl;
	int* startPoint = getLeftTopPoint(this->fileName);
	int* endPoint = getRightToppoint(this->fileName);
	double degree = -atan((double)(endPoint[1] - startPoint[1]) / (double)(endPoint[0] - startPoint[0]));
	IplImage* img = cvLoadImage(this->fileName.c_str());
	IplImage* correct_img = rotateImage(degree);
	cvSaveImage(out_fileName.c_str(), correct_img);
	cvReleaseImage(&img);
	cvReleaseImage(&correct_img);
	return;
}