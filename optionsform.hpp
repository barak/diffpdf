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

#ifndef OPTIONSFORM_HPP
#define OPTIONSFORM_HPP

#include <QDialog>

class QColor;
class QComboBox;
class QLabel;
class QSpinBox;


class OptionsForm : public QDialog
{
    Q_OBJECT

public:
    OptionsForm(QWidget *parent=0);


private slots:
    void setColor();
    void accept();

private:
    void updateLineStyleCombobox();

    QComboBox *lineStyleComboBox;
    QLabel *highlightColorLabel;

    QColor highlight_color;
};

#endif // OPTIONSFORM_HPP


