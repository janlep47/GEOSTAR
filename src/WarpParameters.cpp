// WarpParameters.cpp
//
// by Janice Richards, Sept 23, 2017
//
//--------------------------------------------
#include <Eigen/Dense>

#include <string>
#include <iostream>
#include <vector>
#include <array>

#include "H5Cpp.h"

#include "WarpParameters.hpp"
#include "Point.hpp"
#include "Image.hpp"
#include "Raster.hpp"
#include "Slice.hpp"
#include "Exceptions.hpp"
#include "attributes.hpp"


namespace GeoStar {

    WarpParameters::WarpParameters() {
        //rbx.resize(21);
        //rby.resize(21);
        //rbxPrime.resize(21);
        //rbyPrime.resize(21);
    }// end-WarpParameters-constructor

    
    //WarpParameters(const double *rix, const double *riy,
    //     const double *rox, const double *roy,
    //     const int ngcp, int *order) {
    //    rbx.resize();
    //}



    // compute (X',Y') = (FX(X,Y),FY(X,Y)
    GeoStar::Point WarpParameters::GCPTransform(const double &rix, const double &riy) {
        double xerr, yerr;
        xerr = 0;
        yerr = 0;
        return GCPTransform(rix, riy, &xerr, &yerr);
    }// end: GCPTransform

    // compute (X',Y') = (FX(X,Y),FY(X,Y)
    GeoStar::Point WarpParameters::GCPTransform(const double &rix, const double &riy,
                             const double *roxerr, const double *royerr) {
        int i, j;
        j = pbx;
        
        //Eigen::VectorXd x(j);
        double y, yerr;
        
        switch(j) {
            case 3:
                x(0) = 1.0;
                x(1) = rix;
                x(2) = riy;
                break;
            case 6:
                x(0) = 1.0;
                x(1) = rix;
                x(2) = riy;
                x(3) = rix*riy;
                x(4) = rix*rix;
                x(5) = riy*riy;
                break;
            case 10:
                x(0) = 1.0;
                x(1) = rix;
                x(2) = riy;
                x(3) = rix*riy;
                x(4) = rix*rix;
                x(5) = riy*riy;
                x(6) = rix*rix*riy;
                x(7) = rix*riy*riy;
                x(8) = rix*rix*rix;
                x(9) = riy*riy*riy;
                break;
            case 15:
                x(0) = 1.0;
                x(1) = rix;
                x(2) = riy;
                x(3) = rix*riy;
                x(4) = rix*rix;
                x(5) = riy*riy;
                x(6) = rix*rix*riy;
                x(7) = rix*riy*riy;
                x(8) = rix*rix*rix;
                x(9) = riy*riy*riy;
                x(10) = rix*rix*riy*riy;
                x(11) = rix*rix*rix*riy;
                x(12) = rix*riy*riy*riy;
                x(13) = rix*rix*rix*rix;
                x(14) = riy*riy*riy*riy;
                break;
            case 21:
                x(0) = 1.0;
                x(1) = rix;
                x(2) = riy;
                x(3) = rix*riy;
                x(4) = rix*rix;
                x(5) = riy*riy;
                x(6) = rix*rix*riy;
                x(7) = rix*riy*riy;
                x(8) = rix*rix*rix;
                x(9) = riy*riy*riy;
                x(10) = rix*riy*riy;
                x(10) = rix*rix*riy*riy;
                x(11) = rix*rix*rix*riy;
                x(12) = rix*riy*riy*riy;
                x(13) = rix*rix*rix*rix;
                x(14) = riy*riy*riy*riy;
                x(15) = rix*rix*rix*riy*riy;
                x(16) = rix*rix*riy*riy*riy;
                x(17) = rix*rix*rix*rix*riy;
                x(18) = rix*riy*riy*riy*riy;
                x(19) = rix*rix*rix*rix*rix;
                x(20) = riy*riy*riy*riy*riy;
                break;
            default:
                // screwed up ... die
                return GeoStar::Point((double)-999.0, (double)-999.0);
        } // end switch
        /*
        if (rbx.size() == 3) {
            //std::cerr << " size of rbx: " << rbx.size() << " ";
            return GeoStar::Point(rbx(0)+ rbx(1)*x(1) + rbx(2)*x(2), rby(0)+ rby(1)*x(1) + rby(2)*x(2));
        } else {
            return GeoStar::Point(rbx.dot(x), rby.dot(x));
        }
         */
        
        point.setX(rbx.dot(x));
        point.setY(rby.dot(x));
        return point;
        
        
        //return GeoStar::Point(rbx.dot(x), rby.dot(x));
    }// end: GCPTransform

    
    // calculate the error associated with the regression model of the set of 2D GCP
    //  (Ground Control Point) pairs.
    double WarpParameters::GCPRootMeanSq(const std::vector<double> rix, const std::vector<double> riy,
                             const std::vector<double> rox, const std::vector<double> roy,
                             double *rmsx, double *rmsy) {
        double totalError = 0;
        *rmsx = 0.0;
        *rmsy = 0.0;
        
        int ngcps = std::min(rix.size(),riy.size());
        int ngcps2 = std::min(rox.size(),roy.size());
        ngcps = std::min (ngcps,ngcps2);
        
        int i;
        GeoStar::Point pt;
        for (i = 0; i < ngcps; i++) {
            pt = GCPTransform(rix[i],riy[i]);
            totalError += pow((pt.getX() - rox[i]), 2.0) + pow((pt.getY() - roy[i]),2.0);
            *rmsx += pow(pt.getX() - rox[i], 2.0);
            *rmsy += pow(pt.getY() - roy[i], 2.0);
        }
        totalError /= ngcps;
        totalError = sqrt(totalError);
        *rmsx /= ngcps;
        *rmsy /= ngcps;
        *rmsx = sqrt(*rmsx);
        *rmsy = sqrt(*rmsy);
        return totalError;
    }// end: GCPRootMeanSq
    

