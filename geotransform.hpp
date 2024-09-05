#pragma once

#include <gdal.h>
#include <gdal_priv.h>
#include <ogr_spatialref.h>
#include <ogr_geometry.h>
#include <filesystem>
#include <iostream>

class GeoTransform 
{
public:
    GeoTransform(double scaleX, double scaleY, int srssEPSG, int outtEPSG, OGRPoint upp_left_coord);
    ~GeoTransform(void);
    int EPSGdefiner(GDALDataset *ds);
    void GeoConverter(std::string path, int col, int row);
protected:
    OGRPoint TransformPoint(double x, double y, OGRSpatialReference* srs, OGRSpatialReference* outsrs);
    void GeoConverterEPSG_4326(GDALDataset *image, char* wkt);

    OGRSpatialReference outsrs;
    OGRSpatialReference srs;
    OGRPoint upper_left_coord;
    double x_scale;
    double y_scale;
    int srsEPSG;
    int outEPSG;
};