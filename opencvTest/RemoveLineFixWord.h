#pragma once
#include "Base.h"
//补齐干扰线与识别文字交叉处的阈值
#define FIX_WORDTHRE 6

//判断干扰线起终点时连续多少个像素阈值
#define CURE_THRE 8

class CRemoveLineFixWord :
	public CBase
{
public:
	CRemoveLineFixWord(string infile);
	~CRemoveLineFixWord();

	string EliminateInterCurve(string oriImgName);
	string fixWord(string oriImgName);
	void rename_img(int i, string prefix);
	void repeatFucntion(int i, string prefix);
	void repeatFucntion2(string prefix, string str1);
	void removeLineAndFixWord(int* offset);
};

