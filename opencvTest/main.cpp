#include "Base.h"
#include "ClipItem.h"
#include "CuttingEdge.h"
#include "RemoveLineFixWord.h"
#include "RotateImage.h"
#include "Threshold.h"

void getRuntime(DWORD start_time)
{
	cout << "Processing is completed!" << endl;
	DWORD end_time = GetTickCount();
	cout << "The run time is:" << (end_time - start_time) / 1000.0 << "s!" << endl;
}

int main(int argc, char* argv[])
{
	DWORD start_time = GetTickCount();

	string filename = "E:/[code]/QT/certificateOCR/certificateOCR/images/test1/1.jpg";
	CRotateImage rotate_img(filename);
	rotate_img.correctImage();
	CCuttingEdge cutedge_img(rotate_img.getOutFileName());
	cutedge_img.clipOriginImage();
	CThreshold thre_img(cutedge_img.getOutFileName());
	thre_img.thresholdImg();
	//thre_img.denoisingImg(thre_img.getOutFileName(), 5);
	CClipItem clip_img(thre_img.getOutFileName());
	int* offset = clip_img.getOffset(cutedge_img.getOutFileName());
	clip_img.clipImageItem(cutedge_img.getOutFileName(), offset);
	CRemoveLineFixWord remFix_img(thre_img.getOutFileName());
	remFix_img.removeLineAndFixWord(offset);

	getRuntime(start_time);
	//cv::waitKey(0);
	getchar();
	return 0;
}