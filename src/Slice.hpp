// Slice.hpp
//
// by Janice Richards, Sept 6, 2017
//
//----------------------------------------
#ifndef SLICE_HPP_
#define SLICE_HPP_


#include <string>
#include <vector>


namespace GeoStar {
  class Slice {

  private:
      long int x0;
      long int y0;
      long int deltaX;
      long int deltaY;
      long int strideX;
      long int strideY;
      long int numberPixels;


  public:
      // create a slice with strideX and strideY (defaulted to 1), to be able to skip over pixels,
      //  i.e. skip every strideX pixels (in x-direction), etc.
      Slice(const long int &x0, const long int &y0, const long int &deltaX, const long int &deltaY, const long int &numberPixels = 0, const long int &strideX = 1, const long int &strideY = 1);
    
      //Slice(const long int &x0, const long int &y0, const long int &deltaX, const long int &deltaY, const long int numberOfPixels, const long int &strideX = 1, const long int &strideY = 1);

      Slice();
      
      // return x0
      inline long int getX0() const {
        return x0;
      }
      
      // return y0
      inline long int getY0()  const {
          return y0;
      }
      
      // return deltaX
      inline long int getDeltaX()  const {
          return deltaX;
      }
      
      // return deltaY
      inline long int getDeltaY()  const {
          return deltaY;
      }
      
      // return strideX
      inline long int getStrideX() {
          return strideX;
      }
      
      // return strideY
      inline long int getStrideY() {
          return strideY;
      }
      
      // return numberPixels
      inline long int getNumberPixels()  const {
          return numberPixels;
      }
      
      // set x0
      inline void setX0(const long int &x0) {
          this->x0 = x0;
      }
      
      // set y0
      inline void setY0(const long int &y0) {
          this->y0 = y0;
      }
      
      // set delta-x
      void setDeltaX(const long int &deltaX);
      
      // set delta-y
      void setDeltaY(const long int &deltaY);
      
      // set stride-x
      void setStrideX(const long int &strideX);
      
      // set stride-y
      void setStrideY(const long int &strideY);
      
      // set numberPixels
      //inline void setNumberPixels(const long int &numberPixels) {
      //    this->numberPixels = numberPixels;
      //}

  }; // end class: Slice
  
}// end namespace GeoStar


#endif //SLICE_HPP_
