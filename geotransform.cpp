#include "geotransform.hpp"

GeoTransform::GeoTransform(int srssEPSG, int outtEPSG)
{
    srsEPSG = srssEPSG;
    outEPSG = outtEPSG;
}

GeoTransform::~GeoTransform(void)
{
}

// setting the EPSG value from existing tiffs
int GeoTransform::EPSGdefiner(GDALDataset *ds){

    if (ds == nullptr) {
        std::cout << "Failed to open the dataset." << std::endl;
        return 0;
    }

    const char *wkt = ds->GetProjectionRef();
    OGRSpatialReference proj;
    proj.importFromWkt(&wkt);
    return proj.GetEPSGGeogCS();
}

// function of converting image points (upper-left and lower-right corners) from one coordinate 
// system to another
OGRPoint GeoTransform::TransformPoint(double x, double y, OGRSpatialReference* srs, OGRSpatialReference* outsrs) {
    OGRPoint point(x, y);
    point.assignSpatialReference(srs);
    point.transformTo(outsrs);
    return point;
}

void GeoTransform::GeoConverterEPSG_4326 (GDALDataset *image, char* wkt){
    // recording of athenian coefficients
    double TiePointTags[] = { upper_left_coord.getY(), -y_scale, 0, upper_left_coord.getX(), 0, x_scale };

    // setting geographical reference
    image->SetGeoTransform(TiePointTags);
    // setting final EPSG format
    image->SetProjection(wkt);
    // closing of the file
    GDALClose(image);
}

void GeoTransform::FromLatLonToMetres(double lat, double lon, double & x, double & y, std::string projection)
{
    Math::real lon0 = floor(lon/6)*6+3;
    if (projection == "UTM"){
        UTM->Forward(lon0, lat, lon, x, y);
        x+=500000;
    } else {
        Gauss_Kruger->Forward(lon0, lat, lon, x, y);
        x+=7500000;
    }
}

void GeoTransform::FromMeteresToLatLon(Math::real lon0, Math::real x, Math::real y, Math::real &lat, Math::real &lon, std::string projection)
{
    if (projection=="UTM"){
        x-=500000;
        UTM->Reverse(lon0, x, y, lat, lon);
    } else {
        x-=7500000;
        Gauss_Kruger->Reverse(lon0, x, y, lat, lon);
    }
}

void GeoTransform::ScaleCounter(std::vector<cv::Mat>& movems)
{
    if (displacement.empty()){
        return;
    }
    int k = 1;
    //устанавливаем начало координат матриц смещений кадров в (0;0): начало координат - центр 1-го кадра
    for (auto i : movems){
        i.at<double>(0,2)-=movems[0].at<double>(0,2);
        i.at<double>(1,2)-=movems[0].at<double>(1,2);
    }
    //переменные для вычисления среднего смещения кадров за 1 секунду
    double x_average = 0;
    double y_average = 0;
    x_scale = 0;
    y_scale = 0;
    for (int i = 0; i < movems.size(); ++i){
        x_average+= movems[i].at<double>(0,2);
        y_average+= movems[i].at<double>(1,2);
        if (i % (camera_freq-1) == 0){
            //смещаем начало координат в точку, где произошло прошлое измерение смещения (смотрим каждый раз на смещение только за 1 с) 
            displacement[k].setX(displacement[k].getX()-displacement[k-1].getX());
            displacement[k].setY(displacement[k].getY()-displacement[k-1].getY());
            //суммируем все вычисленные масштабы для каждой секунды
            x_scale+= abs(displacement[k].getX() / (x_average/camera_freq));
            y_scale+= abs(displacement[k].getY() / (y_average/camera_freq));
            ++k;
            x_average = 0;
            y_average = 0;
            //перемещаем начало координат для матриц смещений кадров аналогично данным displacement, полученным с датчиков
            for (int j = i+1; j < movems.size(); ++j){
                movems[j].at<double>(0,2)-=movems[i+1].at<double>(0,2);
                movems[j].at<double>(1,2)-=movems[i+1].at<double>(1,2);
            } 
        }
    }
    //вычисляем среднее значение масштаба за все время
    x_scale/=k;
    y_scale/=k;
}

void GeoTransform::SetSRTInfo(std::pair<double, double> &reper_center_coord, std::vector<OGRPoint> &srt_displacement)
{
    displacement = srt_displacement;
    //получаем координаты репера в метрах
    FromLatLonToMetres(reper_center_coord.first, reper_center_coord.second, center_coord_utm.first, center_coord_utm.second, "UTM");
    //вычисляем абсолютные координаты центра первого кадра в метрических координатах UTM
    center_coord_utm.first += displacement[0].getX(); 
    center_coord_utm.second += displacement[0].getY(); 
}

void GeoTransform::GeoConverter (std::string path, int col, int row, OGRPoint upp_left_coord) {

    GDALAllRegister();

    // designating of a final coordinate system
    outsrs.importFromEPSG(outEPSG);
    char* wkt = nullptr;
    outsrs.exportToWkt(&wkt);

    // designating of an original (start) coordinate system
    srs.importFromEPSG(srsEPSG);

    // opening of the tiff
    GDALDataset *image = (GDALDataset*)GDALOpen(path.c_str(), GA_Update);

    int x_count_of_rastr = image->GetRasterXSize();
    int y_count_of_rastr = image->GetRasterYSize();

    if (center_coord_utm.first !=0 && center_coord_utm.second !=0){
        upper_left_coord.setX(center_coord_utm.first  - (x_scale*x_count_of_rastr)/2);
        upper_left_coord.setY(center_coord_utm.second + (y_scale*y_count_of_rastr)/2);
        upper_left_coord = TransformPoint(upper_left_coord.getX(), upper_left_coord.getY(), &srs, &outsrs);
    }
    else { 
        upper_left_coord = TransformPoint(upp_left_coord.getX() , upp_left_coord.getY() , &srs, &outsrs);
    }
    
    upper_left_coord.setX(upper_left_coord.getX() + x_count_of_rastr*x_scale*col);
    upper_left_coord.setY(upper_left_coord.getY() - y_count_of_rastr*y_scale*row);

    if (outEPSG == 4326){
        GeoConverterEPSG_4326(image, wkt);
    }

    // recording of athenian coefficients
    double TiePointTags[] = { upper_left_coord.getX(), x_scale, 0, upper_left_coord.getY(), 0, -y_scale };

    // setting geographical reference
    image->SetGeoTransform(TiePointTags);
    // setting final EPSG format
    image->SetProjection(wkt);
    // closing the file
    GDALClose(image);
}