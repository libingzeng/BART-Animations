//
//  MitchellFilter.cpp
//  ABC-Ray
//
//  Created by libingzeng on 2018/7/7.
//  Copyright © 2018 曾立兵. All rights reserved.
//

#include <stdio.h>

#include "MitchellFilter.h"


MitchellFilter::MitchellFilter(void)
    : Filter(Vector2D(2.f, 2.f), 16), B(1.f / 3.f), C(1.f / 3.f) {}


MitchellFilter::MitchellFilter(const Vector2D &radius, float B, float C)
    : Filter(radius, 16), B(B), C(C) {}


// Mitchell Filter Method Definitions
float MitchellFilter::Evaluate(const Point2D &p) const {
    return Mitchell1D(p.x * invRadius.x) * Mitchell1D(p.y * invRadius.y);
}


void MitchellFilter::fillFilterTable(void) const{
    for (int y = 0; y < filterTableWidth; ++y) {
        for (int x = 0; x < filterTableWidth; ++x) {
            Point2D p;
            p.x = (x + 0.5f) * radius.x / filterTableWidth;
            p.y = (y + 0.5f) * radius.y / filterTableWidth;
            filterTable.push_back(Evaluate(p));
        }
    }
}

