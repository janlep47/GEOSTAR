// RasterFunction.hpp
//
// by Janice Richards, Feb 25, 2018
//
//----------------------------------------
#ifndef RASTER_FN_HPP_
#define RASTER_FN_HPP_


#include <string>
#include <vector>


namespace GeoStar {
    class RasterFunction {
        
    public:
        RasterFunction() {};
        virtual bool operator() (double pixelVal) = 0;
        virtual bool operator() (std::vector<double> pixelValues) = 0;
    }; // end class: RasterFunction
    
    //namespace GeoStar {
    class MyRasterFunction : public RasterFunction {
    public:
        MyRasterFunction() {};
        virtual bool operator() (double pixelVal) {
            if (pixelVal < 150) return false;
            else return true;
        }
        virtual bool operator() (std::vector<double> pixelValues) {
            double pixelVal;
            for (int i = 0; i < pixelValues.size(); i++) {
                pixelVal = pixelValues[i];
                if (pixelVal < 150) return false;
            }
            return true;
        }
    };
    
    
    /*
  class RasterFunction {

  //private:


  public:
      RasterFunction() {};
      
      virtual bool operator() (std::vector<double> pixelValues) = false;
      
  }; // end class: RasterFunction
     */
  
}// end namespace GeoStar


#endif //RASTER_FN_HPP_
