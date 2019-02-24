// WarpParameters.hpp
//
// by Janice Richards, Sept 23, 2017
//
//----------------------------------------
#ifndef WARP_PARAMETERS_HPP_
#define WARP_PARAMETERS_HPP_


#include <Eigen/Dense>

#include <string>
#include <vector>

#include "H5Cpp.h"

#include "Slice.hpp"
#include "RasterType.hpp"
#include "attributes.hpp"
#include "Exceptions.hpp"
#include "Point.hpp"


namespace GeoStar {
    
    
    
    /** \brief WarpParameters -- Implementation of warping operations for GeoStar Raster objects.
     
     
     This class is used to create a linear least squares solution in reponse to a set of ground control
     points.  The solution can then be used to map coordinates of an input image, to new coordinates of
     an output image (the "warped" image).  This class is used in two steps: first, creating the object
     data, with a set of ground control points.  These ground control points are a list of random coordinates
     from an input image, with the corresponding coordinates that map to the output image.  Once this
     WarpParameters object data has been created, the user can call a function to get the corresponding output
     coordinates from any input coordinates of the input image.
     
     Currently, this class is used by Raster:warp() function.
     
     \see Raster
     
     \par Usage Overview
     The WarpParameters class is meant to be used when dealing with GeoStar Rasters.
     
     \par Details
     This class also has two main functions.  One is to create the WarpParameters object/data.  The other
     is to return an output coordinate point, based on a given input coordinate point.
     
     */

    
    
    class Point;
  
    class WarpParameters {

    private:
        int pbx;   // # of elements computed for BETA b(X)
        int pby;     // # of elements computed for BETA b(Y)
        Eigen::VectorXd rbx;   // Actual current BETAS b(X) -coefficients for f(X) = x'
        Eigen::VectorXd rby;   // Actual current BETAS b(Y) -coefficients for f(Y) = y'
        Eigen::VectorXd rbxPrime; // coefficients for f(X') = x (going backwards)
        Eigen::VectorXd rbyPrime; // coefficients for f(Y') = y      "
        
        Eigen::VectorXd x;   // NEW ... for testing;   ... MADE NO DIFF!
        GeoStar::Point point; // NEW ... for testing;
        // NOT USED YET: (if ever?)
        //double rabx[3]; // Actual current affine BETAS b(X)
        //double raby[3]; // Actual current affine BETAS b(Y)
        //double covx[21][21]; // covariance matrix for b(X)
        //double covy[21][21]; // covariance matrix for b(Y)
        //int ngcp;
        //int order;
        
        
        // determine the coefficients for a bilinear polynomial (of a specified order), for
        //  modelling 2D GCPs by regression analysis - called by GCPRegression2D
        //void regression2D(const double *rix, const double *riy,
        //                  const double *rox, const double *roy,
        //                  const int ngcp, int *order);
        void regression2D(const std::vector<double> rix, const std::vector<double> riy,
                          const std::vector<double> rox, const std::vector<double> roy,
                          int *order);

    public:

        // create a WarpParameters
        WarpParameters();
        
        //WarpParameters(const std::vector<double> rix, const std::vector<double> riy,
        //               const std::vector<double> rox, const std::vector<double> roy,
        //               int *order);


