// Raster.hpp
//
// by Leland Pierce, Feb 18, 2017
//
//----------------------------------------
#ifndef RASTER_HPP_
#define RASTER_HPP_


#include <string>
#include <vector>

#include "H5Cpp.h"

#include "RasterType.hpp"
#include "attributes.hpp"

//#include "geostar.hpp"
//#include "boost/filesystem.hpp"

namespace GeoStar {
  class Image;
  
  class Raster {

  private:
    std::string rastername;
    std::string rastertype;
    RasterType  raster_datatype;
      
      long int dataCols;
      long int dataRows;
      uint8_t data[100*100];
      //std::array<uint8_t, 10000> data;
      
      void getNearest(int& p, double& diff);
      uint8_t getScaledUintPixel(int r, int c, double xDiff, double yDiff, std::vector<long int> &sliceInput);
      uint8_t dataAt(int m, int n);

  public:
    H5::DataSet *rasterobj;

    // opening an existing Raster
    // non-existence is an error
    Raster(Image *image, const std::string &name);

    // creating a new Raster
    // existence is an error
    Raster(Image *image, const std::string &name, const RasterType &type,
           const int &nx, const int &ny);

    // write the "object_type" attribute
    inline void write_object_type(const std::string &value) {
      GeoStar::write_object_type((H5::H5Location *)rasterobj,value);
    }

    // read the "object_type" attribute
    std::string read_object_type() const {
      return GeoStar::read_object_type((H5::H5Location *)rasterobj);
    }

    // cleans up the H5::DataSet object 
    inline ~Raster() {
      delete rasterobj;
    }

    // write data from a C-array to a slice in the DataSet in the file.
    // size of slice is the same for both C-array and DataSet
    // slice definition: x0, y0, dx, dy
    // pass in whatever type buffer you are using. 
    // It should be consistent with the RasterType you pass in.
    // indexing in buffer: [iy*nx+ix]
    void write(const std::vector<long int> slice, const RasterType &type, void *buffer);

    // read data to a C-array from a slice in the DataSet in the file.
    // size of slice is the same for both C-array and DataSet
    // slice definition: x0, y0, dx, dy
    // pass in whatever type buffer you are using. 
    // It should be consistent with the RasterType you pass in.
    // indexing in buffer: [iy*nx+ix]
    void read (const std::vector<long int> slice, const RasterType &type, void *buffer);

    //returns the actual size of the image in the x-direction
    long int get_nx() const;

    //returns the actual size of the image in the y-direction
    long int get_ny() const;

    // in-place simple threshhold
    // < value : set to 0.
    void thresh(const double &value);

    // read a single-channel TIFF image
    // infile is the name of the intput TIFF file
    // name is the string to identify the new raster that is created
    // if the image is more than 1 channel, it just uses the first.
    // for now, it creates a 8U channel.
    void read_tiff_1_8U(const std::string &infile, const std::string &name);

    // creates a new Raster (dataset) that's a scaled copy of this Raster
    void *scale(GeoStar::Raster *rasNew,
                  const std::string &name,
                  const RasterType &type,
                  const int &nx, const int &ny);

  }; // end class: Raster
  
}// end namespace GeoStar


#endif //RASTER_HPP_
