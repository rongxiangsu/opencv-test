#include "RemoveLineFixWord.h"


CRemoveLineFixWord::CRemoveLineFixWord(string infile)
{
	this->fileName = infile;
}


CRemoveLineFixWord::~CRemoveLineFixWord()
{
}

//去除干扰线
string CRemoveLineFixWord::EliminateInterCurve(string oriImgName)
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
		string returnstring = outFileName + "@!0,0,0,0";
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
		string returnstring = outFileName + "@!" + left_min + "," + left_max + "," + right_min + "," + right_max;
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
		string returnstring = outFileName + "@!" + left_min + "," + left_max + "," + right_min + "," + right_max;
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
		string returnstring = outFileName + "@!" + left_min + "," + left_max + "," + right_min + "," + right_max;
		return returnstring;
	}
}

//补齐干扰线与文字重叠部分
string CRemoveLineFixWord::fixWord(string oriImgName)
{
	vector<string> result1 = split(oriImgName, "@!");
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

//若没有干扰线则直接重命名文件输出
void CRemoveLineFixWord::rename_img(int i, string prefix)
{
	string str1 = inttoStr(i);
	string oriImgName = prefix + "_clipitem_" + str1 + "_val.jpg";
	Mat origin_img = imread(oriImgName);
	string outFileName = outputFileName(oriImgName, "_removeLine_fixword.jpg");
	imwrite(outFileName, origin_img);
	return;
}

//声明removeLineAndFixWord方法中重用多次的执行去除干扰线和补齐文字的方法 
void CRemoveLineFixWord::repeatFucntion(int i, string prefix)
{
	string str1 = inttoStr(i);
	string inputFilename = prefix + "_clipitem_" + str1 + "_val.jpg";
	string eliminate_img = EliminateInterCurve(inputFilename);
	string fixword_img = fixWord(eliminate_img);
}

//由于一些字段的截图为2或3张，所以图片编号方式有所不同，针对这些图片重新再写一个重用的方法
void CRemoveLineFixWord::repeatFucntion2(string prefix, string str1)
{
	string inputFilename = prefix + "_clipitem_" + str1 + "_val.jpg";
	string eliminate_img = EliminateInterCurve(inputFilename);
	string fixword_img = fixWord(eliminate_img);
}

//针对每一张截出的待识别图像执行去除干扰线和补齐文字的方法
void CRemoveLineFixWord::removeLineAndFixWord(int* offset)
{
	cout << "Eliminating Interferential Curve ..." << endl;
	int upper_page_offset = offset[0];
	int lower_page_offset = offset[1];
	vector<string> res1 = split(fileName, ".");
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