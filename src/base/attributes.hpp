// attributes.cpp
// 
// by Leland Pierce, Feb 19, 2017
//
//-------------------------------------
#ifndef ATTRIBUTES_HPP_
#define ATTRIBUTES_HPP_

#include <string>

#include "H5Cpp.h"

namespace GeoStar {

  // write_object_type: writes the "object_type" attribute to the
  //                    object with the given value.
  // inputs: obj: an HDF5 object (file, group, or dataset)
  //         value: the desired value of the "object_type" attribute
  // effects: the object in the file now has an attribute named
  //          "object_type" with the passed-in value (a string).
  void write_object_type(H5::H5Location *obj, const std::string &value);


  // read_object_type: reads the value of the "object_type" attribute
  //                   from the given object. If none exists, a blank
  //                   string is returned.
  // inputs: obj: an HDF5 object (file, group, or dataset)
  // effects: returns the value for the "object_type" attribute
  //          that is attached to the given object.
  //          returns a blank string if no "object_type" attribute
  //          is attached to this object.
  std::string read_object_type(const H5::H5Location *obj);

}// end namespace GeoStar

#endif // ATTRIBUTES_HPP_
