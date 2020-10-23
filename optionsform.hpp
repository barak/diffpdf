#ifndef OPTIONSFORM_HPP
#define OPTIONSFORM_HPP
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

#include <QBrush>
#include <QDialog>
#include <QPen>

class QCheckBox;
class QComboBox;
class QDialogButtonBox;


class OptionsForm : public QDialog
{
    Q_OBJECT

public:
    OptionsForm(QPen *pen, QBrush *brush, bool *showToolTips,
                QWidget *parent=0);

private slots:
    void updateColor(int index);
    void updateBrushStyle(int index);
    void updatePenStyle(int index);
    void updateUi();
    void accept();

private:
    void createWidgets();
    void createLayout();
    void createConnections();
    void updateSwatches();

    QComboBox *colorComboBox;
    QComboBox *brushStyleComboBox;
    QComboBox *penStyleComboBox;
    QCheckBox *showToolTipsCheckBox;
    QDialogButtonBox *buttonBox;

    QPen *m_pen;
    QBrush *m_brush;
    bool *m_showToolTips;
    QPen pen;
    QBrush brush;
};

#endif // OPTIONSFORM_HPP


