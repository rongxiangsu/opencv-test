#pragma once
#include "Base.h"
//�����������ʶ�����ֽ��洦����ֵ
#define FIX_WORDTHRE 6

//�жϸ��������յ�ʱ�������ٸ�������ֵ
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

