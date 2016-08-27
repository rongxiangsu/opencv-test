#include "Base.h"

CBase::CBase()
{
}


CBase::~CBase()
{
}

string CBase::getOutFileName()
{
	return out_fileName;
}

int* CBase::getLeftTopPoint(string oriImgName)
{
	int* temp = new int[2];
	Mat img = imread(oriImgName);
	if (img.empty())
	{
		fprintf(stderr, "Can not load image %s\n", oriImgName);
		return NULL;
	}
	int start_x = 0;
	int start_y = 0;
	int flag = 0;
	int flag2 = 0;
	for (int row = 100; row < 500; row++)
	{
		Vec3b * p1 = img.ptr<Vec3b>(row);
		for (int col = 50; col < 500; col++)
		{
			for (int i = col; i < col + LEFTTOP_PIXELNUM; i++)
			{
				if (p1[i][0] <= LRTB_THRESHOLD && p1[i][1] <= LRTB_THRESHOLD && p1[i][2] <= LRTB_THRESHOLD)
				{
					flag += 1;
				}
				else
				{
					flag = 0;
					break;
				}
			}
			if (flag >= LEFTTOP_PIXELNUM - 5)
			{
				flag2 = 0;
				for (int j = row; j < row + LEFTTOP_PIXELNUM; j++)
				{
					Vec3b * p1 = img.ptr<Vec3b>(j);
					if (p1[col][0] <= LRTB_THRESHOLD && p1[col][1] <= LRTB_THRESHOLD && p1[col][2] <= LRTB_THRESHOLD)
					{
						flag2 += 1;
					}
					else
					{
						flag2 = 0;
						break;
					}
				}
				if (flag2 >= LEFTTOP_PIXELNUM - 5)
				{
					start_y = row;
					start_x = col;
					flag2 = 0;
					flag = 0;
					break;
				}
			}
			flag = 0;
		}
		if (start_x != 0 || start_y != 0)
		{
			break;
		}
	}
	temp[0] = start_x;
	temp[1] = start_y;
	return temp;
}

//获得图像右下角点裁剪边界点
int* CBase::getRightBottomPoint(string oriImgName)
{
	int* temp = new int[2];
	Mat img = imread(oriImgName);
	if (img.empty())
	{
		fprintf(stderr, "Can not load image %s\n", oriImgName);
		return NULL;
	}
	int end_x = 0;
	int end_y = 0;
	int flag = 0;
	int flag2 = 0;
	for (int row = img.rows - 60; row > img.rows - 500; row--)
	{
		Vec3b * p1 = img.ptr<Vec3b>(row);
		for (int col = img.cols - 40; col > img.cols - 500; col--)
		{
			for (int i = col; i > col - RIGHTBOTTOM_PIXELNUM; i--)
			{
				if (p1[i][0] <= LRTB_THRESHOLD && p1[i][1] <= LRTB_THRESHOLD && p1[i][2] <= LRTB_THRESHOLD)
				{
					flag += 1;
				}
				else
				{
					flag = 0;
					break;
				}
			}
			if (flag >= RIGHTBOTTOM_PIXELNUM - 5)
			{
				flag2 = 0;
				for (int j = row; j > row - RIGHTBOTTOM_PIXELNUM; j--)
				{
					Vec3b * p1 = img.ptr<Vec3b>(j);
					if (p1[col][0] <= LRTB_THRESHOLD && p1[col][1] <= LRTB_THRESHOLD && p1[col][2] <= LRTB_THRESHOLD)
					{
						flag2 += 1;
					}
					else
					{
						flag2 = 0;
						break;
					}
				}
				if (flag2 >= RIGHTBOTTOM_PIXELNUM - 5)
				{
					end_y = row;
					end_x = col;
					flag2 = 0;
					flag = 0;
					break;
				}
			}
			flag = 0;
		}
		if (end_x != 0 || end_y != 0)
		{
			break;
		}
	}
	temp[0] = end_x;
	temp[1] = end_y;
	return temp;
}

