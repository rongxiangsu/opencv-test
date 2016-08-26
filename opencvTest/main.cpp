#include <iostream>
#include <stdio.h>
#include <windows.h>
#include <math.h>
#include <cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

//倾斜矫正常量
#define GRAY_THRESH 150
#define HOUGH_VOTE 100

//去除背景操作的阈值常量
#define IMG_THRESHOLD 100

//查询左上角点坐标时，定义连续出现多少个低于阈值的像素点
#define LEFTTOP_PIXELNUM 50
#define RIGHTTOP_PIXELNUM 50
#define RIGHTBOTTOM_PIXELNUM 50
#define LRTB_THRESHOLD 140
//补齐干扰线与识别文字交叉处的阈值
#define FIX_WORDTHRE 6

//判断干扰线起终点时连续多少个像素阈值
#define CURE_THRE 8

//去噪阈值
#define DENOISING_THRE 10
#define DENOISING_WHITE_THRE 245
#define DENOISING_MINAREA 30

//判断印刷文字是否发生偏移的像素阈值，以及像素个数阈值
#define OFFSET_THRE 6
#define OFFSET_NUM_FORUPPER 85
#define OFFSET_NUM_FORLOWER 27

//判断上半页证件转移登记栏是否有内容的像素阈值，以及像素个数阈值
#define CONTENT_THRE 6
#define CONTENT_NUM 85

//int 转换成string
string inttoStr(int i)
{
	stringstream ss;
	string str1;
	ss << i;
	ss >> str1;
	return str1;
}

//分割字符串的方法
vector<string> split(string str, string pattern)
{
	string::size_type pos;
	vector<string> result;
	str += pattern;//扩展字符串以方便操作  
	int size = str.size();
	for (int i = 0; i < size; i++)
	{
		pos = str.find(pattern, i);
		if (pos < size)
		{
			string s = str.substr(i, pos - i);
			result.push_back(s);
			i = pos + pattern.size() - 1;
		}
	}
	return result;
}

//分割输入图像名称，返回输出图像名称的函数
string outputFileName(string oriImgName, string outputSuffix)
{
	vector<string> result = split(oriImgName, ".");
	string outFileName = result[0] + outputSuffix;
	return outFileName;
}

//图像旋转函数
IplImage* rotateImage(IplImage* img, double degree)
{
	double a = sin(degree), b = cos(degree);
	int width = img->width, height = img->height;
	//旋转后的新图尺寸  
	int width_rotate = int(height * fabs(a) + width * fabs(b));
	int height_rotate = int(width * fabs(a) + height * fabs(b));
	IplImage* img_rotate = cvCreateImage(cvSize(width_rotate, height_rotate), img->depth, img->nChannels);
	cvZero(img_rotate);
	//保证原图可以任意角度旋转的最小尺寸  
	int tempLength = sqrt((double)width * width + (double)height *height) + 10;
	int tempX = (tempLength + 1) / 2 - width / 2;
	int tempY = (tempLength + 1) / 2 - height / 2;
	IplImage* temp = cvCreateImage(cvSize(tempLength, tempLength), img->depth, img->nChannels);
	cvZero(temp);
	//将原图复制到临时图像tmp中心  
	cvSetImageROI(temp, cvRect(tempX, tempY, width, height));
	cvCopy(img, temp, NULL);
	cvResetImageROI(temp);
	//旋转数组map  
	// [ m0  m1  m2 ] ===>  [ A11  A12   b1 ]  
	// [ m3  m4  m5 ] ===>  [ A21  A22   b2 ]  
	float m[6];
	int w = temp->width;
	int h = temp->height;
	m[0] = b;
	m[1] = a;
	m[3] = -m[1];
	m[4] = m[0];
	// 将旋转中心移至图像中间    
	m[2] = w * 0.5f;
	m[5] = h * 0.5f;
	CvMat M = cvMat(2, 3, CV_32F, m);
	cvGetQuadrangleSubPix(temp, img_rotate, &M);
	cvReleaseImage(&temp);
	return img_rotate;
}

//获得图像左上角点裁剪边界点
int* getLeftTopPoint(string oriImgName)
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
int* getRightBottomPoint(string oriImgName)
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
int* getRightToppoint(string oriImgName)
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

//7. 获得裁剪后图像的左上角点，由于裁边后的图像的位置发生变化，所以原先的方法不适用，要改变一些阈值
int* getLeftTopPoint2(string oriImgName)
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

int* getRightToppoint2(string oriImgName)
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

//1. 计算应该矫正的度数
double calculateDegree(string oriImgName)
{
	int* startPoint = getLeftTopPoint(oriImgName);
	int* endPoint = getRightToppoint(oriImgName);
	double A = -atan((double)(endPoint[1] - startPoint[1]) / (double)(endPoint[0] - startPoint[0]));
	return A;
}

