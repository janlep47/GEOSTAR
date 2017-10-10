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
#include "Exceptions.hpp"


namespace GeoStar {
  class Image;
  
  class Raster {

  private:
    std::string rastername;
    std::string rastertype;
    RasterType  raster_datatype;
      
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

      
      //H5::PredType getHdf5Type(const RasterType &type);
      //template <RasterType T> static H5::PredType getHdf5Type() {return H5::PredType::NATIVE_UINT8;}
      template <typename T> static H5::PredType getHdf5Type() {return H5::PredType::NATIVE_UINT8;}
      
      // write data from a C-array to a slice in the DataSet in the file.
      // size of slice is the same for both C-array and DataSet
      // slice definition: x0, y0, dx, dy
      // pass in whatever type buffer you are using.
      // It should be consistent with the RasterType you pass in.
      // indexing in buffer: [iy*nx+ix]
      //
      // can call 'writenew(slice, bufr);'   don't need to say writenew<int>(slice, bufr); for example
      template<typename T>
      void write(const std::vector<long int> slice, std::vector<T> buffer) {
          
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
          
          int totalSize = (count[0] - start[0]) * (count[1] - start[1]);
          if (buffer.size() < totalSize) throw SliceSizeError;
          
          H5::PredType h5Type = Raster::getHdf5Type<T>();
          rasterobj->write( (void *)&buffer[0], h5Type, memspace, dataspace );
      }
      
      
      
      
      // read data to a C-array from a slice in the DataSet in the file.
      // size of slice is the same for both C-array and DataSet
      // slice definition: x0, y0, dx, dy
      // pass in whatever type buffer you are using.
      // It should be consistent with the RasterType you pass in.
      // indexing in buffer: [iy*nx+ix]
      template<typename T>
      void read(const std::vector<long int> slice, std::vector<T> buffer) {
          
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
          
          int totalSize = (count[0] - start[0]) * (count[1] - start[1]);
          if (buffer.size() < totalSize) buffer.resize(totalSize);
          
          H5::PredType h5Type = Raster::getHdf5Type<T>();
          rasterobj->read( (void *)&buffer[0], h5Type, memspace, dataspace );
      } // end: read
      
      

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
    void scale(GeoStar::Raster *rasNew, const std::string &name, const RasterType &type, const int &nx, const int &ny);

  }; // end class: Raster
  
}// end namespace GeoStar


#endif //RASTER_HPP_