    // determine the coefficients for a bilinear polynomial (of a specified order), for
    //  modelling 2D GCPs by regression analysis
    void WarpParameters::regression2D(const std::vector<double> rix, const std::vector<double> riy,
                            const std::vector<double> rox, const std::vector<double> roy,
                            int *order) {
        
        int ngcp = std::min(rix.size(),riy.size());
        int ngcp2 = std::min(rox.size(),roy.size());
        ngcp = std::min (ngcp,ngcp2);

        double chisq;
        
        int i1, i2;
        int i, j;
        int nunknowns[]={0,3,6,10,15,21};
        
        // this routine does a least-squares fit, with ngcp>= 3
        // error-return if not the case:
        if (ngcp < 3) {
            *order = 0;
            pbx = 0;
            pby = 0;
            return;
        }
        
        // set order to that appropriate for the given # of GCPs
        switch (*order) {
            case 1:
                // already dealt with
                break;
            case 2:
                if (ngcp < nunknowns[2]) *order=1;
                break;
            case 3:
                if (ngcp < nunknowns[3]) *order=2;
                break;
            case 4:
                if (ngcp < nunknowns[4]) *order=3;
                break;
            case 5:
                if (ngcp < nunknowns[5]) *order=4;
                break;
            default:
                // order is garbage, set it ourselves:
                if (ngcp < 3) {
                    *order=0;
                    return;
                } else if (ngcp < 6) *order=1;
                else if (ngcp < 10) *order=2;
                else if (ngcp < 15) *order=3;
                else if (ngcp < 21) *order=4;
                else   *order=5;
                break;
        }
        
        switch (*order) {
            case 1:
                x.resize(3);
                rbx.resize(3);
                rby.resize(3);
                rbxPrime.resize(3);
                rbyPrime.resize(3);
                break;
            case 2:
                x.resize(6);
                rbx.resize(6);
                rby.resize(6);
                rbxPrime.resize(6);
                rbyPrime.resize(6);
                break;
            case 3:
                x.resize(10);
                rbx.resize(10);
                rby.resize(10);
                rbxPrime.resize(10);
                rbyPrime.resize(10);
                break;
            case 4:
                x.resize(15);
                rbx.resize(15);
                rby.resize(15);
                rbxPrime.resize(15);
                rbyPrime.resize(15);
                break;
            case 5:
                x.resize(21);
                rbx.resize(21);
                rby.resize(21);
                rbxPrime.resize(21);
                rbyPrime.resize(21);
                break;
        }
       
        // for the problem y = Xc, where c are the unknown coeffs,
        // form X and y for fitting the x-terms in the WarpParameters:
        
        j=nunknowns[*order];
        Eigen::MatrixXd X(ngcp, j), cov(j,j);
        Eigen::VectorXd y(ngcp), c(j);
        
        
        switch(*order){
            case 1:
                for (i = 0; i < ngcp; i++){
                    X(i,0) = 1.0;
                    X(i,1) = rix[i];
                    X(i,2) = riy[i];
                    y(i) = rox[i];
                }
                break;
            case 2:
                for (i = 0; i < ngcp; i++){
                    X(i,0) = 1.0;
                    X(i,1) = rix[i];
                    X(i,2) = riy[i];
                    X(i,3) = rix[i]*riy[i];
                    X(i,4) = rix[i]*rix[i];
                    X(i,5) = riy[i]*riy[i];
                    y(i) = rox[i];
                }
                break;
            case 3:
                for (i = 0; i < ngcp; i++){
                    X(i,0) = 1.0;
                    X(i,1) = rix[i];
                    X(i,2) = riy[i];
                    X(i,3) = rix[i]*riy[i];
                    X(i,4) = rix[i]*rix[i];
                    X(i,5) = riy[i]*riy[i];
                    X(i,6) = rix[i]*rix[i]*riy[i];
                    X(i,7) = rix[i]*riy[i]*riy[i];
                    X(i,8) = rix[i]*rix[i]*rix[i];
                    X(i,9) = riy[i]*riy[i]*riy[i];
                    y(i) = rox[i];
                }
                break;
            case 4:
                for (i = 0; i < ngcp; i++){
                    X(i,0) = 1.0;
                    X(i,1) = rix[i];
                    X(i,2) = riy[i];
                    X(i,3) = rix[i]*riy[i];
                    X(i,4) = rix[i]*rix[i];
                    X(i,5) = riy[i]*riy[i];
                    X(i,6) = rix[i]*rix[i]*riy[i];
                    X(i,7) = rix[i]*riy[i]*riy[i];
                    X(i,8) = rix[i]*rix[i]*rix[i];
                    X(i,9) = riy[i]*riy[i]*riy[i];
                    X(i,10) = rix[i]*rix[i]*riy[i]*riy[i];
                    X(i,11) = rix[i]*rix[i]*rix[i]*riy[i];
                    X(i,12) = rix[i]*riy[i]*riy[i]*riy[i];
                    X(i,13) = rix[i]*rix[i]*rix[i]*rix[i];
                    X(i,14) = riy[i]*riy[i]*riy[i]*riy[i];
                    y(i) = rox[i];
                }
                break;
            case 5:
                for (i = 0; i < ngcp; i++){
                    X(i,0) = 1.0;
                    X(i,1) = rix[i];
                    X(i,2) = riy[i];
                    X(i,3) = rix[i]*riy[i];
                    X(i,4) = rix[i]*rix[i];
                    X(i,5) = riy[i]*riy[i];
                    X(i,6) = rix[i]*rix[i]*riy[i];
                    X(i,7) = rix[i]*riy[i]*riy[i];
                    X(i,8) = rix[i]*rix[i]*rix[i];
                    X(i,9) = riy[i]*riy[i]*riy[i];
                    X(i,10) = rix[i]*rix[i]*riy[i]*riy[i];
                    X(i,11) = rix[i]*rix[i]*rix[i]*riy[i];
                    X(i,12) = rix[i]*riy[i]*riy[i]*riy[i];
                    X(i,13) = rix[i]*rix[i]*rix[i]*rix[i];
                    X(i,14) = riy[i]*riy[i]*riy[i]*riy[i];
                    X(i,15) = rix[i]*rix[i]*rix[i]*riy[i]*riy[i];
                    X(i,16) = rix[i]*rix[i]*riy[i]*riy[i]*riy[i];
                    X(i,17) = rix[i]*rix[i]*rix[i]*rix[i]*riy[i];
                    X(i,18) = rix[i]*riy[i]*riy[i]*riy[i]*riy[i];
                    X(i,19) = rix[i]*rix[i]*rix[i]*rix[i]*rix[i];
                    X(i,20) = riy[i]*riy[i]*riy[i]*riy[i]*riy[i];
                    y(i) = rox[i];
                }
                break;
        }
        
        //c = X.jacobiSvd(ComputeThinU | ComputeThinV).solve(y);
        c = X.fullPivHouseholderQr().solve(y);   // BEST SO FAR
        //c = X.jacobiSvd<FullPivHouseholderQRPreconditioner>().solve(y);  bad
        //c = X.jacobiSvd(ComputeU | ComputeV).solve(y);  bad
        //c = X.jacobiSvd().compute().solve(y);  bad
        //c = X.jacobiSvd(ComputeThinU | ComputeThinV).compute(y);   bad
        //c = X.jacobiSvd().compute(y);   bad
        
        pbx = j;
        //std::cerr << "rbx's " << std::endl;
        for (i = 0; i < j; i++){
            rbx(i) = c(i);
            //std::cerr << rbx(i) << " ";
        }
        
        // REPEAT FOR y-COORDINATES:
        // X is still correct from the above stuff
        for (i = 0; i < ngcp; i++){
            y(i) = roy[i];
        }
        
        //c = X.jacobiSvd(ComputeThinU | ComputeThinV).solve(y);
        c = X.fullPivHouseholderQr().solve(y);

        pby = j;
        //std::cerr << std::endl << "rby's " << std::endl;
        for (i = 0; i < j; i++){
            rby(i) = c(i);
            //std::cerr << rby(i) << " ";
        }
        //std::cerr << std::endl;
        
    }// end: regression2D

    
    // determine the coefficients for a bilinear polynomial (of a specified order), for
    //  modelling 2D GCPs by regression analysis
    void WarpParameters::GCPRegression2D(const std::vector<double> rix, const std::vector<double> riy,
                               const std::vector<double> rox, const std::vector<double> roy,
                               int *order, double *rmsx, double *rmsy) {
        double dx, dy;
        double a,b;
        
        int ngcp = std::min(rix.size(),riy.size());
        int ngcp2 = std::min(rox.size(),roy.size());
        ngcp = std::min (ngcp,ngcp2);

        if(ngcp==0) {
            /* identity transform: */
            pbx = 3;
            pby = 3;
            rbx(0) = 0.0;
            rbx(1) = 1.0;
            rbx(2) = 0.0;
            rby(0) = 0.0;
            rby(1) = 0.0;
            rby(2) = 1.0;
            *order=1;
            
        } else if( ngcp==1) {
            /* offset transform: */
            dx = rox[0] - rix[0];
            dy = roy[0] - riy[0];
            pbx = 3;
            pby = 3;
            rbx(0) = dx;
            rbx(1) = 1.0;
            rbx(2) = 0.0;
            rby(0) = dy;
            rby(1) = 0.0;
            rby(2) = 1.0;
            *order=1;
            
        } else if (ngcp == 2) {
            /* offset and scaling: */
            /* solve simple xi' = a + b xi, i=1,2 */
            if( (rix[1]-rix[0]) == 0.0 ) {
                b=1.0;
            } else {
                b=(rox[1]-rox[0])/(rix[1]-rix[0]);
            }
            a = rox[0] - b*rix[0];
            pbx = 3;
            pby = 3;
            rbx(0) = a;
            rbx(1) = b;
            rbx(2) = 0.0;
            
            /* solve simple yi' = a + b yi, i=1,2 */
            if( (riy[1]-riy[0]) == 0.0 ) {
                b=1.0;
            } else {
                b=(roy[1]-roy[0])/(riy[1]-riy[0]);
            }
            a = roy[0] - b*riy[0];
            rby(0) = a;
            rby(1) = 0.0;
            rby(2) = b;
            
            *order =1;
            
        } else if( ngcp>=3) {
            /* general case: */
            regression2D(rix, riy,  rox, roy, order );
        }
        
        GCPRootMeanSq (rix,riy,rox,roy,rmsx,rmsy);
    }// end: GCPRegression2D

}// end namespace GeoStar