//2. 倾斜矫正
string correctImge(string oriImgName, double degree)
{
	cout << "Correcting image..." << endl;
	const char* imgname = oriImgName.c_str();
	IplImage * img = cvLoadImage(imgname);
	IplImage *correct_img = rotateImage(img, degree);
	string outFileName = outputFileName(oriImgName, "_correct.jpg");
	const char* outputStr = outFileName.c_str();
	cvSaveImage(outputStr, correct_img);
	cvReleaseImage(&img);
	cvReleaseImage(&correct_img);
	//Mat mat_correct_img = cvarrToMat(correct_img);
	//imwrite(outFileName, mat_correct_img);
	return outFileName;
}

//3. 裁剪矫正后的图像，去除黑边，只保留有用的部分
string clipOriginImage(string oriImgName)
{
	cout << "Clipping image..." << endl;
	int* startPoint = getLeftTopPoint(oriImgName);
	int* endPoint = getRightBottomPoint(oriImgName);
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
	Mat img = imread(oriImgName);
	Mat mask = Mat::Mat(img, R1, R2);
	string outFileName = outputFileName(oriImgName, "_clip.jpg");
	imwrite(outFileName, mask);
	return outFileName;
}

//直方图均衡化
IplImage* EqualizeHistColorImage(IplImage *pImage)
{
	IplImage *pEquaImage = cvCreateImage(cvGetSize(pImage), pImage->depth, 3);
	// 原图像分成各通道后再均衡化,最后合并即彩色图像的直方图均衡化  
	const int MAX_CHANNEL = 4;
	IplImage *pImageChannel[MAX_CHANNEL] = { NULL };
	int i;
	for (i = 0; i < pImage->nChannels; i++)
		pImageChannel[i] = cvCreateImage(cvGetSize(pImage), pImage->depth, 1);

	cvSplit(pImage, pImageChannel[0], pImageChannel[1], pImageChannel[2], pImageChannel[3]);

	for (i = 0; i < pImage->nChannels; i++)
		cvEqualizeHist(pImageChannel[i], pImageChannel[i]);

	cvMerge(pImageChannel[0], pImageChannel[1], pImageChannel[2], pImageChannel[3], pEquaImage);

	for (i = 0; i < pImage->nChannels; i++)
		cvReleaseImage(&pImageChannel[i]);
	return pEquaImage;
}

//执行直方图均衡化
string HistogramProcess(string originImg)
{
	cout << "Histogram..." << endl;
	IplImage *pSrcImage = cvLoadImage(originImg.c_str(), CV_LOAD_IMAGE_UNCHANGED);
	IplImage *pHisEquaImage = EqualizeHistColorImage(pSrcImage);
	string outFileName = outputFileName(originImg, "_histo.jpg");
	const char* outputStr = outFileName.c_str();
	cvSaveImage(outputStr, pHisEquaImage);
	cvReleaseImage(&pSrcImage);
	cvReleaseImage(&pHisEquaImage);
	return outFileName;
}

//4. 去除证件照除文字以外的背景
string removeBackgroud(string oriImgName)
{
	cout << "Removing background..." << endl;
	Mat origin_img = imread(oriImgName);
	if (origin_img.empty())
	{
		fprintf(stderr, "Can not load image %s\n", oriImgName);
		return NULL;
	}
	Mat temp_img(origin_img.size().height, origin_img.size().width, CV_8UC3);

	for (int row = 0; row < origin_img.rows; ++row)
	{
		Vec3b * p1 = origin_img.ptr<Vec3b>(row);
		Vec3b * p2 = temp_img.ptr<Vec3b>(row);
		for (int col = 0; col < origin_img.cols; ++col)
		{
			if (p1[col][2] < IMG_THRESHOLD && p1[col][1] < IMG_THRESHOLD && p1[col][0] < IMG_THRESHOLD)
			{
				p2[col][0] = 0;
				p2[col][1] = 0;
				p2[col][2] = 0;
			}
			else
			{
				p2[col][0] = 255;
				p2[col][1] = 255;
				p2[col][2] = 255;
			}
		}
	}
	string outFileName = outputFileName(oriImgName, "_removeBG.jpg");
	imwrite(outFileName, temp_img);
	return outFileName;
}

//5. 去除噪声点////////////
void denoisingImg(string oriImgName, int threshold)
{
	IplImage* src = cvLoadImage(oriImgName.c_str(), CV_LOAD_IMAGE_GRAYSCALE);
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
	//string outFileName = outputFileName(oriImgName, "_denosing.jpg");
	cvSaveImage(oriImgName.c_str(), dst);
	cvReleaseImage(&src);
	cvReleaseImage(&dst);
	return;
}


