/*
    Copyright (c) 2008-10 Qtrac Ltd. All rights reserved.
    This program or module is free software: you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 2 of
    the License, or (at your option) any later version. This program is
    distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
    for more details.
*/

#include "generic.hpp"
#include <QRectF>


void scaleRect(int dpi, QRectF *rect)
{
    if (dpi == DPI_FACTOR)
        return;
    qreal x1;
    qreal y1;
    qreal x2;
    qreal y2;
    rect->getCoords(&x1, &y1, &x2, &y2);
    qreal factor = dpi / qreal(DPI_FACTOR);
    x1 *= factor;
    y1 *= factor;
    x2 *= factor;
    y2 *= factor;
    rect->setCoords(x1, y1, x2, y2);
}


