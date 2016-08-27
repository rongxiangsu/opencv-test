#pragma once
#include "Base.h"

//判断印刷文字是否发生偏移的像素阈值，以及像素个数阈值
#define OFFSET_THRE 6
#define OFFSET_NUM_FORUPPER 85
#define OFFSET_NUM_FORLOWER 27
//判断上半页证件转移登记栏是否有内容的像素阈值，以及像素个数阈值
#define CONTENT_THRE 6
#define CONTENT_NUM 85

class CClipItem :
	public CBase
{
public:
	CClipItem(string infile);
	~CClipItem();

	string clipItemMask(int* referPoint, int rowStart, int rowEnd, int colStart, int colEnd, Mat img, string oriImgName, int valOrItem, string number, int offset);
	int offsetOrNot(string oriImgName, int threshold);
	int* getOffset(string cuttingedge_filename);

	int contentOrNot(string oriImgName);
	void clipImageItem(string cuttingedge_filename, int* offset);
};

