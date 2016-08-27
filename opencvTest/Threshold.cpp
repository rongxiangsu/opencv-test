#include "Threshold.h"


CThreshold::CThreshold(string infile)
{
	this->fileName = infile;
	out_fileName = outputFileName(infile, "_threshold.jpg");
}


CThreshold::~CThreshold()
{
}

void CThreshold::thresholdImg()
{
	IplImage* src = cvLoadImage(fileName.c_str(), CV_LOAD_IMAGE_GRAYSCALE);
	cvThreshold(src, src, IMG_THRESHOLD, 255, CV_THRESH_BINARY);
	cvSaveImage(out_fileName.c_str(), src);
	cvReleaseImage(&src);
	return;
}

