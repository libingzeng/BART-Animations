//
//  GaussianFilter.cpp
//  ABC-Ray
//
//  Created by libingzeng on 2018/7/7.
//  Copyright © 2018 曾立兵. All rights reserved.
//

#include <stdio.h>
#include "GaussianFilter.h"

GaussianFilter::GaussianFilter(void)
    : Filter(Vector2D(2.0f, 2.0f), 16),
    alpha(2.0f),
    expX(exp(-2.0f * 2.0f * 2.0f)),
    expY(exp(-alpha * 2.0f * 2.0f))
{}


GaussianFilter::GaussianFilter(const Vector2D &radius, float alpha)
    : Filter(radius, 16),
    alpha(alpha),
    expX(exp(-alpha * radius.x * radius.x)),
    expY(exp(-alpha * radius.y * radius.y))
{}


// Gaussian Filter Method Definitions
float GaussianFilter::Evaluate(const Point2D &p) const {
    return Gaussian(p.x, expX) * Gaussian(p.y, expY);
}


void GaussianFilter::fillFilterTable(void) const{
    for (int y = 0; y < filterTableWidth; ++y) {
        for (int x = 0; x < filterTableWidth; ++x) {
            Point2D p;
            p.x = (x + 0.5f) * radius.x / filterTableWidth;
            p.y = (y + 0.5f) * radius.y / filterTableWidth;
            filterTable.push_back(Evaluate(p));
        }
    }
}