//6.1 & 8.1 获得待裁剪条目的范围
string clipItemMask(int* referPoint, int rowStart, int rowEnd, int colStart, int colEnd, Mat img, string oriImgName, int valOrItem, string number, int offset)
{
	//referpoint是左上角点
	Range R1;//R1表示裁剪行范围
	R1.start = referPoint[1] + rowStart + offset;
	R1.end = referPoint[1] + rowEnd + offset;
	Range R2;//R2表示裁剪列范围
	R2.start = referPoint[0] + colStart;
	R2.end = referPoint[0] + colEnd;
	// 	int imgheight = rowEnd - rowStart;
	// 	int imgwidth = colEnd - colStart;
	// 	cout << imgwidth/imgheight << endl;
	Mat mask = Mat::Mat(img, R1, R2);
	if (valOrItem == 0)
	{
		string outFileName = outputFileName(oriImgName, "_clipitem_" + number + "_item.jpg");
		imwrite(outFileName, mask);
		denoisingImg(outFileName, 7);
		return outFileName;
	}
	else if (valOrItem == 1)
	{
		string outFileName = outputFileName(oriImgName, "_clipitem_" + number + "_val.jpg");
		imwrite(outFileName, mask);
		denoisingImg(outFileName, 7);
		return outFileName;
	}
	else
	{
		cout << "Missing parameter 'valOrItem' in function clipItemMask()!" << endl;
		return NULL;
	}
}

//6.2 判断印刷偏移的距离
int offsetOrNot(string oriImgName, int threshold)
{
	Mat img = imread(oriImgName);
	if (img.empty())
	{
		fprintf(stderr, "Can not load image %s\n", oriImgName);
		return 0;
	}
	int num = 0;
	int offset_row = 0;
	for (int row = 0; row < img.rows; row++)
	{
		Vec3b * p1 = img.ptr<Vec3b>(row);
		num = 0;
		for (int col = 0; col < img.cols; col++)
		{
			if (p1[col][2] < OFFSET_THRE && p1[col][1] < OFFSET_THRE && p1[col][0] < OFFSET_THRE)
			{
				num++;
			}
		}
		//扫描每一行，若出现黑色像素点个数多于阈值，则该行为出现印刷文本的行数，再与正常行数做差，其差值即为偏移行数
		if (num > threshold)
		{
			//如果前五行就出现满足条件的情况，则认为不是偏移，可能是截图的时候把表格线也截入了，然后继续扫描
			if (row < 5)
			{
				continue;
			}
			else
			{
				offset_row = row;
				break;
			}
		}
		if (row == img.rows - 1)
		{
			offset_row = row;
			break;
		}
	}
	int normal_row = img.rows / 4;
	int offset_row_final = offset_row - normal_row;
	//如果偏移距离在[-5，5]内，则认为不存在干扰线影响
	if ((offset_row_final >= -5 && offset_row_final <= 0) || (offset_row_final > 0 && offset_row_final <= 5))
	{
		return 0;
	}
	return offset_row_final;
}

//6. 裁剪出第1和第6条目的图片，然后以此为标准判断印刷偏移的距离
int* getOffset(int* leftTop, string clipimg)
{
	Mat img = imread(clipimg);
	if (img.empty())
	{
		fprintf(stderr, "Can not load image %s\n", clipimg);
		return NULL;
	}
	string test1 = clipItemMask(leftTop, 5, 89, 810, 2230, img, clipimg, 1, "test_item1", 0);
	string test2 = clipItemMask(leftTop, 1720, 1810, 1704, 2000, img, clipimg, 1, "test_item6", 0);
	denoisingImg(test1, 7);
	denoisingImg(test2, 7);
	int offset_row1 = offsetOrNot(test1, OFFSET_NUM_FORUPPER);
	int offset_row2 = offsetOrNot(test2, OFFSET_NUM_FORLOWER);
	//offset_row1和offset_row2分别表示上半页的偏移距离和下半页的偏离距离
	int* temp = new int[2];
	temp[0] = offset_row1;
	temp[1] = offset_row2;
	return temp;
}