  /** \brief WarpParameters::GCPTransform returns a pair of output raster coordinates, based on a given
   pair of input raster coordinates.  The output coordinates are calculated using previously generated
   WarpParameters linear least squares solution coefficients.

   WarpParameters::GCPTransform returns a single pair of coordinates of the "warped" output raster, from
   a single pair of coordinates of the input raster.

   \see  Raster, Point

   \param[in] rix
       This is a double, set by the user, that holds the desired value for the unwarped raster x-coordinate
        pixel point.
   \param[in] riy
       This is a double, set by the user, that holds the desired value of the unwarped raster y-coordinate
        pixel point.


   \returns
       the warped coordinates as a point.

   \par Exceptions
       none

   \par Example
       To return the new coordinates of a warped raster, from a given pixel coordinates of the input raster:
       \code
       #include "geostar.hpp"
       #include <string>
       int main()
       {
           GeoStar::File *file;
       try {
               file = new GeoStar::File("sirc_raco","existing");
       }//end-try
       catch (...) {
           cerr << "GeoStar::File open failure"<<endl;
       }//end-catch

           GeoStar::Image *img;
       try {
               img = file->open_image("image1");
       }//end-try
       catch (...) {
           cerr << "GeoStar::open_image failure"<<endl;
       }//end-catch

           GeoStar:: Raster *ras;
       try {
               ras = img->open_raster("ex1");
       }//end-try
       catch (...) {
           cerr << "GeoStar::open_raster failure"<<endl;
       }//end-catch
   
   
           Slice inSlice(0,0,40,50);
           Slice outSlice(0,0,100,200);
           GeoStar::WarpParameters warpData;
           std::vector<double> rix, riy, rox, roy;
           rix[0] = 5; rix[1] = 30; rix[2] = 16;
           riy[0] = 7; riy[1] = 22; riy[2] = 9;
           rox[0] = 12; rox[1] = 51; rox[2] = 68;
           roy[0] = 22; roy[1] = 35; roy[2] = 53;
           int order = 1;
           double rmsx = 0.0, rmsy = 0.0;
           warpData.GCPregression(rix, riy, rox, roy, &order, &rmsx, &rmsy);
   
           double xin, yin;
           double xout, yout;
           xin = 4; yin = 35;
           GeoStar::Point point = warpData.GCPTransform(xin, yin);
           xout = point.getX(); yout = point.getY();
   
       }//end-main
       \endcode

    \par Details
       Implementation in attributes.cpp for any object type.
       The implementation in this class makes it easier to use in the File context.
  
  */
        // compute (X',Y') = (FX(X,Y),FY(X,Y))
        GeoStar::Point GCPTransform(const double &rix, const double &riy);


        /** \brief WarpParameters::GCPTransform returns a pair of output raster coordinates, based on a given
         pair of input raster coordinates.  The output coordinates are calculated using previously generated
         WarpParameters linear least squares solution coefficients.
         
         WarpParameters::GCPTransform returns a single pair of coordinates of the "warped" output raster, from
         a single pair of coordinates of the input raster.
         
         \see  Raster, Point
         
         \param[in] rix
         This is a double, set by the user, that holds the desired value for the unwarped raster x-coordinate
         pixel point.
         \param[in] riy
         This is a double, set by the user, that holds the desired value of the unwarped raster y-coordinate
         pixel point.
         \param[in] &rixerror
         This is the address to a double, which will hold the root mean square error of the x-coordinate, based
         on the least squares solution, compared with the input ground control points used to generate this
         least squares solution.
         \param[in] &roxerror
         This is the address to a double, which will hold the root mean square error of the y-coordinate, based
         on the least squares solution, compared with the input ground control points used to generate this
         least squares solution.
         
         
         \returns
         the warped coordinates as a point.
         
         \par Exceptions
         none
         
         \par Example
         To return the new coordinates of a warped raster, from a given pixel coordinates of the input raster:
         \code
         #include "geostar.hpp"
         #include <string>
         int main()
         {
         GeoStar::File *file;
         try {
         file = new GeoStar::File("sirc_raco","existing");
         }//end-try
         catch (...) {
         cerr << "GeoStar::File open failure"<<endl;
         }//end-catch
         
         GeoStar::Image *img;
         try {
         img = file->open_image("image1");
         }//end-try
         catch (...) {
         cerr << "GeoStar::open_image failure"<<endl;
         }//end-catch
         
         GeoStar:: Raster *ras;
         try {
         ras = img->open_raster("ex1");
         }//end-try
         catch (...) {
         cerr << "GeoStar::open_raster failure"<<endl;
         }//end-catch
         
         
         Slice inSlice(0,0,40,50);
         Slice outSlice(0,0,100,200);
         GeoStar::WarpParameters warpData;
         std::vector<double> rix, riy, rox, roy;
         rix[0] = 5; rix[1] = 30; rix[2] = 16;
         riy[0] = 7; riy[1] = 22; riy[2] = 9;
         rox[0] = 12; rox[1] = 51; rox[2] = 68;
         roy[0] = 22; roy[1] = 35; roy[2] = 53;
         int order = 1;
         double rmsx = 0.0, rmsy = 0.0;
         warpData.GCPregression(rix, riy, rox, roy, &order, &rmsx, &rmsy);
         
         double xin, yin;
         double xout, yout;
         double xerror, yerror;
         xin = 4; yin = 35;
         GeoStar::Point point = warpData.GCPTransform(xin, yin, &xerror, &yerror);
         xout = point.getX(); yout = point.getY();
         
         }//end-main
         \endcode
         
         \par Details
         Implementation in attributes.cpp for any object type.
         The implementation in this class makes it easier to use in the File context.
         
         */
        // compute (X',Y') = (FX(X,Y),FY(X,Y))
        GeoStar::Point GCPTransform(const double &rix, const double &riy,
                           const double *roxerr, const double *royerr);


