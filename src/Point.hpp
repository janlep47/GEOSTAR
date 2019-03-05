// Point.hpp
//
// by Janice Richards, Oct 2, 2017
//
//----------------------------------------
#ifndef POINT_HPP_
#define POINT_HPP_


#include <string>
#include <vector>


namespace GeoStar {
  class Point {

  private:
      //double x;
      //double y;

  public:
      
      double x;   // new ...
      double y;   // new ...

      
      Point() {
          this->x = 0.;
          this->y = 0.;
      }
      
      // create a new GCPdata object
      Point(double x, double y) {
          this->x = x;
          this->y = y;
      }
      

      inline double getX() {
          return x;
      }

      inline double getY() {
          return y;
      }
      
      inline void setX(double val) {
          x = val;
      }
      
      inline void setY(double val) {
          y = val;
      }
      
  }; // end class: Point
  
}// end namespace GeoStar


#endif //POINT_HPP_
