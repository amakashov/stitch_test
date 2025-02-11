#pragma once

#include <gdal.h>
#include <gdal_priv.h>
#include <opencv2/opencv.hpp>
#include <ogr_spatialref.h>
#include <ogr_geometry.h>
#include <filesystem>
#include <iostream>
#include <GeographicLib/TransverseMercator.hpp>

#define a_Krassov 6378245
#define f_Krassov 1/298.3
// #define a_PZ90 6378136
// #define f_PZ90 1/298.257839303
#define a_WGS84 6378137
#define f_WGS84 1/298.257223563

using namespace GeographicLib;

class GeoTransform 
{
public:
    // GeoTransform(int srssEPSG, int outtEPSG);
    GeoTransform(int srssEPSG, int outtEPSG);
    ~GeoTransform(void);
    int EPSGdefiner(GDALDataset *ds);
    void GeoConverter(std::string path, int col, int row, OGRPoint upp_left_coord, 
    std::pair<double,double> upper_left_coord_in_pixels);
    void ScaleCounter (std::vector<cv::Mat>& movems);
    void SetSRTInfo (std::pair<double, double> &reper_conter_coord, std::vector<OGRPoint> &srt_displacement);
    int srsEPSG;
    int outEPSG;
protected:
    OGRPoint TransformPoint(double x, double y, OGRSpatialReference* srs, OGRSpatialReference* outsrs);
    void GeoConverterEPSG_4326(GDALDataset *image, char* wkt);
    void FromLatLonToMetres (double lat, double lon, double & x, double & y, std::string projection);
    void FromMeteresToLatLon (Math::real lon0, Math::real x, Math::real y, Math::real &lat, Math::real &lon, std::string projection);
    
    TransverseMercator* UTM = new TransverseMercator(a_WGS84, f_WGS84, 0.9996);
    TransverseMercator* Gauss_Kruger = new TransverseMercator(a_Krassov, f_Krassov, 1);
    OGRSpatialReference outsrs;
    OGRSpatialReference srs;
    OGRPoint upper_left_coord;
    std::pair<double, double> center_coord_utm = std::make_pair(0, 0);
    std::vector<OGRPoint> displacement;
    double x_scale = 1;
    double y_scale = 4;
    // int srsEPSG;
    // int outEPSG;
    int camera_freq = 25; // Гц
};