        /** \brief WarpParameters::GCPRootMeanSq calculates the error associated with the regression model
           of the set of 2D GCP (Ground Control Point) pairs.
         
         WarpParameters::GCPRootMeanSq returns an overall error of a single pair of input and output
         ground control coordinates, based on the current least squares solution previously calculated.
         The root mean square error is also calculated in each of the x- and y- coordinates, for the
         given input and cooresponding (warped) output coordinates.
         
         \see  Raster, Point
         
         \param[in] rix
         This is a double, set by the user, that holds the desired value for the unwarped raster x-coordinate
         pixel point.
         \param[in] riy
         This is a double, set by the user, that holds the desired value of the unwarped raster y-coordinate
         pixel point.
         \param[in] rox
         This is a double, set by the user, that holds the desired value of the cooresponding warped raster
         x-coordinate pixel point.
         \param[in] roy
         This is a double, set by the user, that holds the desired value of the cooresponding warped raster
         y-coordinate pixel point.
         \param[in] &rixerror
         This is the address to a double, which will hold the root mean square error of the x-coordinate, based
         on the least squares solution, compared with the input ground control points used to generate this
         least squares solution.
         \param[in] &roxerror
         This is the address to a double, which will hold the root mean square error of the y-coordinate, based
         on the least squares solution, compared with the input ground control points used to generate this
         least squares solution.
         
         
         \returns
         the total root mean square error.
         
         \par Exceptions
         none
         
         \par Example
         To return the new coordinates of a warped raster, from a given pixel coordinates of the input raster:
         \code
         #include "geostar.hpp"
         #include <string>
         int main()
         {
         GeoStar::File *file;
         try {
         file = new GeoStar::File("sirc_raco","existing");
         }//end-try
         catch (...) {
         cerr << "GeoStar::File open failure"<<endl;
         }//end-catch
         
         GeoStar::Image *img;
         try {
         img = file->open_image("image1");
         }//end-try
         catch (...) {
         cerr << "GeoStar::open_image failure"<<endl;
         }//end-catch
         
         GeoStar:: Raster *ras;
         try {
         ras = img->open_raster("ex1");
         }//end-try
         catch (...) {
         cerr << "GeoStar::open_raster failure"<<endl;
         }//end-catch
         
         
         Slice inSlice(0,0,40,50);
         Slice outSlice(0,0,100,200);
         GeoStar::WarpParameters warpData;
         std::vector<double> rix, riy, rox, roy;
         rix[0] = 5; rix[1] = 30; rix[2] = 16;
         riy[0] = 7; riy[1] = 22; riy[2] = 9;
         rox[0] = 12; rox[1] = 51; rox[2] = 68;
         roy[0] = 22; roy[1] = 35; roy[2] = 53;
         int order = 1;
         double rmsx = 0.0, rmsy = 0.0;
         warpData.GCPregression(rix, riy, rox, roy, &order, &rmsx, &rmsy);
         
         double xin, yin;
         double xout, yout;
         xin = 4; yin = 35;
         GeoStar::Point point = warpData.GCPTransform(xin, yin);
         xout = point.getX(); yout = point.getY();
         
         }//end-main
         \endcode
         
         \par Details
         Implementation in attributes.cpp for any object type.
         The implementation in this class makes it easier to use in the File context.
         
         */
        // calculate the error associated with the regression model of the set of 2D GCP
        //  (Ground Control Point) pairs.
        //double GCPRootMeanSq(const double *rix, const double *riy,
        //                   const double *rox, const double *roy, const int ngcps,
        //                   double *rmsx, double *rmsy);
        double GCPRootMeanSq(const std::vector<double> rix, const std::vector<double> riy,
                             const std::vector<double> rox, const std::vector<double> roy,
                             double *rmsx, double *rmsy);

