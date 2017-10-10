// File.cpp
// 
// by Leland Pierce, Feb 18, 2017
//
//-------------------------------------

#include <string>
#include <iostream>

#include "H5Cpp.h"

#include "File.hpp"
#include "Image.hpp"
#include "Exceptions.hpp"
#include "attributes.hpp"

#include "boost/filesystem.hpp"

namespace GeoStar {

  // create a new hdf5 file, or open an exising one.
  // name is the pathname of the file.
  // access can be:
  //          "new", create a new file, existence is an error
  //      or "existing", opens an existing file, non-existence is an error.
  File::File(const std::string &name, const std::string &access) {
    FileAccessException FileAccessError;
    FileExistsException FileExistsError;
    FileDoesNotExistException FileDoesNotExistError;

    if(access=="new") {
      // existence is an error:
      boost::filesystem::path p(name);
      if(boost::filesystem::exists( p )){
        throw FileExistsError;
      }//endif
      fileobj = new H5::H5File( name, H5F_ACC_EXCL );

    } else if(access=="existing") {
      // non-existence is an error:
      boost::filesystem::path p(name);
      if(!boost::filesystem::exists( p )){
        throw FileDoesNotExistError;
      }//endif
      fileobj = new H5::H5File( name, H5F_ACC_RDWR );

    } else {
      throw FileAccessError;
    }// endif

    filename = name;
    filetype="geostar::hdf5";

    // set objtype attribute.
    write_object_type(filetype);

  }// end-File-constructor



  bool File::groupExists(const std::string &name) {
    try{
      H5::Exception::dontPrint();
      H5::Group obj = fileobj->openGroup( name );
      return true;
    }
    catch (...) {
      return false;
    }
  }// end: groupExists


}// end namespace GeoStar
