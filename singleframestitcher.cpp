#include "singleframestitcher.hpp"

#include <iostream>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <sstream>
#include <direct.h> 

using namespace std;
using namespace cv;

void SingleFrameStitcher::ImageCounter(std::vector<cv::Mat> &movems, cv::Size imSize, cv::Size resultImageSize)
{
	double maxX=imSize.width/2, maxY=imSize.height/2, minX=-imSize.width/2, minY=-imSize.height/2;
	int sizeX, sizeY;

	if (movems.empty())
		return;

	for (auto const & cadr : movems)
	{
		vector<double> relativeCoordinates = CornerCoordinatesCounter (cadr,imSize);

		if(cadr.at<double>(0,2)+relativeCoordinates[0]>maxX)
			maxX=cadr.at<double>(0,2)+relativeCoordinates[0];
		if(cadr.at<double>(0,2)+relativeCoordinates[1]<minX)
			minX=cadr.at<double>(0,2)+relativeCoordinates[1];
		if(cadr.at<double>(1,2)+relativeCoordinates[2]>maxY)
			maxY=cadr.at<double>(1,2)+relativeCoordinates[2];
		if(cadr.at<double>(1,2)+relativeCoordinates[3]<minY)
			minY=cadr.at<double>(1,2)+relativeCoordinates[3];
	}

	sizeX= int (maxX-minX);
	sizeY= int (maxY-minY);
	m_origin.x = floor(minX) + imSize.width/2;
	m_origin.y = floor(minY) + imSize.height/2;

	if (!(resultImageSize == Size(0,0))){
		m_singleImageSize = resultImageSize;
	} else {
		m_singleImageSize = cv::Size(sizeX, sizeY);
	}

	count_x = ceil(sizeX/m_singleImageSize.width);
	count_y = ceil(sizeY/m_singleImageSize.height);

	cout << "Count of images will be " << count_x*count_y << " with size " << m_singleImageSize.width << "x" << m_singleImageSize.height << endl;
	cout << "with origin point (" <<m_origin.x << "," << m_origin.y << ")" << endl;

}

std::vector<double> SingleFrameStitcher::CornerCoordinatesCounter(const cv::Mat &cadr, cv::Size imSize){

	double relativeMaxX = 0, relativeMinX = 0, relativeMaxY = 0, relativeMinY = 0;

	vector<double> corners_coordinates_x;
	vector<double> corners_coordinates_y;
	vector<double> relative_max_min_x_y;

	corners_coordinates_x.push_back((imSize.width/2)*cadr.at<double>(0,0) +  (imSize.height/2)*cadr.at<double>(0,1));
	corners_coordinates_x.push_back((-imSize.width/2)*cadr.at<double>(0,0) +  (imSize.height/2)*cadr.at<double>(0,1));
	corners_coordinates_x.push_back((-imSize.width/2)*cadr.at<double>(0,0) +  (-imSize.height/2)*cadr.at<double>(0,1));
	corners_coordinates_x.push_back((imSize.width/2)*cadr.at<double>(0,0) +  (-imSize.height/2)*cadr.at<double>(0,1));

	for (int i = 0; i < corners_coordinates_x.size(); ++i){
		if (corners_coordinates_x[i] > relativeMaxX)
			relativeMaxX = corners_coordinates_x[i];
		if (corners_coordinates_x[i] < relativeMinX)
			relativeMinX = corners_coordinates_x[i];
	}

	relative_max_min_x_y.push_back(relativeMaxX);
	relative_max_min_x_y.push_back(relativeMinX);

	corners_coordinates_y.push_back((imSize.width/2)*cadr.at<double>(1,0) +  (imSize.height/2)*cadr.at<double>(1,1));
	corners_coordinates_y.push_back((-imSize.width/2)*cadr.at<double>(1,0) +  (imSize.height/2)*cadr.at<double>(1,1));
	corners_coordinates_y.push_back((-imSize.width/2)*cadr.at<double>(1,0) +  (-imSize.height/2)*cadr.at<double>(1,1));
	corners_coordinates_y.push_back((imSize.width/2)*cadr.at<double>(1,0) +  (-imSize.height/2)*cadr.at<double>(1,1));

	for (int i = 0; i < corners_coordinates_y.size(); ++i){
		if (corners_coordinates_y[i] > relativeMaxY)
			relativeMaxY = corners_coordinates_y[i];
		if (corners_coordinates_y[i] < relativeMinY)
			relativeMinY = corners_coordinates_y[i];
	}

	relative_max_min_x_y.push_back(relativeMaxY);
	relative_max_min_x_y.push_back(relativeMinY);

	return relative_max_min_x_y;
}

