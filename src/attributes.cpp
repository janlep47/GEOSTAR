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


    void write_object_type(H5::H5Object *obj, const std::string &value){
        std::string object_type="object_type";

        // 1. delete the attribute, if it's there:
        if (obj->attrExists(object_type)) {
            obj->removeAttr(object_type);
        }// endif

        // 2. create the attribute:
        H5::StrType strtype(H5::PredType::C_S1, value.length());
        H5::DataSpace att_space(H5S_SCALAR);
        H5::Attribute att = obj->createAttribute( object_type, strtype, att_space );

        // 3. write to the attribute:
        try {
            att.write( strtype, value );
        } catch (...) {
            throw_AttributeError("writing "+object_type+" = "+value);
        }

    }// end write_object_type

    
    
    void write_attribute(H5::H5Object *obj, const std::string &name, const std::string &value){
        // 1. delete the attribute, if it's there:
        if (obj->attrExists(name)) {
            obj->removeAttr(name);
        }// endif
        
        // 2. create the attribute:
        H5::StrType strtype(H5::PredType::C_S1, value.length());
        H5::DataSpace att_space(H5S_SCALAR); // or H5S_SIMPLE, default is H5S_SCALAR
        H5::Attribute att = obj->createAttribute( name, strtype, att_space );
        
        // 3. write to the attribute:
        try {
            att.write( strtype, value );
        }
        catch (...) {
            throw_AttributeError("writing "+name+" = "+value);
        }
        
    }// end write_object_type




    std::string read_object_type(const H5::H5Object *obj) {
        std::string object_type = "object_type";

        if (obj->attrExists(object_type)) {
            try {
                H5::Attribute att = obj->openAttribute(object_type);
                H5::StrType stype = att.getStrType();
                std::string str;
                att.read(stype, str);
                return str;
            } catch (...) {
                throw_AttributeError("reading '"+object_type+"'");
            }
        } else {
            //return std::string("");
            throw_AttributeDoesNotExistError(obj->getFileName(), "object_type");
        }
    }// end read_object_type

    
    
    
    std::string read_attribute(const H5::H5Object *obj, const std::string &name) {
        if (obj->attrExists(name)) {
            try {
                H5::Attribute att = obj->openAttribute(name);
                H5::StrType stype = att.getStrType();
                std::string str;
                att.read(stype, str);
                return str;
            } catch (...) {
                throw_AttributeError("reading '"+name+"'");
            }
        } else {
            std::string src = obj->getFileName()+obj->getObjName();
            throw_AttributeDoesNotExistError(src, name);
        }
    }// end read_object_type


}// end namespace GeoStar
