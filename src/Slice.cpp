// Slice.cpp
//
// by Janice Richards, Sept 6, 2017
//
//--------------------------------------------


#include <string>
#include <iostream>
#include <vector>
#include <array>

#include "Slice.hpp"

namespace GeoStar {
    
    Slice::Slice() {
        this->x0 = 0;
        this->y0 = 0;
        this->deltaX = 0;
        this->deltaY = 0;
        this->strideX = 0;
        this->strideY = 0;
        this->numberPixels = 0;
    }

  Slice::Slice(const long int &x0, const long int &y0, const long int &deltaX, const long int &deltaY, const long int &numberPixels, const long int &strideX, const long int &strideY){
      this->x0 = x0;
      this->y0 = y0;
      this->deltaX = deltaX;
      this->deltaY = deltaY;
      this->strideX = strideX;
      this->strideY = strideY;
      if (numberPixels <= 0) {
          int numberXPixels = deltaX;
          if (strideX != 1) numberXPixels /= strideX;
          int numberYPixels = deltaY;
          if (strideY != 1) numberYPixels /= strideY;
          this->numberPixels = numberXPixels * numberYPixels;
      } else this->numberPixels = numberPixels;
      /*
      if (numberPixels == 0) {
          int numberXPixels = x0 + deltaX - 1;
          if (strideX != 1) numberXPixels /= strideX;
          int numberYPixels = y0 + deltaY - 1;
          if (strideY != 1) numberYPixels /= strideY;
          this->numberPixels = numberXPixels * numberYPixels;
      } else this->numberPixels = numberPixels;
       */
  }// end-Slice-constructor
    /*
    Slice::Slice(const long int &x0, const long int &y0, const long int &deltaX, const long int &deltaY, const long int numberOfPixels, const long int &strideX, const long int &strideY) {
        this->x0 = x0;
        this->y0 = y0;
        this->deltaX = deltaX;
        this->deltaY = deltaY;
        this->strideX = strideX;
        this->strideY = strideY;
        this->numberPixels = numberPixels;
    }
*/
    
    
    // set delta-x
    void Slice::setDeltaX(const long int &deltaX) {
        this->deltaX = deltaX;
        this->numberPixels = deltaX*deltaY;
    }
    
    // set delta-y
    void Slice::setDeltaY(const long int &deltaY) {
        this->deltaY = deltaY;
        this->numberPixels = deltaX*deltaY;
    }
    
    // set stride-x
    void Slice::setStrideX(const long int &strideX) {
        this->strideX = strideX;
        this->numberPixels = deltaX*deltaY;
    }
    
    // set stride-y
    void Slice::setStrideY(const long int &strideY) {
        this->strideY = strideY;
        this->numberPixels = deltaX*deltaY;
    }

    
}// end namespace GeoStar