//8.2 判断证件上半页的转移登记栏是否有内容，若有则予以截图，若没有则不截图
int contentOrNot(string oriImgName)
{
	Mat img = imread(oriImgName);
	if (img.empty())
	{
		fprintf(stderr, "Can not load image %s\n", oriImgName);
		return 0;
	}
	int num = 0;
	for (int row = 0; row < img.rows; row++)
	{
		Vec3b * p1 = img.ptr<Vec3b>(row);
		num = 0;
		for (int col = 0; col < img.cols; col++)
		{
			if (p1[col][2] < CONTENT_THRE && p1[col][1] < CONTENT_THRE && p1[col][0] < CONTENT_THRE)
			{
				num++;
			}
		}
		//扫描每一行，若出现黑色像素点个数多于阈值，则该行为出现印刷文本的行数，即认为该栏目有内容
		if (num > CONTENT_NUM)
		{
			//如果前五行就出现满足条件的情况，则认为不是文本，可能是截图的时候把表格线也截入了，然后继续扫描
			if (row < 5)
			{
				continue;
			}
			else
			{
				return 1;
			}
		}
	}
	return 0;
}

//8. 裁剪证件条目
void clipImageItem(int* leftTop, string oriImgName, int* offset)
{
	cout << "Clipping item..." << endl;
	Mat img = imread(oriImgName);
	if (img.empty())
	{
		fprintf(stderr, "Can not load image %s\n", oriImgName);
		return;
	}
	int upper_page_offset = offset[0];
	int lower_page_offset = offset[1];
	//item l.机动车所有人/身份证明名称/号码
	//clipItemMask(5, 89, 100, 795, img, oriImgName,0,"1");
	//裁剪每一个条目对应的值
	//上半页证件
	clipItemMask(leftTop, 5, 89, 810, 2230, img, oriImgName, 1, "1", upper_page_offset);
	clipItemMask(leftTop, 95, 186, 406, 1075, img, oriImgName, 1, "2", upper_page_offset);
	clipItemMask(leftTop, 93, 181, 1365, 1620, img, oriImgName, 1, "3", upper_page_offset);
	clipItemMask(leftTop, 95, 184, 2010, 2285, img, oriImgName, 1, "4", upper_page_offset);
	//检查上半页的转移登记栏是否有内容，若有则截图，若没有则不截图
	int dis = 192;
	for (int i = 1; i < 7; i++)
	{
		string ii = inttoStr(i);
		string checkContent = clipItemMask(leftTop, 290 + dis*(i - 1), 375 + dis*(i - 1), 810, 2230, img, oriImgName, 1, "test_zy" + ii + "_1", upper_page_offset);
		//		denoisingImg(checkContent);
		if (contentOrNot(checkContent)){
			clipItemMask(leftTop, 290 + dis*(i - 1), 375 + dis*(i - 1), 810, 2230, img, oriImgName, 1, "zy" + ii + "_1", upper_page_offset);
			clipItemMask(leftTop, 388 + dis*(i - 1), 473 + dis*(i - 1), 406, 1075, img, oriImgName, 1, "zy" + ii + "_2", upper_page_offset);
			clipItemMask(leftTop, 387 + dis*(i - 1), 471 + dis*(i - 1), 1365, 1620, img, oriImgName, 1, "zy" + ii + "_3", upper_page_offset);
			clipItemMask(leftTop, 390 + dis*(i - 1), 471 + dis*(i - 1), 2010, 2285, img, oriImgName, 1, "zy" + ii + "_4", upper_page_offset);
			continue;
		}
		else{
			break;
		}
	}
	//下半页证件
	clipItemMask(leftTop, 1720, 1810, 497, 1000, img, oriImgName, 1, "5", lower_page_offset);
	clipItemMask(leftTop, 1720, 1810, 1704, 2000, img, oriImgName, 1, "6", lower_page_offset);
	clipItemMask(leftTop, 1815, 1905, 497, 900, img, oriImgName, 1, "7", lower_page_offset);
	clipItemMask(leftTop, 1815, 1905, 1704, 1930, img, oriImgName, 1, "8", lower_page_offset);
	clipItemMask(leftTop, 1910, 2000, 497, 1000, img, oriImgName, 1, "9", lower_page_offset);
	clipItemMask(leftTop, 1910, 2007, 1704, 1960, img, oriImgName, 1, "10", lower_page_offset);
	clipItemMask(leftTop, 2005, 2098, 497, 1000, img, oriImgName, 1, "11", lower_page_offset);
	clipItemMask(leftTop, 2010, 2104, 1704, 1900, img, oriImgName, 1, "12", lower_page_offset);
	clipItemMask(leftTop, 2100, 2195, 497, 915, img, oriImgName, 1, "13", lower_page_offset);
	clipItemMask(leftTop, 2109, 2200, 1704, 1920, img, oriImgName, 1, "14_1", lower_page_offset);
	clipItemMask(leftTop, 2109, 2200, 2018, 2210, img, oriImgName, 1, "14_2", lower_page_offset);
	clipItemMask(leftTop, 2195, 2270, 497, 1325, img, oriImgName, 1, "15", lower_page_offset);
	clipItemMask(leftTop, 2207, 2296, 1704, 2000, img, oriImgName, 1, "16", lower_page_offset);
	clipItemMask(leftTop, 2300, 2380, 560, 830, img, oriImgName, 1, "17_1", lower_page_offset);
	clipItemMask(leftTop, 2300, 2380, 930, 1200, img, oriImgName, 1, "17_2", lower_page_offset);
	clipItemMask(leftTop, 2305, 2373, 1704, 1830, img, oriImgName, 1, "18", lower_page_offset);
	clipItemMask(leftTop, 2395, 2480, 497, 860, img, oriImgName, 1, "19", lower_page_offset);
	clipItemMask(leftTop, 2400, 2489, 1817, 2208, img, oriImgName, 1, "20", lower_page_offset);
	clipItemMask(leftTop, 2492, 2568, 497, 880, img, oriImgName, 1, "21", lower_page_offset);
	clipItemMask(leftTop, 2497, 2565, 1704, 1830, img, oriImgName, 1, "22", lower_page_offset);
	clipItemMask(leftTop, 2590, 2677, 555, 800, img, oriImgName, 1, "23_1", lower_page_offset);
	clipItemMask(leftTop, 2590, 2677, 940, 1215, img, oriImgName, 1, "23_2", lower_page_offset);
	clipItemMask(leftTop, 2590, 2677, 1344, 1510, img, oriImgName, 1, "23_3", lower_page_offset);
	clipItemMask(leftTop, 2688, 2767, 553, 870, img, oriImgName, 1, "24_1", lower_page_offset);
	clipItemMask(leftTop, 2680, 2760, 942, 1289, img, oriImgName, 1, "24_2", lower_page_offset);
	clipItemMask(leftTop, 2680, 2760, 1347, 1590, img, oriImgName, 1, "24_3", lower_page_offset);
	clipItemMask(leftTop, 2785, 2873, 497, 780, img, oriImgName, 1, "25", lower_page_offset);
	clipItemMask(leftTop, 2785, 2873, 1341, 1606, img, oriImgName, 1, "26", lower_page_offset);
	clipItemMask(leftTop, 2882, 2970, 497, 780, img, oriImgName, 1, "27", lower_page_offset);
	clipItemMask(leftTop, 2888, 2974, 1341, 1606, img, oriImgName, 1, "28", lower_page_offset);
	clipItemMask(leftTop, 2978, 3068, 497, 850, img, oriImgName, 1, "29", lower_page_offset);
	clipItemMask(leftTop, 2985, 3070, 1341, 1686, img, oriImgName, 1, "30", lower_page_offset);
	if (lower_page_offset > 30)	{
		clipItemMask(leftTop, 3075, 3190, 497, 850, img, oriImgName, 1, "31", 10);
		clipItemMask(leftTop, 3079, 3187, 1341, 1686, img, oriImgName, 1, "32", 10);
		clipItemMask(leftTop, 3079, 3187, 1970, 2290, img, oriImgName, 1, "34", 10);
	}
	else{
		clipItemMask(leftTop, 3075, 3190, 497, 850, img, oriImgName, 1, "31", 0);
		clipItemMask(leftTop, 3079, 3187, 1341, 1686, img, oriImgName, 1, "32", 0);
		clipItemMask(leftTop, 3079, 3187, 1970, 2290, img, oriImgName, 1, "34", 0);
	}
	return;
}

