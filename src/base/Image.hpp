// Image.hpp
//
// by Leland Pierce, Feb 18, 2017
// 
// revised March 11, 2017: added: read_tiff_1_8U()
//
//-------------------------------------
#ifndef IMAGE_HPP_
#define IMAGE_HPP_

#include <string>

#include "H5Cpp.h"
#include "Raster.hpp"
#include "attributes.hpp"

namespace GeoStar {
  class File;
  
  class Image {
    
  private:
    std::string imagename;
    std::string imagetype;

  public:
    H5::Group *imageobj;

    // opening an Image in this file.
    // if exists: it open it
    // if does not exist: it creates it.
    Image(File *file, const std::string &name);

    // write the "object_type" attribute
    inline void write_object_type(const std::string &value) {
      GeoStar::write_object_type((H5::H5Location *)imageobj,value);
    }

    // read the "object_type" attribute
    inline std::string read_object_type() const {
      return GeoStar::read_object_type((H5::H5Location *)imageobj);
    }

    // cleans up the H5::Group object 
    inline ~Image() {
      delete imageobj;
    }

    // creates a new Raster (dataset) within this Image (group)
    // existence is an error
    inline Raster *create_raster(const std::string &name, 
                                 const RasterType &type,
                                 const int &nx, const int &ny) {
      return new Raster(this,name,type,nx,ny);
    }


    // opens an existing Raster (dataset) within this Image (group)
    // non-existence is an error
    Raster *open_raster(const std::string &name) {
      return new Raster(this,name);
    }


    // does the named DataSet exist in this Image (group)?
    bool datasetExists(const std::string &name);

    // create a new dataset in this Image (group)
    // existence is an error
    inline H5::DataSet createDataset(const std::string &name, 
                                     const H5::DataType &data_type,
                                     const H5::DataSpace &data_space) {
      return imageobj->createDataSet(name,data_type,data_space);
    }

    // open an existing dataset in this Image (group)
    // non-existence is an error
    inline H5::DataSet openDataset(const std::string &name) {
      return imageobj->openDataSet(name);
    }


    // read a single-channel image file
    // infile is the name of the intput image file
    // name is the string to identify the new raster that is created
    // if the image is more than 1 channel, it just uses the first.
    // for now, it creates a float channel.
    // returns the newly-created Raster.
    Raster *read_file(const std::string &infile, const std::string &name);


  }; // end class: Image
  
}// end namespace GeoStar


#endif
