//
//  filter.h
//  ABC-Ray
//
//  Created by libingzeng on 2018/7/7.
//  Copyright © 2018 曾立兵. All rights reserved.
//

#ifndef Filter_h
#define Filter_h

#include "Vector2D.h"
#include "Point2D.h"
#include <vector>

// Filter Declarations
class Filter {
public:
    // Filter Interface
    virtual ~Filter();
    Filter(const Vector2D &radius, const int width)
        : radius(radius), invRadius(Vector2D(1 / radius.x, 1 / radius.y)), filterTableWidth(width) {}
    virtual float Evaluate(const Point2D &p) const = 0;
    virtual void fillFilterTable(void) const = 0;

    // Filter Public Data
    const Vector2D radius, invRadius;
    int filterTableWidth;
    mutable std::vector<float> filterTable;
};

#endif /* Filter_h */
