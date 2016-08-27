#pragma once
#include "Base.h"

//去除背景操作的阈值常量
#define IMG_THRESHOLD 95

class CThreshold :
	public CBase
{
public:
	CThreshold(string infile);
	~CThreshold();
	void thresholdImg();

};

