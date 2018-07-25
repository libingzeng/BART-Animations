//
//  MitchellFilter.h
//  ABC-Ray
//
//  Created by libingzeng on 2018/7/7.
//  Copyright © 2018 曾立兵. All rights reserved.
//

#ifndef MitchellFilter_h
#define MitchellFilter_h

#include "Filter.h"
#include "RGBColor.h"
#include <math.h>

// Mitchell Filter Declarations
class MitchellFilter : public Filter {
public:
    // MitchellFilter Public Methods
    MitchellFilter(void);
    MitchellFilter(const Vector2D &radius, float B, float C);
    
    ~MitchellFilter(){}
    
    float Evaluate(const Point2D &p) const;
    void fillFilterTable(void) const;

    float Mitchell1D(float x) const {
        x = fabs(2 * x);
        if (x > 1)
            return ((-B - 6 * C) * x * x * x + (6 * B + 30 * C) * x * x +
                    (-12 * B - 48 * C) * x + (8 * B + 24 * C)) * (1.f / 6.f);
        else
            return ((12 - 9 * B - 6 * C) * x * x * x +
                    (-18 + 12 * B + 6 * C) * x * x + (6 - 2 * B)) * (1.f / 6.f);
    }
    
private:
    const float B, C;
};


#endif /* MitchellFilter_h */
