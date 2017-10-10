// RasterType.hpp
//
// by Leland Pierce, Feb 18, 2017
//
//----------------------------------------
#ifndef RASTERTYPE_HPP_
#define RASTERTYPE_HPP_


#include <string>

#include "H5Cpp.h"


namespace GeoStar {

  enum RasterType {
    
    INT8U, INT8S, INT16U, INT16S, INT32U, INT32S, INT64U, INT64S,
    REAL32, REAL64,
    COMPLEX_INT16, COMPLEX_INT32, COMPLEX_INT64, COMPLEX_INT128,
    COMPLEX_REAL64, COMPLEX_REAL128
    
  }; // end: RasterType
    
}// end namespace GeoStar


#endif // RASTER_HPP_