        /** \brief WarpParameters::GCPRegression2D generates the WarpParameters linear least squares solution
         coefficients.
         
         WarpParameters::GCPRegression2D creates the WarpParameters object data, which is the linear least
         squares solution to a set of input and output ground control points (coordinates).
         
         \see  Raster, Point
         
         \param[in] rix
         This is a vector of doubles, set by the user, that holds the desired values of the x-coordinates
         of the unwarped ground control points.
         pixel point.
         \param[in] riy
         This is a vector of doubles, set by the user, that holds the desired values of the y-coordinates
         of the unwarped ground control points.
         \param[in] rox
         This is a vector of doubles, set by the user, that holds the desired values of the x-coordinates
         of the corresponding warped ground control points.
         \param[in] roy
         This is a  vector of doubles, set by the user, that holds the desired values of the y-coordinates
         of the corresponding warped ground control points.
         \param[in] order
         This is the address to an int, set by the user, that hold the desired order of the linear least
         squares solution to be generated.  This can be set by this function, to more accurately reflect
         the number of ground control points given, if the order is set too high for the number of points
         given.
         \param[in] rmsx
         I DON'T KNOW WHAT THIS IS SUPPOSED TO BE ......
         \param[in] rmsy
         I DON'T KNOW WHAT THIS IS SUPPOSED TO BE ......

         
         \returns
         nothing
         
         \par Exceptions
         none
         
         \par Example
         To return the new coordinates of a warped raster, from a given pixel coordinates of the input raster:
         \code
         #include "geostar.hpp"
         #include <string>
         int main()
         {
         GeoStar::File *file;
         try {
         file = new GeoStar::File("sirc_raco","existing");
         }//end-try
         catch (...) {
         cerr << "GeoStar::File open failure"<<endl;
         }//end-catch
         
         GeoStar::Image *img;
         try {
         img = file->open_image("image1");
         }//end-try
         catch (...) {
         cerr << "GeoStar::open_image failure"<<endl;
         }//end-catch
         
         GeoStar:: Raster *ras;
         try {
         ras = img->open_raster("ex1");
         }//end-try
         catch (...) {
         cerr << "GeoStar::open_raster failure"<<endl;
         }//end-catch
         
         
         Slice inSlice(0,0,40,50);
         Slice outSlice(0,0,100,200);
         GeoStar::WarpParameters warpData;
         std::vector<double> rix, riy, rox, roy;
         rix[0] = 5; rix[1] = 30; rix[2] = 16;
         riy[0] = 7; riy[1] = 22; riy[2] = 9;
         rox[0] = 12; rox[1] = 51; rox[2] = 68;
         roy[0] = 22; roy[1] = 35; roy[2] = 53;
         int order = 1;
         double rmsx = 0.0, rmsy = 0.0;
         warpData.GCPregression(rix, riy, rox, roy, &order, &rmsx, &rmsy);
         
         // At this point, warping can now take place, since the linear least squares solution
         //  has been generated
         ...
         }//end-main
         \endcode
         
         \par Details
         Implementation in attributes.cpp for any object type.
         The implementation in this class makes it easier to use in the File context.
         
         */
        // determine the coefficients for a bilinear polynomial (of a specified order), for
        //  modelling 2D GCPs by regression analysis
        //void GCPRegression2D(const double *rix, const double *riy,
        //                     const double *rox, const double *roy,
        //                     const int ngcp, int *order,
        //                     double *rmsx, double *rmsy);
        void GCPRegression2D(const std::vector<double> rix, const std::vector<double> riy,
                             const std::vector<double> rox, const std::vector<double> roy,
                             int *order, double *rmsx, double *rmsy);

  }; // end class: WarpParameters
  
}// end namespace GeoStar


#endif //WARP_PARAMETERS_HPP_
