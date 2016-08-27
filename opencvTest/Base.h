#pragma once
#include <cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <windows.h>

using namespace std;
using namespace cv;

//查询左上角点坐标时，定义连续出现多少个低于阈值的像素点
#define LEFTTOP_PIXELNUM 50
#define RIGHTTOP_PIXELNUM 50
#define RIGHTBOTTOM_PIXELNUM 50
#define LRTB_THRESHOLD 140

class CBase
{
public:
	CBase();
	~CBase();

	string inttoStr(int i)
	{
		stringstream ss;
		string str1;
		ss << i;
		ss >> str1;
		return str1;
	}

	vector<string> split(string str, string pattern)
	{
		string::size_type pos;
		vector<string> result;
		str += pattern;
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

	/*
	@split input image file name and return an output file name
	including suffix gave by parameter "outputSuffix"
	*/
	string outputFileName(string fileName, string outputSuffix)
	{
		vector<string> result = split(fileName, ".");
		string outFileName = result[0] + outputSuffix;
		return outFileName;
	}

	int* getLeftTopPoint(string oriImgName);
	int* getRightBottomPoint(string oriImgName);
	int* getRightToppoint(string oriImgName);
	int* getLeftTopPoint_afterClip(string oriImgName);
	int* getRightToppoint_afterClip(string oriImgName);
	string getOutFileName();

	void denoisingImg(string filename, int threshold);

protected:
	string fileName;
	string out_fileName;
};

