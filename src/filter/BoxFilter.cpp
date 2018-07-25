//
//  BoxFilter.cpp
//  ABC-Ray
//
//  Created by libingzeng on 2018/7/7.
//  Copyright © 2018 曾立兵. All rights reserved.
//

#include <stdio.h>
#include "BoxFilter.h"

BoxFilter::BoxFilter(void)
    : Filter(Vector2D(0.5f, 0.5f), 16)
{}


BoxFilter::BoxFilter(const Vector2D &radius)
    : Filter(radius, 16)
{}


// Gaussian Filter Method Definitions
float BoxFilter::Evaluate(const Point2D &p) const {
    return 1.0;
}


void BoxFilter::fillFilterTable(void) const{
    for (int y = 0; y < filterTableWidth; ++y) {
        for (int x = 0; x < filterTableWidth; ++x) {
            Point2D p;
            p.x = (x + 0.5f) * radius.x / filterTableWidth;
            p.y = (y + 0.5f) * radius.y / filterTableWidth;
            filterTable.push_back(Evaluate(p));
        }
    }
}
