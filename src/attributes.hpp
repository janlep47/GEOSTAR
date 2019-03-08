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

  /** \brief write_object_type -- Write attribute specifying the object-type for this object.

      \see write_attribute(), read_object_type()

      \param[in] obj
                 The object that his attribute should get attached to.
                 Should be cast to an H5::H5Object pointer.

      \param[in] value
                 A string giving the value of the object-type.
                 For example "geostar::file" for files.
      \returns 
               Nothing
      
      \par Exceptions
              AttributeError if any error writing the value.

      \par Example
           Each GeoStar object class should implement a method, 
           as in the following, for the File class:
           \code
    inline void write_object_type(const std::string &value) {
        GeoStar::write_object_type((H5::H5Object *)fileobj,value);
    }
           \endcode
           
       \par Details
       This uses HDF5 string attributes. Without the object_type attribute, 
       GeoStar has no way of knowing what kind of object it is dealing with.
       In such cases it will refuse to do anything.
       So, its critical that this is set, usually by the class constructor.

  */
    void write_object_type(H5::H5Object *obj, const std::string &value);

  /** \brief write_attribute -- Write an attribute and its value for this object.

      \see write_object_type(), read_attribute()

      \param[in] obj
                 The object that his attribute should get attached to.
                 Should be cast to an H5::H5Object pointer.

      \param[in] name
                 A string giving the name of the atrtibute.
                 For example "object_type".

      \param[in] value
                 A string giving the value of the attribute.
                 For example "geostar::file" for the 
                 "object_type" attribute for files.
      \returns 
               Nothing
      
      \par Exceptions
              AttributeError if any error writing the value.

      \par Example
           To keep track of the read/write state of an object, one could use
           an attribute named "access", with values "read" or "write".
           Setting an object to be read-only could be done as in the following code:
           \code
           GeoStar::write_attribute((H5::H5Object *)obj,"access","read");
           \endcode
           
       \par Details
       This uses HDF5 string attributes. 

  */
    void write_attribute(H5::H5Object *obj, const std::string &name, const std::string &value);


  /** \brief read_object_type -- Read attribute specifying the object-type for this object.

      \see read_attribute(), write_object_type()

      \param[in] obj
                 The object that his attribute should get attached to.
                 Should be cast to an H5::H5Object pointer.

      \returns 
               The string value of the attribute
      
      \par Exceptions
              Attribute DoesNotExistError if the attribute cannot be found.
              AttributeError if any error reading the value.
              

      \par Example
           Each GeoStar object class should implement a method, 
           as in the following, for the File class:
           \code
    inline std::string read_object_type() const {
        return GeoStar::read_object_type((H5::H5Object *)fileobj);
    }
           \endcode
           
       \par Details
       This uses HDF5 string attributes. Without the object_type attribute, 
       GeoStar has no way of knowing what kind of object it is dealing with.
       In such cases it will refuse to do anything.
       So, its critical that this is set, usually by the class constructor.

  */
    std::string read_object_type(const H5::H5Object *obj);

  /** \brief read_attribute -- Read the value of an attribute for this object.

      \see read_object_type(), write_attribute()

      \param[in] obj
                 The object that his attribute should be attached to.
                 Should be cast to an H5::H5Object pointer.

      \param[in] name
                 A string giving the name of the atrtibute.
                 For example "object_type".

      \returns 
                 A string giving the value of the attribute.
                 For example "geostar::file" for the 
                 "object_type" attribute for files.
      
       \par Exceptions
              AttributeDoesNotExistError if the "name" cannot be found associated with this object.
              AttributeError if any error reading the value.

      \par Example
           To keep track of the read/write state of an object, one could use
           an attribute named "access", with values "read" or "write".
             Reading this attribute can be done with the following code:
           \code
           if( GeoStar::read_attribute((H5::H5Object *)obj,"access") == "write"){
              .. do something to change the object here....
           }
           \endcode
           
       \par Details
       This uses HDF5 string attributes. 

  */
     std::string read_attribute(const H5::H5Object *obj, const std::string &name);

}// end namespace GeoStar

#endif // ATTRIBUTES_HPP_