//9.1 去除干扰线
string  EliminateInterCurve(string oriImgName)
{
	Mat origin_img = imread(oriImgName);
	string outFileName = outputFileName(oriImgName, "_removeLine.jpg");
	int left_min_pos_y = 0;
	int left_max_pos_y = 0;
	int flag1 = 0;
	//从第0行开始向下遍历，如果出现某一行的flag值大于等于4，那么认为该行位干扰线的起始行
	for (int row = 0; row < origin_img.rows; row++)
	{
		Vec3b * p1 = origin_img.ptr<Vec3b>(row);
		for (int col = 0; col < CURE_THRE; col++)
		{
			if (p1[col][0] <= 20 && p1[col][1] <= 20 && p1[col][2] <= 20)
			{
				flag1 += 1;
			}
		}
		if (flag1 >= CURE_THRE - 4)
		{
			left_min_pos_y = row;
			flag1 = 0;
			break;
		}
		flag1 = 0;
	}
	if (left_min_pos_y < 3)
	{
		imwrite(outFileName, origin_img);
		string returnstring = outFileName + "/0,0,0,0";
		return returnstring;
	}
	//从left_min_pos_y行开始向下遍历，如果出现某一行的flag值小于4，那么认为找到了干扰线的行最大值
	for (int row = left_min_pos_y; row < origin_img.rows - 1; row++)
	{
		Vec3b * p1 = origin_img.ptr<Vec3b>(row);
		for (int col = 0; col < CURE_THRE; col++)
		{
			if (p1[col][0] <= 20 && p1[col][1] <= 20 && p1[col][2] <= 20)
			{
				flag1 += 1;
			}
		}
		if (flag1 < CURE_THRE - 4 || row == (origin_img.rows - 2))
		{
			left_max_pos_y = row;
			flag1 = 0;
			break;
		}
		flag1 = 0;
	}
	int left_pos_y = (left_min_pos_y + left_max_pos_y) / 2;
	int right_min_pos_y = 0;
	int right_max_pos_y = 0;
	for (int row = 0; row < origin_img.rows; row++)
	{
		Vec3b * p1 = origin_img.ptr<Vec3b>(row);
		for (int col = origin_img.cols; col > origin_img.cols - CURE_THRE; col--)
		{
			if (p1[col][0] <= 20 && p1[col][1] <= 20 && p1[col][2] <= 20)
			{
				flag1 += 1;
			}
		}
		if (flag1 >= CURE_THRE - 4)
		{
			right_min_pos_y = row;
			flag1 = 0;
			break;
		}
		flag1 = 0;
	}
	for (int row = right_min_pos_y; row < origin_img.rows - 1; row++)
	{
		Vec3b * p1 = origin_img.ptr<Vec3b>(row);
		for (int col = origin_img.cols; col > origin_img.cols - CURE_THRE; col--)
		{
			if (p1[col][0] <= 20 && p1[col][1] <= 20 && p1[col][2] <= 20)
			{
				flag1 += 1;
			}
		}
		if (flag1 < CURE_THRE - 4 || row == (origin_img.rows - 2))
		{
			right_max_pos_y = row;
			flag1 = 0;
			break;
		}
		flag1 = 0;
	}
	int right_pos_y = (right_min_pos_y + right_max_pos_y) / 2;

	int pix_num1 = 0;
	int pix_num2 = 0;
	Vec3b * po1 = origin_img.ptr<Vec3b>(left_pos_y);
	Vec3b * po2 = origin_img.ptr<Vec3b>(right_pos_y);
	for (int col = 0; col < origin_img.cols; col++)
	{
		if (po1[col][0] <= 20 && po1[col][1] <= 20 && po1[col][2] <= 20)
		{
			pix_num1 += 1;
		}
		if (po2[col][0] <= 20 && po2[col][1] <= 20 && po2[col][2] <= 20)
		{
			pix_num2 += 1;
		}
	}
	IplImage *src1;
	const char* imgSrc1 = oriImgName.c_str();
	src1 = cvLoadImage(imgSrc1);
	int thickness1 = left_max_pos_y - left_min_pos_y;
	int thickness2 = right_max_pos_y - right_min_pos_y;
	const char* imgSrc2 = outFileName.c_str();

	if (oriImgName == "1_correct_clip_threshold_clipitem_15_val.jpg")
	{
		cout << left_pos_y << endl << right_pos_y << endl;
	}

	if (abs(left_pos_y - right_pos_y) <= 5)
	{
		int thickness = (thickness2 + thickness1) / 2 + 2;
		cvLine(src1, cvPoint(0, left_pos_y), cvPoint(origin_img.cols, right_pos_y), CV_RGB(255, 255, 255), thickness);//绘直线
		cvSaveImage(imgSrc2, src1);
		cvReleaseImage(&src1);
		string left_min = inttoStr(left_min_pos_y);
		string left_max = inttoStr(left_max_pos_y);
		string right_min = inttoStr(right_min_pos_y);
		string right_max = inttoStr(right_max_pos_y);
		string returnstring = outFileName + "/" + left_min + "," + left_max + "," + right_min + "," + right_max;
		return returnstring;
	}
	else if (pix_num1 >= pix_num2)
	{
		cvLine(src1, cvPoint(0, left_pos_y), cvPoint(origin_img.cols, left_pos_y), CV_RGB(255, 255, 255), thickness1 + 2);//绘直线
		cvSaveImage(imgSrc2, src1);
		cvReleaseImage(&src1);
		string left_min = inttoStr(left_min_pos_y);
		string left_max = inttoStr(left_max_pos_y);
		string right_min = inttoStr(left_min_pos_y);
		string right_max = inttoStr(left_max_pos_y);
		string returnstring = outFileName + "/" + left_min + "," + left_max + "," + right_min + "," + right_max;
		return returnstring;
	}
	else
	{
		cvLine(src1, cvPoint(0, right_pos_y), cvPoint(origin_img.cols, right_pos_y), CV_RGB(255, 255, 255), thickness2 + 2);//绘直线
		cvSaveImage(imgSrc2, src1);
		cvReleaseImage(&src1);
		string left_min = inttoStr(right_min_pos_y);
		string left_max = inttoStr(right_max_pos_y);
		string right_min = inttoStr(right_min_pos_y);
		string right_max = inttoStr(right_max_pos_y);
		string returnstring = outFileName + "/" + left_min + "," + left_max + "," + right_min + "," + right_max;
		return returnstring;
	}
}

