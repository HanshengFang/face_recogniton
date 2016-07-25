#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include<cv.h>
#include<highgui.h>
#include<iostream>


#define FACEWIDTH 100
#define FACEHEIGHT 100
#define faceNumMaximun 10//最多能一次检测的人脸数

using namespace std;
using namespace cv;

////人脸区域位置
//typedef struct FaceRect{
//	int x;					//x坐标
//	int y;					//y坐标
//	int width;				//宽度
//	int height;				//高度
//};

//规格化得到的人脸图片
IplImage* StandardImage(IplImage *img)
{
	//灰度化
	IplImage *grayImg = cvCreateImage(cvGetSize(img), 8, 1);
	cvCvtColor(img, grayImg, CV_BGR2GRAY);
	//直方图均衡化
	IplImage *eqhistImg = cvCreateImage(cvGetSize(grayImg), 8, 1);
	cvEqualizeHist(grayImg, eqhistImg);
	//标准化尺寸
	IplImage *resizeImage = cvCreateImage(cvSize(FACEWIDTH, FACEHEIGHT), 8, 1);
	cvResize(eqhistImg, resizeImage, CV_INTER_LINEAR);
	cvReleaseImage(&eqhistImg);
	cvReleaseImage(&grayImg);
	return resizeImage;
}

//截取人脸区域
IplImage* getFaceArea(IplImage *img, CvRect *faces)
{
	CvRect Rect = faces[0];
	IplImage *srcimg;
	srcimg = cvCloneImage(img);
	//获取人脸矩形框区域
	cvSetImageROI(srcimg, Rect);
	CvSize Size1 = cvGetSize(srcimg);
	CvMat * Matrix = cvCreateMat(Size1.width, Size1.height, CV_8UC3);
	cvGetMat(srcimg, Matrix, NULL, 3);
	IplImage *img2 = cvCreateImageHeader(Size1, IPL_DEPTH_8U, 0);
	cvGetImage(Matrix, img2);

	//标准化图像
	IplImage *img3 = StandardImage(img2);
	IplImage *dst = cvCreateImage(cvSize(FACEWIDTH, FACEHEIGHT), 8, 1);
	cvCopy(img3, dst, NULL);
	cvReleaseImageHeader(&img2);
	cvReleaseImageHeader(&img3);
	cvReleaseImage(&srcimg);
	return dst;
}

