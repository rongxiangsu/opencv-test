#pragma once
#include "Base.h"

//�ж�ӡˢ�����Ƿ���ƫ�Ƶ�������ֵ���Լ����ظ�����ֵ
#define OFFSET_THRE 6
#define OFFSET_NUM_FORUPPER 85
#define OFFSET_NUM_FORLOWER 27
//�ж��ϰ�ҳ֤��ת�ƵǼ����Ƿ������ݵ�������ֵ���Լ����ظ�����ֵ
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