//获得图像右上角点，用于计算矫正度数
int* CBase::getRightToppoint(string oriImgName)
{
	int* temp = new int[2];
	Mat img = imread(oriImgName);
	if (img.empty())
	{
		fprintf(stderr, "Can not load image %s\n", oriImgName);
		return NULL;
	}
	int start_x = 0;
	int start_y = 0;
	int flag = 0;
	int flag2 = 0;
	for (int row = 100; row < 500; row++)
	{
		Vec3b * p1 = img.ptr<Vec3b>(row);
		for (int col = img.cols - 40; col > img.cols - 500; col--)
		{
			for (int i = col; i > col - RIGHTTOP_PIXELNUM; i--)
			{
				if (p1[i][0] <= LRTB_THRESHOLD && p1[i][1] <= LRTB_THRESHOLD && p1[i][2] <= LRTB_THRESHOLD)
				{
					flag += 1;
				}
				else
				{
					flag = 0;
					break;
				}
			}
			if (flag >= RIGHTTOP_PIXELNUM - 5)
			{
				flag2 = 0;
				for (int j = row; j < row + RIGHTTOP_PIXELNUM; j++)
				{
					Vec3b * p1 = img.ptr<Vec3b>(j);
					if (p1[col][0] <= LRTB_THRESHOLD && p1[col][1] <= LRTB_THRESHOLD && p1[col][2] <= LRTB_THRESHOLD)
					{
						flag2 += 1;
					}
					else
					{
						flag2 = 0;
						break;
					}
				}
				if (flag2 >= RIGHTTOP_PIXELNUM - 5)
				{
					start_y = row;
					start_x = col;
					flag2 = 0;
					flag = 0;
					break;
				}
			}
			flag = 0;
		}
		if (start_x != 0 || start_y != 0)
		{
			break;
		}
	}
	temp[0] = start_x;
	temp[1] = start_y;
	return temp;
}

//获得裁剪后图像的左上角点，由于裁边后的图像的位置发生变化，所以原先的方法不适用，要改变一些阈值
int* CBase::getLeftTopPoint_afterClip(string oriImgName)
{
	int* temp = new int[2];
	Mat img = imread(oriImgName);
	if (img.empty())
	{
		fprintf(stderr, "Can not load image %s\n", oriImgName);
		return NULL;
	}
	int start_x = 0;
	int start_y = 0;
	int flag = 0;
	int flag2 = 0;
	for (int row = 100; row < img.rows; row++)
	{
		Vec3b * p1 = img.ptr<Vec3b>(row);

		for (int col = 0; col < 100; col++)
		{
			for (int i = col; i < col + LEFTTOP_PIXELNUM; i++)
			{
				if (p1[i][0] <= LRTB_THRESHOLD && p1[i][1] <= LRTB_THRESHOLD && p1[i][2] <= LRTB_THRESHOLD)
				{
					flag += 1;
				}
				else
				{
					flag = 0;
					break;
				}
			}
			if (flag >= LEFTTOP_PIXELNUM - 5)
			{
				flag2 = 0;
				for (int j = row; j < row + LEFTTOP_PIXELNUM; j++)
				{
					Vec3b * p1 = img.ptr<Vec3b>(j);
					if (p1[col][0] <= LRTB_THRESHOLD && p1[col][1] <= LRTB_THRESHOLD && p1[col][2] <= LRTB_THRESHOLD)
					{
						flag2 += 1;
					}
					else
					{
						flag2 = 0;
						break;
					}
				}
				if (flag2 >= LEFTTOP_PIXELNUM - 5)
				{
					start_y = row;
					start_x = col;
					flag2 = 0;
					flag = 0;
					break;
				}
			}
			flag = 0;
		}
		if (start_x != 0 || start_y != 0)
		{
			break;
		}
	}
	temp[0] = start_x;
	temp[1] = start_y;
	return temp;
}

