// Raster.cpp
//
// by Leland Pierce, Feb 18, 2017
//
//--------------------------------------------


#include <string>
#include <iostream>
#include <vector>
#include <array>

#include "H5Cpp.h"

#include "Image.hpp"
#include "Raster.hpp"
#include "Exceptions.hpp"
#include "attributes.hpp"

namespace GeoStar {

  Raster::Raster(Image *image, const std::string &name){
    RasterOpenErrorException RasterOpenError;
    RasterDoesNotExistException RasterDoesNotExist;

    if(!image->datasetExists(name)) throw RasterDoesNotExist;

    // check if its a valid Raster:
    rasterobj = new H5::DataSet(image->openDataset(name));
    if(read_object_type() != "geostar::raster") {
      delete rasterobj;
      throw RasterOpenError;
    }//endif

    // finish setting object-specific data:
    rastername = name;
    rastertype = "geostar::raster";
    
  }// end-Raster-constructor




  Raster::Raster(Image *image, const std::string &name, const RasterType &type,
           const int &nx, const int &ny){

    RasterCreationErrorException RasterCreationError;
    RasterExistsException RasterExistsError;

    if(image->datasetExists(name)) throw RasterExistsError;

    // create a 2D dataset
    hsize_t dims[2];
    dims[0] = ny; 
    dims[1] = nx; 
    H5::DataSpace dataspace(2, dims);

    switch(type) {
    case INT8U:
      rasterobj = new H5::DataSet(image->createDataset(name, H5::PredType::NATIVE_UINT8, dataspace));
      break;
    case INT16U:
      rasterobj = new H5::DataSet(image->createDataset(name, H5::PredType::NATIVE_UINT16, dataspace));
      break;
    case REAL32:
      rasterobj = new H5::DataSet(image->createDataset(name, H5::PredType::NATIVE_FLOAT, dataspace));
      break;
    default:
      throw RasterCreationError;
    }// end case

    rastername = name;
    raster_datatype=type;
    rastertype = "geostar::raster";
    
    // set objtype attribute.
    write_object_type(rastertype);

  }// end-Raster-constructor

  










  // write data to the Raster 
  void Raster::write(const std::vector<long int> slice, const GeoStar::RasterType &type, void *buffer) {

    RasterWriteErrorException RasterWriteError;
    SliceSizeException SliceSizeError;

    // slice needs to have: x0, y0, dx, dy
    if(slice.size() < 4) throw SliceSizeError;
    
    // size of the slice of data is the SAME as the size of the slice in the file:
    hsize_t memdims[2];
    memdims[0]=slice[3];
    memdims[1]=slice[2];
    H5::DataSpace memspace(2,memdims);

    H5::DataSpace dataspace = rasterobj->getSpace();
    
    // set the slice within the file's dataset we want to write to:
    hsize_t count[2];
    hsize_t start[2];
    start[0]=slice[1];
    start[1]=slice[0];
    count[0]=slice[3];
    count[1]=slice[2];
    dataspace.selectHyperslab(H5S_SELECT_SET, count, start);
    
    switch(type){
    case INT8U:
      rasterobj->write( buffer, H5::PredType::NATIVE_UINT8, memspace, dataspace );
      break;
    case INT16U:
      rasterobj->write( buffer, H5::PredType::NATIVE_UINT16, memspace, dataspace );
      break;
    case REAL32:
      rasterobj->write( buffer, H5::PredType::NATIVE_FLOAT, memspace, dataspace );
      break;
    default:
      throw RasterWriteError;
    }//end-switch

  }// end: write





