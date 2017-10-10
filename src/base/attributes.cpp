// attributes.cpp
// 
// by Leland Pierce, Feb 19, 2017
//
//-------------------------------------

#include <string>
#include <iostream>

#include "H5Cpp.h"

#include "File.hpp"
#include "Image.hpp"
#include "Exceptions.hpp"


namespace GeoStar {


  void write_object_type(H5::H5Location *obj, const std::string &value){
    AttributeErrorException AttributeError;

    std::string object_type="object_type";

    // 1. delete the attribute, if it's there:
    if(obj->attrExists(object_type)){
      obj->removeAttr(object_type);
    }// endif

    // 2. create the attribute:
    H5::StrType strtype(H5::PredType::C_S1, value.length());
    H5::DataSpace att_space(H5S_SCALAR);
    H5::Attribute att = obj->createAttribute( object_type, strtype, att_space );

    // 3. write to the attribute:
    try {
      att.write( strtype, value );
    }
    catch (...) {
      throw AttributeError;
    }

  }// end write_object_type




  std::string read_object_type(const H5::H5Location *obj) {
    AttributeErrorException AttributeError;

    std::string object_type = "object_type";

    if(obj->attrExists(object_type)){
      try {
        H5::Attribute att = obj->openAttribute(object_type);
        H5::StrType stype = att.getStrType();
        std::string str;
        att.read(stype, str);
        return str;
      }
      catch (...) {
         throw AttributeError;
      }
    } else {
      return std::string("");
    }
  }// end read_object_type


}// end namespace GeoStar
