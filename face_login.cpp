#include<iostream>
#include<cv.h>
#include<highgui.h>
#include <thread>
#include <direct.h>
#include <io.h>
#include <iostream>

#define img_width 1080 //定义保存图片的宽高
#define img_height 720

using namespace cv;
using namespace std;

Mat curImage;				//当前图像
Mat img;                //当前图像拷贝
bool isexit = false;        //是否退出
bool isReg = false;
//bool isFaceExisted;	//图像是否存在

/**
* @brief string转char*
* s : string字符串
* @return char*
**/
char *StringTOchar(string s)
{
	char *pText = new char[s.length() + 1];
	strcpy(pText, s.c_str());
	return pText;
}

/**
* @brief 将int转化为string
* n : 待转化数据
* @return 返回 n所转化的string结果
* */
string ItoS(int n)
{
	char a[10];
	string str;
	_itoa(n, a, 10);
	str = a;
	return str;
}

/**
* @brief 新建文件目录
* fileName : 文件目录
* @return void
**/
void NewFile(char* fileName)
{
	char *tag;
	for (tag = fileName; *tag; tag++)
	{
		if (*tag == '\\')
		{
			char buf[1000], path[1000];
			strcpy(buf, fileName);
			buf[strlen(fileName) - strlen(tag) + 1] = NULL;
			strcpy(path, buf);
			if (_access(path, 6) == -1)
			{
				_mkdir(path);
			}
		}
	}
}

/**
* @brief 显示画面线程
* capture : 视频流
* @return void
* */
bool ShowThread(VideoCapture capture)
{

	int t = 10;
	while (curImage.data == 0){
		capture >> curImage;
		waitKey(40);
		t--;
		if (t <= 0){
			std::cout << "摄像头加载失败!" << std::endl;
			return false;
		}
	}

	Mat frame;
	while (true && !isexit)
	{
		capture >> curImage;
		if (curImage.data == 0){
			std::cout << "摄像头连接中断!" << std::endl;
			break;
		}
		curImage.copyTo(frame);
		curImage.copyTo(img);
		imshow("", frame);
		waitKey(40);
	}
	return false;
}

void MainThread(VideoCapture capture, int trainNum, string file)
{
	while (true)
	{
		int pattern;
		cout << "请选择模式：";
		cin >> pattern;
		if (pattern == 0)
		{
			isexit = true;
			break;
		}
		else if (pattern == 1)
		{
			string id = "";
			cout << "请输入注册ID：";
			cin >> id;
			string temp = file;
			file = file + "\\" + id + "\\";
			char *fileName = StringTOchar(file);
			if (_access(fileName, 6) != -1)
			{
				cout << "用户已经存在！！！" << endl;
				continue;
			}
			else
			{
				NewFile(fileName);
			}
			file = temp;
			int num_pic = 0;
			while (num_pic <= trainNum)
			{
				num_pic++;
				if (num_pic <= trainNum)
				{
					string temp = fileName;
					string pic = temp + ItoS(num_pic) + ".bmp";
					IplImage* img_face = cvCreateImage(cvSize(img_width, img_height), 8, 3);
					img_face = cvCloneImage(&(IplImage)img);
					cvSaveImage(pic.c_str(), img_face);
					cvWaitKey(1000);//保存每张图片之间间隔1s，用于调整姿势，存储不同状态的人脸。
				}
			}
			isReg = true;
			cout << "注册完成！！！" << endl;
		}
		else
		{
			continue;
		}
	}
}
int main()
{
	string path = ".//login";//注册图片保存路径
	int trainNum = 5;//每类的注册样本数

	cout << "********************人脸注册程序********************" << endl;
	cout << "**********     0: 退出      1. 注册          ***********" << endl << endl;
	VideoCapture capture(0);
	std::thread td1(ShowThread, capture);
	std::thread td2(MainThread, capture, trainNum, path);
	td1.join();
	td2.join();

	return 0;
}