void SingleFrameStitcher::RetranslateToOrigin(std::vector<cv::Mat> &movems, cv::Size imSize)
{
    Mat start = cv::Mat::eye(3,3,CV_64FC1);

	start.at<double>(0,2)=-m_origin.x;
	start.at<double>(1,2)=-m_origin.y;

	//cout << image_number.end()-image_number_iterator << endl;

	for (auto & move : movems)
	{
		move=start* move;
		
		if (count_x == 1 && count_y == 1){
			continue;
		}

		int x_center_coordinate = (int)move.at<double>(0,2) % m_singleImageSize.width;
		int y_center_coordinate = (int)move.at<double>(1,2) % m_singleImageSize.height;
		int image_number_x =(int)move.at<double>(0,2) / m_singleImageSize.width;
		int image_number_y = (int)move.at<double>(1,2) / m_singleImageSize.height;

		std::vector<double> relativeCoordinates = CornerCoordinatesCounter (move, imSize);

		int over_x = ((x_center_coordinate+(int)relativeCoordinates[0])/m_singleImageSize.width)+1;
		if ((x_center_coordinate+(int)relativeCoordinates[0]) % m_singleImageSize.width == 0 || over_x == 1){
			over_x-=1;
		}
		int under_x = ((x_center_coordinate+(int)relativeCoordinates[1])/m_singleImageSize.width)-1;
		if ((x_center_coordinate+(int)relativeCoordinates[1]) % m_singleImageSize.width == 0 || under_x == -1){
			under_x+=1;
		}
		int over_y = ((y_center_coordinate+(int)relativeCoordinates[2])/m_singleImageSize.height)+1;
		if ((y_center_coordinate+(int)relativeCoordinates[2]) % m_singleImageSize.height == 0 || over_y == 1){
			over_y-=1;
		}
		int under_y = ((y_center_coordinate+(int)relativeCoordinates[3])/m_singleImageSize.height)-1;
		if ((y_center_coordinate+(int)relativeCoordinates[3])/m_singleImageSize.height == 0 || under_y == -1){
			under_y+=1;
		}

		image_number.push_back({});

		for (int i = under_x; i <= over_x; ++i){
			for (int j = under_y; j <= over_y; ++j){
				image_number[iterator].push_back(make_pair(i+image_number_x,j+image_number_y));
			}
		}
		iterator++;
	}
	// int al = 0;
	// for (auto i : image_number){
	// 	cout << "over_x: " << over_x << ", over_y: " << over_y << ", under_x: " << under_x << ", under_y: " << under_y << " ";
	// 	cout << al << "  "; 
	// 	for (auto j : i){
	// 		cout << j.first << ", " << j.second << "; ";
	// 	}
	// 	cout << endl;
	// 	al++;
	// }
	// cout << iterator;
}

// cv::Mat SingleFrameStitcher::MakePanno(std::vector<cv::Mat> &movems, std::vector<cv::Mat> &images)
// {
// 	return m_result;
// }

void SingleFrameStitcher::CreatePanno(cv::Mat image, cv::Mat origin)
{
	for (int i = 0; i < count_x; ++i){
		for (int j =0; j < count_y; ++j){
			new_m_result.push_back({Mat(m_singleImageSize,CV_8UC1)});
		}
	}
	//m_result = Mat(m_singleImageSize,CV_8UC1);
	cv::Mat current, tmp, mask;
	cv::cvtColor(image, current, cv::COLOR_BGR2GRAY);
	cv::warpPerspective(current,tmp, origin, m_singleImageSize);
	auto kernElem=cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3,3));
	cv::compare(tmp,0,mask,cv::CMP_GT);
	cv::erode(mask,mask, kernElem, cv::Point(-1,-1),5); 
	cv::add(tmp,0,m_result,mask);
}

void SingleFrameStitcher::AppendToPanno(cv::Mat image, cv::Mat origin)
{
	if (m_result.empty())
		throw std::runtime_error("Panno not created yet!");
	Mat grey, tmp, mask;
	cv::cvtColor(image, grey, cv::COLOR_BGR2GRAY);
	cv::Mat trans = origin.clone();
	trans.pop_back();
	cv::warpAffine(grey, tmp, trans, m_result.size());	
	cv::compare(tmp,0,mask, cv::CMP_GT);
	auto kernElem=cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3,3));
	cv::erode(mask,mask, kernElem, cv::Point(-1,-1),5);
	cv::Mat tmp2;
	cv::compare(m_result,0,tmp2,cv::CMP_GT);
	mask = mask & (~tmp2);
	makeSumm(tmp, m_result,  m_result, mask);
}

void SingleFrameStitcher::SaveImage(std::string filename)
{
	if(_mkdir("./result") == -1)
        cerr << " Error Occured : " << strerror(errno) << endl;
    else
        cout << "Directory Created";
	for (int i = 0; i < count_x; ++i){
		for (int j = 0; j < count_y; ++j){
			std::stringstream out_name;
			out_name << "./" << "result/" << filename << i << "_" << j << ".png";
			cv::imwrite( out_name.str(), m_result);
			cv::imwrite("./result/raw.png", new_m_result[0][0]);
		}
	}
}

void SingleFrameStitcher::makeSumm(cv::InputArray& _first, cv::InputArray& _second, cv::InputArray& _result, cv::InputArray& _mask)
{
	cv::Mat first = _first.getMat();
	cv::Mat second = _second.getMat();
	cv::Mat mask;
	if (!_mask.empty())
		mask = _mask.getMat();
	cv::Mat res = _result.getMat();
	unsigned int rows = first.rows;
	unsigned int cols = first.cols;


	for (int i = 0; i<rows; ++i)
	{
		uchar* fRow = first.ptr<uchar>(i); 
		uchar* sRow = second.ptr<uchar>(i);
		uchar* rRow = res.ptr<uchar>(i);
		uchar* mRow = nullptr;
		if (!mask.empty()) 
			mRow = mask.ptr<uchar>(i);

		for (int j = 0; j<cols; ++j)
		{
			if (mRow && mRow[j]>0)
			{
				if (fRow[j]==0)
					rRow[j] = sRow[j];
				else if (sRow[j]==0)
					rRow[j] = fRow[j];
				else
					rRow[j] = sRow[j];
			}
		}
	}
}