  void Raster::read(const std::vector<long int> slice, const GeoStar::RasterType &type, void *buffer) {

    RasterReadErrorException RasterReadError;
    SliceSizeException SliceSizeError;

    // slice needs to have: x0, y0, dx, dy
    if(slice.size() < 4) throw SliceSizeError;

    // size of the slice of data is the SAME as the size of the slice in the file:
    hsize_t memdims[2];
    memdims[0]=slice[3];
    memdims[1]=slice[2];
    H5::DataSpace memspace(2,memdims);

    H5::DataSpace dataspace = rasterobj->getSpace();
    
    // set the slice within the file's dataset we want to write to:
    // we want an offset of (50,50), and a size of 100X100
    hsize_t count[2];
    hsize_t start[2];
    start[0]=slice[1];
    start[1]=slice[0];
    count[0]=slice[3];
    count[1]=slice[2];
    dataspace.selectHyperslab(H5S_SELECT_SET, count, start);
    
    switch(type){
    case INT8U:
      rasterobj->read( buffer, H5::PredType::NATIVE_UINT8, memspace, dataspace );
      break;
    case INT16U:
      rasterobj->read( buffer, H5::PredType::NATIVE_UINT16, memspace, dataspace );
      break;
    case REAL32:
      rasterobj->read( buffer, H5::PredType::NATIVE_FLOAT, memspace, dataspace );
      break;
    default:
      throw RasterReadError;
    }//end-switch
  } // end: read


  //returns the actual size of the raster in the x-direction
  long int Raster::get_nx() const {
    H5::DataSpace space=rasterobj->getSpace();
    hsize_t dims[2];
    space.getSimpleExtentDims(dims);
    return dims[1];
  }// end: get_nx

  //returns the actual size of the raster in the y-direction
  long int Raster::get_ny() const {
    H5::DataSpace space=rasterobj->getSpace();
    hsize_t dims[2];
    space.getSimpleExtentDims(dims);
    return dims[0];
  }// end: get_ny

  // in-place simple threshhold
  // < value : set to 0.
  void Raster::thresh(const double &value) {
    long int nx = get_nx();
    long int ny = get_ny();

    vector<long int>slice(4);
    slice[0]=0;
    slice[1]=0;
    slice[2]=nx;
    slice[3]=1;

    uint8_t *data = new uint8_t[nx];

    for(int line=0;line<ny;++line) {
      slice[1]=line;
      read(slice,GeoStar::INT8U,data);
      for(int pixel=0; pixel<nx;++pixel) {
        if(data[pixel] < value) data[pixel]=0;
      }// endfor: pixel
      write(slice,GeoStar::INT8U,data);
    }// endfor: line

    delete[] data;
  }// end: thresh
    
    
    
    void *Raster::scale(GeoStar::Raster *rasNew, const std::string &name, const RasterType &type,
                          const int &nx, const int &ny) {
        //uint8_t data[100*100];
        //std::vector<uint8_t> data(10000);
        //std::array<uint8_t, 10000> data;

        //Raster *rasNew = imgNew->create_raster(name, type, nx, ny);
        dataCols = get_nx();
        dataRows = get_ny();
        
        cout << "The input channel has " << dataRows << " rows and " << dataCols << " columns." << endl;
        
        //int newRows, newCols;
        int newRows, newCols;
        newRows = ny;
        newCols = nx;
        cout << "The SCALED channel has rows = " << newRows << " and cols = " << newCols << endl;
        // uint8_t data[100*100];       // Old data;  this will hold the current "slice"
        uint8_t newData[newCols];       // New data;  this will hold ONE ROW of the new image
        
        //std::vector<long int> sliceOut(4);
        //std::vector<long int> sliceInput(4);
        //sliceOut = new std::vector<long int>(4);
        //sliceInput = new std::vector<long int>(4);
        std::vector<long int> sliceOut(4);
        std::vector<long int> sliceInput(4);


        // Read in initial slice
        sliceInput[0]=0; sliceInput[1]=0;      // x0, y0  (column/row)
        sliceInput[2]=dataCols; sliceInput[3]=3;  // dx, dy
        
        // Now, set up the slice paramters for the output data:
        sliceOut[0]=0; sliceOut[1]=0;      // x0, y0  (column/row)
        sliceOut[2]=newCols; sliceOut[3]=1;  // dx, dy (#columns/#rows)
        
        
        double rowScaler, colScaler;
        double xScaled, yScaled;
        int oldRow, oldCol;
        double xDiff, yDiff;
        uint8_t pixelVal;
        
        rowScaler = (dataRows*1.0) / newRows;
        colScaler = (dataCols*1.0) / newCols;
        
        cout << "rowScaler = " << rowScaler << " colScaler = " << colScaler << endl;
        for (int i = 0; i < newRows; i++) {
            yScaled = i*rowScaler;
            oldRow = static_cast<int> (yScaled);
            sliceInput[1] = oldRow - 1;
            if (sliceInput[1] < 0) sliceInput[1] = 0;
            
            // Verify our currently read-in slice includes the necessary pixels for doing this scale;
            //  re-read in as necessary
            // verifySliceOK(r,c,xDiff,yDiff);
            try {
                read(sliceInput,GeoStar::INT8U,data);
            } catch (const H5::DataSetIException& e) {
                cerr << e.getCDetailMsg() << endl;
            }
            
            xDiff = yScaled - static_cast<double>(oldRow);
            getNearest(oldRow, yDiff);
            
            for (int j = 0; j < newCols; j++) {
                xScaled = j*colScaler;
                oldCol = static_cast<int> (xScaled);
                xDiff = xScaled - static_cast<double>(oldCol);
                getNearest(oldCol, xDiff);
                pixelVal = getScaledUintPixel(oldRow, oldCol, xDiff, yDiff, sliceInput);
                newData[j] = pixelVal;
            }
            // Current row is scaled, write it to the output raster
            rasNew->write(sliceOut,GeoStar::INT8U,newData);
            sliceOut[1] = i;
        }
        return rasNew;
    }
    
