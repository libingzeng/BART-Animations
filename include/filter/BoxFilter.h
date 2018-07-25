//
//  BoxFilter.h
//  ABC-Ray
//
//  Created by libingzeng on 2018/7/7.
//  Copyright © 2018 曾立兵. All rights reserved.
//

#ifndef BoxFilter_h
#define BoxFilter_h

#include "Macros.h"
#include "Filter.h"
#include "RGBColor.h"
#include <math.h>



// Gaussian Filter Declarations
class BoxFilter : public Filter {
public:
    // BoxFilter Public Methods
    BoxFilter(void);
    BoxFilter(const Vector2D &radius);
    
    ~BoxFilter(){}

    float Evaluate(const Point2D &p) const;
    void fillFilterTable(void) const;
};


#endif /* BoxFilter_h */
