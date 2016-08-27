#pragma once
#include "Base.h"

class CRotateImage :
	public CBase
{
public:
	CRotateImage(string fileName);
	~CRotateImage();

	IplImage* rotateImage(double degree);
	void correctImage();

};

