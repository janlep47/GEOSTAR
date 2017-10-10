// File.hpp
// 
// by Leland Pierce, Feb 18, 2017
//
//-------------------------------------
#ifndef FILE_HPP_
#define FILE_HPP_


#include <string>

#include "H5Cpp.h"

#include "Image.hpp"
#include "attributes.hpp"

namespace GeoStar {

  class File {
    
  private:
    std::string filename;
    std::string filetype;

  public:
    H5::H5File *fileobj;

    // Create or open an HDF5 file with the given name.
    // access can be either: 
    //        "new": create a new file, existence is an error
    //        "existing": open an existing file, non-existence is an error
    File(const std::string &name, const std::string &access);


    // write the "object_type" attribute
    inline void write_object_type(const std::string &value) {
      GeoStar::write_object_type((H5::H5Location *)fileobj,value);
    }

    // read the "object_type" attribute
    inline std::string read_object_type() const {
      return GeoStar::read_object_type((H5::H5Location *)fileobj);
    }

    // cleans up the H5::H5File object 
    inline ~File() {
      delete fileobj;
    }

    // create a new image in this File.
    // existence is an error
    inline Image *create_image(const std::string &name) {
      return new Image(this,name);
    }

    // open an existing image in this file
    // non-existence is an error
    inline Image *open_image(const std::string &name) {
      return new Image(this,name);
    }

    // does the named group exist in this file?
    bool groupExists(const std::string &name);

    // create a new group in this file
    // existence is an error
    inline H5::Group createGroup(const std::string &name) {
      return fileobj->createGroup(name);
    }

    // open an existing group in this file
    // non-existence is an error
    inline H5::Group openGroup(const std::string &name) {
      return fileobj->openGroup(name);
    }

  }; // end class: File
  
}// end namespace GeoStar

#endif // FILE_HPP_