    void Raster::getNearest(int& p, double& diff) {
        if (diff > 0.5) {
            p += 1;
            diff = diff - 1.0;
        }
    }
    
    //
    //    m1,n .         o      .  m,n
    //                   |
    //                   X
    //                   |
    //                   |
    //                   |
    //                   |
    //   m1,n1 .         o      .  m,n1
    //
    
    uint8_t Raster::getScaledUintPixel(int r, int c, double xDiff, double yDiff, std::vector<long int> &sliceInput) {
        using namespace std;
        //
        // sliceUL_row is the row of the original image, represented by the slice at row 0
        // sliceUL_col is the column of the original image, represented by the slice at column 0
        //
        //int m = r - sliceUL_row;
        //int n = c - sliceUL_col;
        int m = r - sliceInput[1];
        int n = c - sliceInput[0];
        int m1, n1;
        
        if (xDiff < 0) {
            m1 = m - 1;
            xDiff *= -1.0;
            if (m1 < 0) m1 = 0;
        } else {
            m1 = m + 1;
            //if (m1 >= sliceRows) m1 = m;
            if (m1 >= sliceInput[3]) m1 = m;
        }
        if (yDiff < 0) {
            n1 = n - 1;
            yDiff *= -1.0;
            if (n1 < 0) n1 = 0;
        } else {
            n1 = n + 1;
            //if (n1 >= sliceCols) n1 = n;
            if (n1 >= sliceInput[2]) n1 = n;
        }
        uint8_t pixelValX1, pixelValX2, pixelVal;
        //pixelValX1 = ((-slice[m][n] + slice[m1][n]) * xDiff) + slice[m][n];
        //pixelValX2 = ((-slice[m][n1] + slice[m1][n1]) * xDiff) + slice[m][n1];
        pixelValX1 = ((-dataAt(m,n) + dataAt(m1,n)) * xDiff) + dataAt(m,n);
        pixelValX2 = ((-dataAt(m,n1) + dataAt(m1,n1)) * xDiff) + dataAt(m,n1);
        pixelVal = ((-pixelValX1 + pixelValX2) * yDiff) + pixelValX1;
        
        return pixelVal;
    }
    
    
    
    uint8_t Raster::dataAt(int m, int n) {
        return data[m*dataCols + n];
    }
    
    




}// end namespace GeoStar