//9.2 补齐干扰线与文字重叠部分
string fixWord(string oriImgName)
{
	vector<string> result1 = split(oriImgName, "/");
	vector<string> result2 = split(result1[0], ".");
	string outFileName = result2[0] + "_fixword.jpg";
	vector<string> result3 = split(result1[1], ",");
	int left_min_pos_y = atoi(result3[0].c_str());
	int left_max_pos_y = atoi(result3[1].c_str());
	//int right_min_pos_y = atoi(result3[2].c_str());
	//int right_max_pos_y = atoi(result3[3].c_str());
	IplImage *src1;
	const char* imgSrc1 = result1[0].c_str();
	src1 = cvLoadImage(imgSrc1);
	if (left_min_pos_y == 0)
	{
		const char* imgSrc2 = outFileName.c_str();
		cvSaveImage(imgSrc2, src1);
		return outFileName;
	}
	int flag2 = 0;
	int medium = (left_min_pos_y + left_max_pos_y) / 2;
	int flag_i = 0;
	//如果干扰线距离图片最下面的距离小于6个像素则直接重命名文件并输出
	if (src1->height - left_max_pos_y > 6)
	{
		flag_i = 2;
	}
	if (flag_i == 0)
	{
		const char* imgSrc2 = outFileName.c_str();
		cvSaveImage(imgSrc2, src1);
		return outFileName;
	}
	//某一像素点的上方有2个及以上的黑色像素点，则认为该点属于原来完整字体部分
	for (int row = left_min_pos_y - 2; row <= medium; row++)
	{
		for (int col = 0; col < src1->width; col++)
		{
			for (int j = row - 1; j > row - 5; j--)
			{
				CvScalar s1 = cvGet2D(src1, j, col);
				if (s1.val[0] <= FIX_WORDTHRE && s1.val[1] <= FIX_WORDTHRE && s1.val[2] <= FIX_WORDTHRE)
				{
					flag2 += 1;
				}
			}
			if (flag2 >= 2)
			{
				CvScalar s = cvGet2D(src1, row, col); // get the (i,j) pixel value
				s.val[0] = 0;
				s.val[1] = 0;
				s.val[2] = 0;
				cvSet2D(src1, row, col, s);//set the (i,j) pixel value
				flag2 = 0;
			}
			flag2 = 0;
		}
	}

	//某一像素点的下方有2个及以上的黑色像素点，则认为该点属于原来完整字体部分
	for (int row = left_max_pos_y + flag_i; row >= medium; row--)
	{
		for (int col = 0; col < src1->width; col++)
		{
			for (int j = row + 1; j < row + 5; j++)
			{
				CvScalar s1 = cvGet2D(src1, j, col);
				if (s1.val[0] <= FIX_WORDTHRE && s1.val[1] <= FIX_WORDTHRE && s1.val[2] <= FIX_WORDTHRE)
				{
					flag2 += 1;
				}
			}
			if (flag2 >= 2)
			{
				CvScalar s = cvGet2D(src1, row, col); // get the (i,j) pixel value
				s.val[0] = 0;
				s.val[1] = 0;
				s.val[2] = 0;
				cvSet2D(src1, row, col, s);//set the (i,j) pixel value
				flag2 = 0;
			}
			flag2 = 0;
		}
	}
	const char* imgSrc2 = outFileName.c_str();
	cvSaveImage(imgSrc2, src1);
	cvReleaseImage(&src1);
	return outFileName;
}

