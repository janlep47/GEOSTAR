// test.cpp
//
// by Janice Richards, Jul 12, 2017
//
// 1st try scaling a simple raster channel
//
// usage: test
//
//---------------------------------------------------------
#include <string>
#include <iostream>
#include <cstdint>
#include <vector>

#include "geostar.hpp"                //TEMP!!

#include "boost/filesystem.hpp"     //TEMP!!


int main() {
    
    using namespace std;
    
    GeoStar::File *file = new GeoStar::File("a1.h5","existing");
    GeoStar::Image *img = file->open_image("ers1");
    GeoStar::Raster *ras = img->open_raster("chan1");
    
    int dataRows, dataCols;
    dataCols = ras->get_nx();
    dataRows = ras->get_ny();
    cout << "The input channel has " << dataRows << " rows and " << dataCols << " columns." << endl;
    
    int newRows, newCols;
    cout << "Enter new number of rows and cols:" << endl;
    cin >> newRows >> newCols;
    cout << "You entered rows = " << newRows << " and cols = " << newCols << endl;
    
    // Now, create the new output raster/image/file:
    GeoStar::File *fileNew = new GeoStar::File("a1.h5","existing");
    GeoStar::Image *imgNew = fileNew->create_image("ers2");  // New image
    GeoStar::Raster *rasNew = imgNew->create_raster("chan1", GeoStar::INT8U, newCols, newRows);

    //GeoStar::Raster *rasNew = ras->scale(imgNew,"chan1",GeoStar::INT8U, newCols,newRows);
    ras->scale(rasNew,"chan1",GeoStar::INT8U, newCols,newRows);
    
    return 0;
}
