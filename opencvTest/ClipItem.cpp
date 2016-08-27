#include "ClipItem.h"


CClipItem::CClipItem(string infile)
{
	this->fileName = infile;
}


CClipItem::~CClipItem()
{
}

string CClipItem::clipItemMask(int* referPoint, int rowStart, int rowEnd, int colStart, int colEnd, Mat img, string oriImgName, int valOrItem, string number, int offset)
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

//判断印刷偏移的距离
int CClipItem::offsetOrNot(string oriImgName, int threshold)
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

//裁剪出第1和第6条目的图片，然后以此为标准判断印刷偏移的距离
//用去边后的图像来获取左上角坐标比较准确
int* CClipItem::getOffset(string cuttingedge_filename)
{
	int* leftTop = getLeftTopPoint_afterClip(cuttingedge_filename);
	Mat img = imread(fileName);
	if (img.empty())
	{
		fprintf(stderr, "Can not load image %s\n", fileName);
		return NULL;
	}
	string test1 = clipItemMask(leftTop, 5, 89, 810, 2230, img, fileName, 1, "test_item1", 0);
	string test2 = clipItemMask(leftTop, 1720, 1810, 1704, 2000, img, fileName, 1, "test_item6", 0);
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


//判断证件上半页的转移登记栏是否有内容，若有则予以截图，若没有则不截图
int CClipItem::contentOrNot(string oriImgName)
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

//裁剪证件条目
void CClipItem::clipImageItem(string cuttingedge_filename, int* offset)
{
	int* leftTop = getLeftTopPoint_afterClip(cuttingedge_filename);
	cout << "Clipping item..." << endl;
	Mat img = imread(fileName);
	if (img.empty())
	{
		fprintf(stderr, "Can not load image %s\n", fileName);
		return;
	}
	int upper_page_offset = offset[0];
	int lower_page_offset = offset[1];
	//item l.机动车所有人/身份证明名称/号码
	//clipItemMask(5, 89, 100, 795, img, oriImgName,0,"1");
	//裁剪每一个条目对应的值
	//上半页证件
	clipItemMask(leftTop, 5, 89, 810, 2230, img, fileName, 1, "1", upper_page_offset);
	clipItemMask(leftTop, 95, 186, 406, 1075, img, fileName, 1, "2", upper_page_offset);
	clipItemMask(leftTop, 93, 181, 1365, 1620, img, fileName, 1, "3", upper_page_offset);
	clipItemMask(leftTop, 95, 184, 2010, 2285, img, fileName, 1, "4", upper_page_offset);
	//检查上半页的转移登记栏是否有内容，若有则截图，若没有则不截图
	int dis = 192;
	for (int i = 1; i < 7; i++)
	{
		string ii = inttoStr(i);
		string checkContent = clipItemMask(leftTop, 290 + dis*(i - 1), 375 + dis*(i - 1), 810, 2230, img, fileName, 1, "test_zy" + ii + "_1", upper_page_offset);
		//denoisingImg(checkContent);
		if (contentOrNot(checkContent)){
			clipItemMask(leftTop, 290 + dis*(i - 1), 375 + dis*(i - 1), 810, 2230, img, fileName, 1, "zy" + ii + "_1", upper_page_offset);
			clipItemMask(leftTop, 388 + dis*(i - 1), 473 + dis*(i - 1), 406, 1075, img, fileName, 1, "zy" + ii + "_2", upper_page_offset);
			clipItemMask(leftTop, 387 + dis*(i - 1), 471 + dis*(i - 1), 1365, 1620, img, fileName, 1, "zy" + ii + "_3", upper_page_offset);
			clipItemMask(leftTop, 390 + dis*(i - 1), 471 + dis*(i - 1), 2010, 2285, img, fileName, 1, "zy" + ii + "_4", upper_page_offset);
			continue;
		}
		else{
			break;
		}
	}
	//下半页证件
	clipItemMask(leftTop, 1720, 1810, 497, 1000, img, fileName, 1, "5", lower_page_offset);
	clipItemMask(leftTop, 1720, 1810, 1704, 2000, img, fileName, 1, "6", lower_page_offset);
	clipItemMask(leftTop, 1815, 1905, 497, 900, img, fileName, 1, "7", lower_page_offset);
	clipItemMask(leftTop, 1815, 1905, 1704, 1930, img, fileName, 1, "8", lower_page_offset);
	clipItemMask(leftTop, 1910, 2000, 497, 1000, img, fileName, 1, "9", lower_page_offset);
	clipItemMask(leftTop, 1910, 2007, 1704, 1960, img, fileName, 1, "10", lower_page_offset);
	clipItemMask(leftTop, 2005, 2098, 497, 1000, img, fileName, 1, "11", lower_page_offset);
	clipItemMask(leftTop, 2010, 2104, 1704, 1900, img, fileName, 1, "12", lower_page_offset);
	clipItemMask(leftTop, 2100, 2195, 497, 915, img, fileName, 1, "13", lower_page_offset);
	clipItemMask(leftTop, 2109, 2200, 1704, 1920, img, fileName, 1, "14_1", lower_page_offset);
	clipItemMask(leftTop, 2109, 2200, 2018, 2210, img, fileName, 1, "14_2", lower_page_offset);
	clipItemMask(leftTop, 2195, 2270, 497, 1325, img, fileName, 1, "15", lower_page_offset);
	clipItemMask(leftTop, 2207, 2296, 1704, 2000, img, fileName, 1, "16", lower_page_offset);
	clipItemMask(leftTop, 2300, 2380, 560, 830, img, fileName, 1, "17_1", lower_page_offset);
	clipItemMask(leftTop, 2300, 2380, 930, 1200, img, fileName, 1, "17_2", lower_page_offset);
	clipItemMask(leftTop, 2305, 2373, 1704, 1830, img, fileName, 1, "18", lower_page_offset);
	clipItemMask(leftTop, 2395, 2480, 497, 860, img, fileName, 1, "19", lower_page_offset);
	clipItemMask(leftTop, 2400, 2489, 1817, 2208, img, fileName, 1, "20", lower_page_offset);
	clipItemMask(leftTop, 2492, 2568, 497, 880, img, fileName, 1, "21", lower_page_offset);
	clipItemMask(leftTop, 2497, 2565, 1704, 1830, img, fileName, 1, "22", lower_page_offset);
	clipItemMask(leftTop, 2590, 2677, 555, 800, img, fileName, 1, "23_1", lower_page_offset);
	clipItemMask(leftTop, 2590, 2677, 940, 1215, img, fileName, 1, "23_2", lower_page_offset);
	clipItemMask(leftTop, 2590, 2677, 1344, 1510, img, fileName, 1, "23_3", lower_page_offset);
	clipItemMask(leftTop, 2688, 2767, 553, 870, img, fileName, 1, "24_1", lower_page_offset);
	clipItemMask(leftTop, 2680, 2760, 942, 1289, img, fileName, 1, "24_2", lower_page_offset);
	clipItemMask(leftTop, 2680, 2760, 1347, 1590, img, fileName, 1, "24_3", lower_page_offset);
	clipItemMask(leftTop, 2785, 2873, 497, 780, img, fileName, 1, "25", lower_page_offset);
	clipItemMask(leftTop, 2785, 2873, 1341, 1606, img, fileName, 1, "26", lower_page_offset);
	clipItemMask(leftTop, 2882, 2970, 497, 780, img, fileName, 1, "27", lower_page_offset);
	clipItemMask(leftTop, 2888, 2974, 1341, 1606, img, fileName, 1, "28", lower_page_offset);
	clipItemMask(leftTop, 2978, 3068, 497, 850, img, fileName, 1, "29", lower_page_offset);
	clipItemMask(leftTop, 2985, 3070, 1341, 1686, img, fileName, 1, "30", lower_page_offset);
	if (lower_page_offset > 30)	{
		clipItemMask(leftTop, 3075, 3190, 497, 850, img, fileName, 1, "31", 10);
		clipItemMask(leftTop, 3079, 3187, 1341, 1686, img, fileName, 1, "32", 10);
		clipItemMask(leftTop, 3079, 3187, 1970, 2290, img, fileName, 1, "34", 10);
	}
	else{
		clipItemMask(leftTop, 3075, 3190, 497, 850, img, fileName, 1, "31", 0);
		clipItemMask(leftTop, 3079, 3187, 1341, 1686, img, fileName, 1, "32", 0);
		clipItemMask(leftTop, 3079, 3187, 1970, 2290, img, fileName, 1, "34", 0);
	}
	return;
}