//9.3 若没有干扰线则直接重命名文件输出
string rename_img(int i, string prefix)
{
	string str1 = inttoStr(i);
	string oriImgName = prefix + "_clipitem_" + str1 + "_val.jpg";
	Mat origin_img = imread(oriImgName);
	string outFileName = outputFileName(oriImgName, "_removeLine_fixword.jpg");
	imwrite(outFileName, origin_img);
	return outFileName;
}

//9.4 声明removeLineAndFixWord方法中重用多次的执行去除干扰线和补齐文字的方法 
void repeatFucntion(int i, string prefix)
{
	string str1 = inttoStr(i);
	string inputFilename = prefix + "_clipitem_" + str1 + "_val.jpg";
	string eliminate_img = EliminateInterCurve(inputFilename);
	string fixword_img = fixWord(eliminate_img);
}

//9.5 由于一些字段的截图为2或3张，所以图片编号方式有所不同，针对这些图片重新再写一个重用的方法
void repeatFucntion2(string prefix, string str1)
{
	string inputFilename = prefix + "_clipitem_" + str1 + "_val.jpg";
	string eliminate_img = EliminateInterCurve(inputFilename);
	string fixword_img = fixWord(eliminate_img);
}

//9. 针对每一张截出的待识别图像执行去除干扰线和补齐文字的方法
void removeLineAndFixWord(int* offset, string oriImgName)
{
	cout << "Eliminating Interferential Curve ..." << endl;
	int upper_page_offset = offset[0];
	int lower_page_offset = offset[1];
	vector<string> res1 = split(oriImgName, ".");
	//如果上半页的偏移距离不为0则执行去除干扰线程序，并补齐文字
	if (upper_page_offset != 0)
	{
		for (int i = 1; i < 5; i++)
		{
			repeatFucntion(i, res1[0]);
		}
	}
	else
	{
		for (int i = 1; i < 5; i++)
		{
			rename_img(i, res1[0]);
		}
	}
	//如果下半页的偏移距离不为0则执行去除干扰线程序，并补齐文字
	if (lower_page_offset != 0)
	{
		for (int i = 5; i < 31; i++)
		{
			if (i == 14 || i == 17 || i == 23 || i == 24)
				continue;
			repeatFucntion(i, res1[0]);
		}
		repeatFucntion2(res1[0], "14_1");
		repeatFucntion2(res1[0], "14_2");
		repeatFucntion2(res1[0], "17_1");
		repeatFucntion2(res1[0], "17_2");
		repeatFucntion2(res1[0], "23_1");
		repeatFucntion2(res1[0], "23_2");
		repeatFucntion2(res1[0], "23_3");
		repeatFucntion2(res1[0], "24_1");
		repeatFucntion2(res1[0], "24_2");
		repeatFucntion2(res1[0], "24_3");
	}
	else
	{
		for (int i = 5; i < 31; i++)
		{
			rename_img(i, res1[0]);
		}
	}
	//31 32 34这三栏的高度比较大，所以当偏移距离大于30像素的时候再执行去除干扰线，
	//否则认为偏移距离不会造成干扰线影响并直接重命名文件输出
	if (lower_page_offset > 30)
	{
		for (int i = 31; i < 35; i++)
		{
			if (i == 33)
				continue;
			repeatFucntion(i, res1[0]);
		}
	}
	else
	{
		for (int i = 31; i < 35; i++)
		{
			if (i == 33)
				continue;
			rename_img(i, res1[0]);
		}
	}
}

