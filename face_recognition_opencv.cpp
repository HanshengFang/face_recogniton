#include<cv.h>
#include<iostream>
#include<highgui.h>
#include<opencv2/contrib/contrib.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

void printMat(CvMat* mat)//打印mat矩阵
{
	for (int i = 0; i < mat->rows; i++)
	{
		for (int j = 0; j < mat->cols; j++)
		{
			cout << cvmGet(mat, i, j) << " ";
		}
		cout << endl;
	}
}

static Mat norm_0_255(InputArray _src) {
	Mat src = _src.getMat();
	// Create and return normalized image:
	Mat dst;
	switch (src.channels()) {
	case 1:
		cv::normalize(_src, dst, 0, 255, NORM_MINMAX, CV_8UC1);
		break;
	case 3:
		cv::normalize(_src, dst, 0, 255, NORM_MINMAX, CV_8UC3);
		break;
	default:
		src.copyTo(dst);
		break;
	}
	return dst;
}

int main()
{
	//存放训练人脸和标签
	vector<int> label_face_train;
	vector<Mat> img_face_train;
	//存放测试人脸和标签
	vector<int> label_face_test;
	vector<Mat> img_face_test;

	//string file_path = "G:\\face_datasets\\gt_db2\\s";
	
	for (int i = 1; i < 41; i++)
	{
		for (int j = 1; j < 11; j++)
		{
			char temp_path[40];
			sprintf(temp_path, "%s%d%s%d%s", "G:\\face_datasets\\att_faces\\s", i, "\\", j, ".pgm");
			Mat img = imread(temp_path, CV_LOAD_IMAGE_GRAYSCALE);
			//imshow("img", img);
			//Mat norm_img;
			if (j == 1 || j == 3 || j == 5)//测试集
			{
				label_face_test.push_back(i);
				img_face_test.push_back(img);
			}
			else//训练集
			{
				label_face_train.push_back(i);
				img_face_train.push_back(img);
			}
		}
	}

	Ptr<FaceRecognizer> model1 = createFisherFaceRecognizer();  //LDA method
	//Ptr<FaceRecognizer> model = createEigenFaceRecognizer();  //PCA method
	//Ptr<FaceRecognizer> model = createLBPHFaceRecognizer(); //LBP method
	//model->train(img_face_train,label_face_train);
	//保存模型
	model1->load("LDA_orl.yml");
	/*int test_size = label_face_test.size();
	vector<Mat>::iterator it;
	vector<int>::iterator it2;
	int count = 0;
	for ((it = img_face_test.begin()), (it2 = label_face_test.begin()); it != img_face_test.end(); it++, it2++)
	{
		int predict_label = model->predict(*it);
		cout << "original:" << *it2 << " " << "predicted: " << predict_label << endl;
		if (predict_label == *it2)
		{
			count++;
		}
	}
	float accuracy = float(count) / float(test_size + 1e-8);
	cout << accuracy << endl;*/
	
	Mat eigenvalues = model1->getMat("eigenvalues");
	Mat W = model1->getMat("eigenvectors");
	Mat mean = model1->getMat("mean");
	int cols = eigenvalues.cols;
	int rows = eigenvalues.rows;
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			cout << eigenvalues.at<double>(i, j) << " ";
		}
		cout<< endl;
	}
	// Display or save the Eigenfaces:
	string output_folder = ".";
	int height = img_face_train[0].rows;
	for (int i = 0; i < min(10, W.cols); i++) {
		string msg = format("Eigenvalue #%d = %.5f", i, eigenvalues.at<double>(i));
		cout << msg << endl;
		// get eigenvector #i
		Mat ev = W.col(i).clone();
		// Reshape to original size & normalize to [0...255] for imshow.
		Mat grayscale = norm_0_255(ev.reshape(1, height));
		// Show the image & apply a Jet colormap for better sensing.
		Mat cgrayscale;
		applyColorMap(grayscale, cgrayscale, COLORMAP_JET);
		// Display or save:
		imshow(format("eigenface_%d", i), cgrayscale);
		waitKey(0);
		imwrite(format("%s/eigenface_%d.png", output_folder.c_str(), i), norm_0_255(cgrayscale));
		
	}
	return 0;
}
