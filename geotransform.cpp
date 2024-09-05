#include "geotransform.hpp"

//namespace fs = std::filesystem;

GeoTransform::GeoTransform(double scaleX, double scaleY, int srssEPSG, int outtEPSG, OGRPoint upp_left_coord)
{
    x_scale = scaleX;
    y_scale = scaleY;
    srsEPSG = srssEPSG;
    outEPSG = outtEPSG;
    upper_left_coord = upp_left_coord;
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

void GeoTransform::GeoConverter (std::string path, int col, int row) {

    GDALAllRegister();

    // designating of a final coordinate system
    outsrs.importFromEPSG(outEPSG);
    char* wkt = nullptr;
    outsrs.exportToWkt(&wkt);

    // designating of an original (start) coordinate system
    srs.importFromEPSG(srsEPSG);

    upper_left_coord = TransformPoint(4174448.528, 7443410.282, &srs, &outsrs);

    // opening of the tiff
    GDALDataset *image = (GDALDataset*)GDALOpen(path.c_str(), GA_Update);

    int x_count_of_rastr = image->GetRasterXSize();
    int y_count_of_rastr = image->GetRasterYSize();
    
    upper_left_coord.setX(upper_left_coord.getX()+x_count_of_rastr*x_scale*col);
    upper_left_coord.setY(upper_left_coord.getY()-y_count_of_rastr*y_scale*row);

    if (outEPSG == 4326){
    GeoConverterEPSG_4326(image, wkt);
    }

    // recording of athenian coefficients
    double TiePointTags[] = { upper_left_coord.getX(), x_scale, 0, upper_left_coord.getY(), 0, -y_scale };

    // setting geographical reference
    image->SetGeoTransform(TiePointTags);
    // setting final EPSG format
    image->SetProjection(wkt);
    // closing of the file
    GDALClose(image);
}