//获得程序运行总时间
void getRuntime(DWORD start_time)
{
	DWORD end_time = GetTickCount();
	cout << "The run time is:" << (end_time - start_time) / 1000.0 << "s!" << endl;//输出运行时间
}

//二值化图像
string thresholdImg(string oriImgName)
{
	IplImage* src = cvLoadImage(oriImgName.c_str(), CV_LOAD_IMAGE_GRAYSCALE);
	cvThreshold(src, src, IMG_THRESHOLD, 255, CV_THRESH_BINARY);
	string outFileName = outputFileName(oriImgName, "_threshold.jpg");
	cvSaveImage(outFileName.c_str(), src);
	return outFileName;
}


int main(int argc, char* argv[])
{
	DWORD start_time = GetTickCount();
	//图片需要和程序放在同级目录下
	string oriImgName = "1.jpg";
	///////////1. 获得矫正角度///////////////////
	double degree = calculateDegree(oriImgName);
	///////////2. 倾斜矫正//////////////
	string correct_img = correctImge(oriImgName, degree);
	///////////3. 去除黑边，即以左上角点和右下角点进行裁剪//////////////
	string cliporigin_img = clipOriginImage(correct_img);

	/////////////二值化/////////////////////////
	string thres_img = thresholdImg(cliporigin_img);
	denoisingImg(thres_img, 5);//对整体图像先进行一次简单的去噪
	///////////获得左上角点坐标，用去背景前的图像作为原始图像处理得到的坐标准确度较高//////////////////
	int* leftTop = getLeftTopPoint2(cliporigin_img);

	///////////判断是否发生偏移，用去背景前的图片作为原始图像获取///////////////
	int* offset = getOffset(leftTop, thres_img);

	///////////裁剪证件条目//////////////
	clipImageItem(leftTop, thres_img, offset);
	///////////去除干扰线并补齐文字//////////////
	removeLineAndFixWord(offset, thres_img);

	cout << "Processing is completed!" << endl;
	getRuntime(start_time);
	//cv::waitKey(0);
	getchar();
	return 0;
}