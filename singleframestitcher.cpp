#include "singleframestitcher.hpp"

#include <iostream>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <sstream>
#include <direct.h> 

using namespace std;
using namespace cv;
//переписанный метод CalculateSize - считает кол-во изображений, на котрорых будут размещены склеенные кадры (если при вводе не были
//указаны размеры по х и по y в пикселях, то все склеивается в 1 изображение по умолчанию) и ничего не возвращает
//т.к. размеры изображений(-ия) и кол-во изображений являются полями класса SingleFrameStitcher
void SingleFrameStitcher::ImageCounter(std::vector<cv::Mat> &movems, cv::Size imSize, cv::Size resultImageSize)
{
	double maxX=imSize.width/2, maxY=imSize.height/2, minX=-imSize.width/2, minY=-imSize.height/2;
	int sizeX, sizeY;

	if (movems.empty())
		return;

	for (auto const & cadr : movems)
	{
		//отдельно вынесен метод расчет координат углов кадров относительно центров кадров
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
	//в вычислении размера полного изображения добавлено округление в большую сторону (так размещенные кадры точно не должны обрезаться
	//- видно на примере склейки test1.avi)
	sizeX= (int)ceil(maxX-minX);
	sizeY= (int)ceil(maxY-minY);
	m_origin.x = (int)floor(minX + (double)imSize.width/2);
	m_origin.y = (int)floor(minY + (double)imSize.height/2);

	if (!(resultImageSize == Size(0,0))){
		m_singleImageSize = resultImageSize;
	} else {
		m_singleImageSize = cv::Size(sizeX, sizeY);
	}

	count_x = (int)ceil(sizeX/(double)m_singleImageSize.width);
	count_y = (int)ceil(sizeY/(double)m_singleImageSize.height);

	cout << "Count of images will be " << count_x*count_y << " with size " << m_singleImageSize.width << "x" << m_singleImageSize.height << endl;
	cout << "with origin point (" <<m_origin.x << "," << m_origin.y << ")" << endl;

}

std::vector<double> SingleFrameStitcher::CornerCoordinatesCounter(const cv::Mat &cadr, cv::Size imSize){

	double relativeMaxX = 0, relativeMinX = 0, relativeMaxY = 0, relativeMinY = 0;

	vector<double> corners_coordinates_x;
	vector<double> corners_coordinates_y;
	vector<double> relative_max_min_x_y;

	//вектор заполняется значениями координат углов повернутого кадра при помощи афинного преобразования
	corners_coordinates_x.push_back(((double)imSize.width/2)*cadr.at<double>(0,0) +  ((double)imSize.height/2)*cadr.at<double>(0,1));
	corners_coordinates_x.push_back((-(double)imSize.width/2)*cadr.at<double>(0,0) +  ((double)imSize.height/2)*cadr.at<double>(0,1));
	corners_coordinates_x.push_back((-(double)imSize.width/2)*cadr.at<double>(0,0) +  (-(double)imSize.height/2)*cadr.at<double>(0,1));
	corners_coordinates_x.push_back(((double)imSize.width/2)*cadr.at<double>(0,0) +  (-(double)imSize.height/2)*cadr.at<double>(0,1));

	//находим максимальное и минимальное значения среди посчитанных координат
	for (int i = 0; i < corners_coordinates_x.size(); ++i){
		if (corners_coordinates_x[i] > relativeMaxX)
			relativeMaxX = corners_coordinates_x[i];
		if (corners_coordinates_x[i] < relativeMinX)
			relativeMinX = corners_coordinates_x[i];
	}

	//добавляем значения в возвращаемый вектор
	relative_max_min_x_y.push_back(relativeMaxX);
	relative_max_min_x_y.push_back(relativeMinX);

	//аналогично х
	corners_coordinates_y.push_back(((double)imSize.width/2)*cadr.at<double>(1,0) +  ((double)imSize.height/2)*cadr.at<double>(1,1));
	corners_coordinates_y.push_back((-(double)imSize.width/2)*cadr.at<double>(1,0) +  ((double)imSize.height/2)*cadr.at<double>(1,1));
	corners_coordinates_y.push_back((-(double)imSize.width/2)*cadr.at<double>(1,0) +  (-(double)imSize.height/2)*cadr.at<double>(1,1));
	corners_coordinates_y.push_back(((double)imSize.width/2)*cadr.at<double>(1,0) +  (-(double)imSize.height/2)*cadr.at<double>(1,1));

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

	for (auto & move : movems)
	{
		move=start* move;
		//отдельно вынесен метод заполнения image_numbers, в котором хранятся номера изображений (где первый номер - номер столбца,
		//второй-номер строки), на которых возможно расположение кадра (более точного метода определения изображений, к которым может 
		//принадлежать кадр пока не удалось создать). работает даже если задавать размеры изображений меньше размера кадра 
		ImageNumberFiller (move, imSize);
	}
}

void SingleFrameStitcher::ImageNumberFiller(cv::Mat &move, cv::Size imSize){

    std::vector<double> relativeCoordinates = CornerCoordinatesCounter (move, imSize);

	int x_center_coordinate = (int)(move.at<double>(0,2) + (double)imSize.width/2)  % m_singleImageSize.width;
	int y_center_coordinate = (int)(move.at<double>(1,2) + (double)imSize.height/2) % m_singleImageSize.height;
	int image_number_x =(int)(move.at<double>(0,2) + (double)imSize.width/2) / m_singleImageSize.width;
	int image_number_y = (int)(move.at<double>(1,2) + (double)imSize.height/2) / m_singleImageSize.height;

	//величина, обозначающая на сколько изображений по х (в положительном направлении, относительно изображения, где расположен центр 
	//кадра) размещенный кадр "вылезает" за границы изображения, где расположен центр кадра 
	int over_x = (x_center_coordinate+(int)relativeCoordinates[0])/m_singleImageSize.width;
	if ((x_center_coordinate+(int)relativeCoordinates[0]) % m_singleImageSize.width == 0){
		over_x-=1;
	}
	//аналогично over_x, но только в отрицательном направлении
	int under_x = 0;
	if (x_center_coordinate+(int)relativeCoordinates[1] < 0) {
		if ((x_center_coordinate+(int)relativeCoordinates[1]) % m_singleImageSize.width == 0){
			under_x = (x_center_coordinate+(int)relativeCoordinates[1])/m_singleImageSize.width;
		} else {
			under_x = (x_center_coordinate+(int)relativeCoordinates[1])/m_singleImageSize.width - 1;
		}

	}
	//аналогично over_x, но только для y
	int over_y = (y_center_coordinate+(int)relativeCoordinates[2])/m_singleImageSize.height;
	if ((y_center_coordinate+(int)relativeCoordinates[2]) % m_singleImageSize.height == 0){
		over_y-=1;
	}

	int under_y = 0;
	if (y_center_coordinate+(int)relativeCoordinates[3] < 0){
		if ((y_center_coordinate+(int)relativeCoordinates[3]) % m_singleImageSize.height == 0){
            under_y = (y_center_coordinate+(int)relativeCoordinates[3])/m_singleImageSize.height;
		} else {
			under_y = (y_center_coordinate+(int)relativeCoordinates[3])/m_singleImageSize.height - 1;
		}
	}
	//заполняем image_number номерами изображений, где может быть расположен кадр (каждому кадру соответсвует свой вектор номеров изображений)
	image_number.push_back({});

	for (int i = under_x; i <= over_x; ++i){
		for (int j = under_y; j <= over_y; ++j){
			image_number[iterator].push_back(make_pair(i+image_number_x,j+image_number_y));
		}
	}
	iterator++;
	return;
}

void SingleFrameStitcher::CreatePanno(cv::Mat image, cv::Mat origin){
	//создаем основу для расположения кадров
	//метод Mat(m_singleImageSize, CV_8UC1) был заменен на Mat::zeros(m_singleImageSize, CV_8UC1), т.к. если не прописывать
	//какие значения должны хранится в матрице, то для CV_8UC1 по умолчанию создается серое изображение(конкретно на моем устройстве)
	for (int i = 0; i < count_x; ++i){
		m_result.push_back({});
		for (int j =0; j < count_y; ++j){
			m_result[i].push_back(Mat::zeros(m_singleImageSize, CV_8UC1));
		}
	}
	iterator = 0;
	cv::Mat current, tmp, mask;
	cv::cvtColor(image, current, cv::COLOR_BGR2GRAY);
	//перебирая все номера изображений, доклеиваем к ним части кадра, т.е. проходим однократно по всем кадрам и многократно по изображениям
	for (auto i : image_number[iterator]){
		cv::Mat new_origin = origin.clone();
		new_origin.at<double>(0,2) -= m_singleImageSize.width*i.first;
		new_origin.at<double>(1,2) -= m_singleImageSize.height*i.second;
		cv::warpPerspective(current,tmp, new_origin, m_singleImageSize);
	    auto kernElem=cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3,3));
	    cv::compare(tmp,0,mask,cv::CMP_GT);
	    cv::erode(mask,mask, kernElem, cv::Point(-1,-1),5); 
	    cv::add(tmp,0,m_result[i.first][i.second],mask);
	}
}

