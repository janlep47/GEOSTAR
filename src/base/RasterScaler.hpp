// RasterScaler.hpp
//
// by Janice Richards, Aug 11, 2017
//
//----------------------------------------
#ifndef RASTERSCALER_HPP_
#define RASTERSCALER_HPP_


#include <string>
#include <vector>

#include "H5Cpp.h"

#include "RasterType.hpp"
#include "Raster.hpp"
#include "attributes.hpp"

//#include "geostar.hpp"
//#include "boost/filesystem.hpp"

namespace GeoStar {
  
    class Raster;

    template<typename T>
  class RasterScaler {

  private:
      Raster *scaleFrom;
      T *data;
      long int dataCols;
      long int dataRows;

      void getNearest(int& p, double& diff);
      T getScaledUintPixel(int r, int c, double xDiff, double yDiff, std::vector<long int> &sliceInput, T *data);
      T dataAt(int m, int n, T *data);

  public:
      void junk();

    // creating a new RasterScaler
    RasterScaler(Raster *source);


    // cleans up the RasterScaler data
    inline ~RasterScaler() {
      delete data;
    }

    // creates a new Raster (dataset) that's a scaled copy of this Raster
    void scale(Raster *rasNew, const std::string &name, const RasterType &type, const int &nx, const int &ny);

  }; // end class: RasterScaler
  

    
    
    

    template<typename T>
    void RasterScaler<T>::junk(){
        std::cout<< "YAY ... here in junk() " << std::endl;
    }

    
    
    template<typename T>
    RasterScaler<T>::RasterScaler(GeoStar::Raster *source){
        scaleFrom = source;
    }
    
    /*
     template<typename T>
     RasterScaler::RasterScaler(Image *image, const std::string &name){
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
     
     
     
     template<typename T>
     RasterScaler::RasterScaler(Image *image, const std::string &name, const RasterType &type,
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
     */
    
    
    template<typename T>
    void RasterScaler<T>::scale(GeoStar::Raster *rasNew, const std::string &name, const RasterType &type,
                                const int &nx, const int &ny) {
        // below, if pass in GeoStar::Image *imgNew ... and return GeoStar::Raster *rasNew ...
        //   ... by creating rasNew HERE instead of before calling this method.
        //Raster *rasNew = imgNew->create_raster(name, type, nx, ny);
        dataCols = scaleFrom->get_nx();
        dataRows = scaleFrom->get_ny();
        
        std::cout << "The input channel has " << dataRows << " rows and " << dataCols << " columns." << std::endl;
        
        int newRows, newCols;
        newRows = ny;
        newCols = nx;
        std::cout << "The SCALED channel has rows = " << newRows << " and cols = " << newCols << std::endl;
        
        
        
        
        std::vector<T> data(10000);  // Old data;  this will hold the current "slice"
        //T newData[newCols];          // New data;  this will hold ONE ROW of the new image
        std::vector<T> newData(newCols);          // New data;  this will hold ONE ROW of the new image
        
        /*
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
         */
        
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
        T pixelVal;
        
        rowScaler = (dataRows*1.0) / newRows;
        colScaler = (dataCols*1.0) / newCols;
        
        std::cout << "rowScaler = " << rowScaler << " colScaler = " << colScaler << std::endl;
        for (int i = 0; i < newRows; i++) {
            yScaled = i*rowScaler;
            oldRow = static_cast<int> (yScaled);
            sliceInput[1] = oldRow - 1;
            if (sliceInput[1] < 0) sliceInput[1] = 0;
            
            // Verify our currently read-in slice includes the necessary pixels for doing this scale;
            //  re-read in as necessary
            // verifySliceOK(r,c,xDiff,yDiff);
            try {
                //scaleFrom->read(sliceInput,GeoStar::INT8U,&data[0]);
                scaleFrom->read(sliceInput,data);
            } catch (const H5::DataSetIException& e) {
                std::cerr << e.getCDetailMsg() << std::endl;
            }
            
            xDiff = yScaled - static_cast<double>(oldRow);
            getNearest(oldRow, yDiff);
            
            for (int j = 0; j < newCols; j++) {
                xScaled = j*colScaler;
                oldCol = static_cast<int> (xScaled);
                xDiff = xScaled - static_cast<double>(oldCol);
                getNearest(oldCol, xDiff);
                pixelVal = getScaledUintPixel(oldRow, oldCol, xDiff, yDiff, sliceInput, &data[0]);
                newData[j] = pixelVal;
            }
            // Current row is scaled, write it to the output raster
            // ********************
            //
            //
            //  later ... make into a switch statement, to write out the data type, depending on passed in
            //    "type"
            //         (parameter 'const RasterType &type')
            //
            //  later .... don't pass in 'data' as a parameter, but just use the private class member 'data'
            //   instead.
            //
            //rasNew->write(sliceOut,GeoStar::INT8U,newData);
            rasNew->write(sliceOut,newData);
            std::cout << "wrote out row: " << i << std::endl;
            sliceOut[1] = i;
        }
        //return rasNew;
        return;
    }
    
    template<typename T>
    void RasterScaler<T>::getNearest(int& p, double& diff) {
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
    
    template<typename T>
    T RasterScaler<T>::getScaledUintPixel(int r, int c, double xDiff, double yDiff, std::vector<long int> &sliceInput, T *data) {
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
        T pixelValX1, pixelValX2, pixelVal;
        //pixelValX1 = ((-slice[m][n] + slice[m1][n]) * xDiff) + slice[m][n];
        //pixelValX2 = ((-slice[m][n1] + slice[m1][n1]) * xDiff) + slice[m][n1];
        pixelValX1 = ((-dataAt(m,n,data) + dataAt(m1,n,data)) * xDiff) + dataAt(m,n,data);
        pixelValX2 = ((-dataAt(m,n1,data) + dataAt(m1,n1,data)) * xDiff) + dataAt(m,n1,data);
        pixelVal = ((-pixelValX1 + pixelValX2) * yDiff) + pixelValX1;
        
        return pixelVal;
    }
    
    
    
    template<typename T>
    T RasterScaler<T>::dataAt(int m, int n, T *data) {
        //return data[m*dataCols + n];
        return *(data + m*dataCols + n);
    }
    
    
    
    
    
    
}// end namespace GeoStar






#endif //RASTERSCALER_HPP_