//检测是否有人脸并画出
bool DetectAndDrawFaces(IplImage* img, CvRect *faceRect,int num)
{
	vector<CvRect>rects;
	faceRect = new CvRect[faceNumMaximun];
	CvMemStorage* storage = 0;
	CvHaarClassifierCascade* cascade = NULL;
	storage = cvCreateMemStorage(0);
	bool flag = false;
	int face_width[10], face_height[10];
	static CvScalar colors[] =
	{
		{ { 0, 255, 0 } }
	};
	double scale = 1.2;

	//缩小图片有利于加快检测速度
	IplImage* gray = cvCreateImage(cvSize(img->width, img->height), 8, 1);
	IplImage* small_img = cvCreateImage(cvSize(cvRound(img->width / scale), (img->height / scale)), 8, 1);

	cvCvtColor(img, gray, CV_BGR2GRAY);
	cvResize(gray, small_img, CV_INTER_LINEAR);
	cascade = (CvHaarClassifierCascade*)cvLoad("haarcascade_frontalface_alt.xml", 0, 0, 0);
	if (!cascade)
	{
		std::cerr << "无法加载haarcascade_frontalface_alt.xml文件" << std::endl;
	}
	if (cascade)
	{
		CvSeq* fcvs = cvHaarDetectObjects(small_img, cascade, storage, 1.1, 2, 0, cvSize(30, 30));
		//int num_face = fcvs->total;
		//cout << num_face << endl;
		num = (fcvs ? fcvs->total : 0);
		cout << num << endl;
		if (num != 0)
		{
			for (int i = 0; i < num; i++)
			{
				if (num>faceNumMaximun)
				{
					break;
				}
				CvRect* r = (CvRect*)cvGetSeqElem(fcvs, i);
				CvRect newR;
				newR.x = r->x*scale;
				newR.y = r->y*scale;
				newR.width = r->width*scale;
				newR.height = r->height*scale;
				//筛选检测到的人脸使之满足最小尺寸10*10，最大尺寸100*100
				if (newR.width > 10 && newR.width<200 && newR.height>10 && newR.height < 200)
				{
					char temp_path[30];
					faceRect[i].x = newR.x - 5;
					faceRect[i].y = newR.y - 5;
					faceRect[i].width = newR.width*1.05;
					faceRect[i].height = newR.height*1.15;
					rects.push_back(faceRect[i]);//人脸图片存到容器中

					//画人脸
					/*
					CvPoint p1, p2;
					p1.x = faceRect[i].x;
					p1.y = faceRect[i].y;
					p2.x = p1.x + faceRect[i].width;
					p2.y = p1.y + faceRect[i].height;
					cvRectangle(img, p1, p2, colors[0], 3, 8, 0);
					*/

					CvSize size = cvSize(faceRect[i].width, faceRect[i].height);
					cvSetImageROI(img, cvRect(faceRect[i].x, faceRect[i].y, faceRect[i].width, faceRect[i].height));
					//创建区域存储人脸图片
					IplImage* dst = cvCreateImage(size, img->depth, img->nChannels);
					cvCopy(img, dst);
					dst=StandardImage(dst);
					cvResetImageROI(dst);
					sprintf(temp_path, "%s%d%s", "./test/", i+7, ".jpg");
					cvSaveImage(temp_path, dst);
					cvReleaseImage(&dst);
				}
			}
			flag = true;
			/*
			//将获取到的人脸图片存储到指定路径path
			vector<CvRect>::iterator it;
			int face_num = 0;
			char file_dst[20];
			for (it = rects.begin(); it != rects.end(); it++)
			{
				int width_face = rects[face_num].width;
				int height_face = rects[face_num].height;
				int x_face = rects[face_num].x;
				int y_face = rects[face_num].y;
				
				CvSize size = cvSize(width_face, height_face);
				cvSetImageROI(img, cvRect(x_face,y_face,width_face,height_face));
				//创建区域存储人脸图片
				IplImage* dst = cvCreateImage(size, img->depth, img->nChannels);
				cvCopy(img, dst);
				//cout << dst->width << endl;
				cvResetImageROI(dst);
				//sprintf(file_dst, "%s%d%s", path, face_num, ".jpg");
				//cvSaveImage(file_dst,dst);
				//path = path + "res.jpg";
				cvSaveImage(".//f4.jpg",dst);
				face_num++;
			}
			*/

		}
	}
	cvReleaseImage(&gray);
	cvReleaseImage(&small_img);
	cvReleaseHaarClassifierCascade(&cascade);
	cvReleaseMemStorage(&storage);
	return flag;
}

int main()
{
	CvRect* faces = new CvRect[faceNumMaximun];
	//string path = "F:\\post_graduate_project\\face_recognition\\face_recognition\\test\\1.jpg";
	//IplImage *face_area;
	IplImage *img = cvLoadImage(".\\Gee.jpg", CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
	while (!img)
	{
		cout << "Load image fail!" << endl;
		break;
	}
	bool has_face = false;
	int face_count=0;//计数人脸的个数
	has_face = DetectAndDrawFaces(img, faces, face_count);
	if (has_face == true)
	{
		//cout << face_num << endl;
		//for (int i = 0; i < face_num; i++)
		//{
		//	face_area = getFaceArea(img, faces);
		//	cvNamedWindow("face image", CV_WINDOW_AUTOSIZE);
		//	cvShowImage("face image", face_area);
		//}
		////cvSaveImage("face_image.jpg", face_area);
		cvNamedWindow("image", CV_WINDOW_AUTOSIZE);
		cvShowImage("image", img);
		cvWaitKey(0);

		cvDestroyWindow("image");
		//cvReleaseImage(&face_area);
		cvReleaseImage(&img);
		delete[] faces;
		
	}
	else
	{
		cout << "no face exits!" << endl;
	}
	return 0;
}
