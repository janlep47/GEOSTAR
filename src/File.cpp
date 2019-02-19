// File.cpp
// 
// by Leland Pierce, Feb 18, 2017
//
//-------------------------------------

#include <string>
#include <iostream>

#include "H5Cpp.h"
//#include "H5File.h"
//#include "h5cpputil.h"

#include "File.hpp"
#include "Image.hpp"
#include "Vector.hpp"
#include "Shape.hpp"

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
    //FileAccessException FileAccessError;
    //FileExistsException FileExistsError;
    //FileDoesNotExistException FileDoesNotExistError;

    if(access=="new") {
      // existence is an error:
      boost::filesystem::path p(name);
      if(boost::filesystem::exists( p )){
        throw_FileExistsError(name);
      }//endif
        fileobj = new H5::H5File( name, H5F_ACC_EXCL );
        //fileobj = new H5::H5File( name, H5F_ACC_EXCL    , H5::FileCreatPropList::DEFAULT,  H5::FileAccPropList::DEFAULT);
        //H5::H5File fileobj( name, H5F_ACC_EXCL );

    } else if(access=="existing") {
      // non-existence is an error:
      boost::filesystem::path p(name);
      if(!boost::filesystem::exists( p )){
        throw_FileDoesNotExistError(name);
      }//endif
        fileobj = new H5::H5File( name, H5F_ACC_RDWR );
        //fileobj = new H5::H5File( name, H5F_ACC_RDWR    , H5::FileCreatPropList::DEFAULT,  H5::FileAccPropList::DEFAULT);
        //H5::H5File fileobj( name, H5F_ACC_RDWR );

    } else {
        throw_FileAccessError(name);
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

  bool File::datasetExists(const std::string &name) {
    try{
      H5::Exception::dontPrint();
      H5::DataSet obj = fileobj->openDataSet( name );
      return true;
    }
    catch (...) {
      return false;
    }

    // perhaps try: return fileobj->exists(name);
  }// end: datasetExists


}// end namespace GeoStar
