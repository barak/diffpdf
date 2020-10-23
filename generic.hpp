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

#ifndef GENERIC_HPP
#define GENERIC_HPP

#include <QMetaType>

class QRectF;


const int DPI_FACTOR = 72;


struct PagePair
{
    PagePair(int l=-1, int r=-1, bool v=false)
        : left(l), right(r), visual_difference(v) {}

    bool isNull() { return left == -1 || right == -1; }

    const int left;
    const int right;
    const bool visual_difference;
};
Q_DECLARE_METATYPE(PagePair)


void scaleRect(int dpi, QRectF *rect);

#endif // GENERIC_HPP