void SingleFrameStitcher::AppendToPanno(cv::Mat image, cv::Mat origin)
{
	if (m_result.empty())
		throw std::runtime_error("Panno not created yet!");
	Mat grey, tmp, mask;
	cv::cvtColor(image, grey, cv::COLOR_BGR2GRAY);
	++iterator;
	for (auto i : image_number[iterator]){
		cv::Mat trans = origin.clone();
	    trans.pop_back();
		trans.at<double>(0,2) -= m_singleImageSize.width*i.first;
		trans.at<double>(1,2) -= m_singleImageSize.height*i.second;
	    cv::warpAffine(grey, tmp, trans, m_singleImageSize);	
	    cv::compare(tmp,0,mask, cv::CMP_GT);
	    auto kernElem=cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3,3));
	    cv::erode(mask,mask, kernElem, cv::Point(-1,-1),5);
	    cv::Mat tmp2;
	    cv::compare(m_result[i.first][i.second],0,tmp2,cv::CMP_GT);
	    mask = mask & (~tmp2);
	    makeSumm(tmp, m_result[i.first][i.second], m_result[i.first][i.second], mask);
	}
}

void SingleFrameStitcher::SaveImage(std::string filename, double scaleX, double scaleY, 
	int srsEPSG, int outEPSG, OGRPoint upper_left_coord)
{
	GeoTransform geos(scaleX, scaleY, srsEPSG, outEPSG, upper_left_coord);
	//создаем папку result, в которой будут храниться результаты работы программы
	//если папка уже была создана, то файлы будут сохраняться в той же папке
	if(_mkdir("./result") == -1)
        cerr << " Error Occured : " << strerror(errno) << endl;
    else
        cout << "Directory Created";
	for (int i = 0; i < count_x; ++i){
		for (int j = 0; j < count_y; ++j){
			std::stringstream out_name;
			out_name << "./" << "result/" << filename << ".tiff";
			cv::imwrite( out_name.str(), m_result[i][j]);
			geos.GeoConverter(out_name.str(), i, j);
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