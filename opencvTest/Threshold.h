#pragma once
#include "Base.h"

//ȥ��������������ֵ����
#define IMG_THRESHOLD 95

class CThreshold :
	public CBase
{
public:
	CThreshold(string infile);
	~CThreshold();
	void thresholdImg();

};

