//
//  Gaussian.h
//  ABC-Ray
//
//  Created by libingzeng on 2018/7/7.
//  Copyright © 2018 曾立兵. All rights reserved.
//

#ifndef Gaussian_h
#define Gaussian_h

#include "Macros.h"
#include "Filter.h"
#include "RGBColor.h"
#include <math.h>



// Gaussian Filter Declarations
class GaussianFilter : public Filter {
public:
    // GaussianFilter Public Methods
    GaussianFilter(void);
    GaussianFilter(const Vector2D &radius, float alpha);
    
    ~GaussianFilter(){}

    float Evaluate(const Point2D &p) const;
    void fillFilterTable(void) const;

private:
    // GaussianFilter Private Data
    const float alpha;
    const float expX, expY;
    
    // GaussianFilter Utility Functions
    float Gaussian(float d, float expv) const {
        return (fmax((float)0, float(exp(-alpha * d * d) - expv)));
    }
};


#endif /* Gaussian_h */