int* CBase::getRightToppoint_afterClip(string oriImgName)
{
	int* temp = new int[2];
	Mat img = imread(oriImgName);
	if (img.empty())
	{
		fprintf(stderr, "Can not load image %s\n", oriImgName);
		return NULL;
	}
	int start_x = 0;
	int start_y = 0;
	int flag = 0;
	int flag2 = 0;
	for (int row = 100; row < img.rows; row++)
	{
		Vec3b * p1 = img.ptr<Vec3b>(row);
		for (int col = img.cols; col > img.cols - 200; col--)
		{
			for (int i = col; i > col - RIGHTTOP_PIXELNUM; i--)
			{
				if (p1[i][0] <= LRTB_THRESHOLD && p1[i][1] <= LRTB_THRESHOLD && p1[i][2] <= LRTB_THRESHOLD)
				{
					flag += 1;
				}
				else
				{
					flag = 0;
					break;
				}
			}
			if (flag >= RIGHTTOP_PIXELNUM - 5)
			{
				flag2 = 0;
				for (int j = row; j < row + RIGHTTOP_PIXELNUM; j++)
				{
					Vec3b * p1 = img.ptr<Vec3b>(j);
					if (p1[col][0] <= LRTB_THRESHOLD && p1[col][1] <= LRTB_THRESHOLD && p1[col][2] <= LRTB_THRESHOLD)
					{
						flag2 += 1;
					}
					else
					{
						flag2 = 0;
						break;
					}
				}
				if (flag2 >= RIGHTTOP_PIXELNUM - 5)
				{
					start_y = row;
					start_x = col;
					flag2 = 0;
					flag = 0;
					break;
				}
			}
			flag = 0;
		}
		if (start_x != 0 || start_y != 0)
		{
			break;
		}
	}
	temp[0] = start_x;
	temp[1] = start_y;
	return temp;
}

//5. 去除噪声点////////////
void CBase::denoisingImg(string filename, int threshold)
{
	IplImage* src = cvLoadImage(filename.c_str(), CV_LOAD_IMAGE_GRAYSCALE);
	IplImage* dst = cvCreateImage(cvGetSize(src), 8, 3);
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* contour = 0;
	cvThreshold(src, src, 30, 255, CV_THRESH_BINARY);
	cvXorS(src, cvScalarAll(255), src);//图像反色
	int contour_num = cvFindContours(src, storage, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	cvZero(dst);// 清空数组  
	CvSeq *_contour = contour;
	for (; contour != 0; contour = contour->h_next)
	{
		double tmparea = fabs(cvContourArea(contour));
		if (tmparea < threshold)
		{
			cvSeqRemove(contour, 0); // 删除面积小于设定值的轮廓  
			continue;
		}
		CvRect aRect = cvBoundingRect(contour, 0);
		if (aRect.width < 3)
		{
			cvSeqRemove(contour, 0);
			continue;
		}
		if (aRect.height < 5 && aRect.width>10)
		{
			cvSeqRemove(contour, 0);
			continue;
		}
		if (aRect.height > 15 && aRect.width < 6)
		{
			cvSeqRemove(contour, 0);
			continue;
		}
		if ((aRect.height / aRect.width) > 10)
		{
			cvSeqRemove(contour, 0);
			continue;
		}
		if ((aRect.width / aRect.height) > 17)
		{
			cvSeqRemove(contour, 0);
			continue;
		}
		cvDrawContours(dst, contour, CV_RGB(255, 255, 255), CV_RGB(255, 255, 255), -1, CV_FILLED, 8);//填充轮廓
	}
	cvXorS(dst, cvScalarAll(255), dst);
	cvSaveImage(filename.c_str(), dst);
	cvReleaseImage(&src);
	cvReleaseImage(&dst);
	return;
}