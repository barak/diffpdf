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

#include "optionsform.hpp"
#include <QtGui>

QPixmap penStyleSwatch(const Qt::PenStyle style, const QColor
        &color=Qt::black, const QSize &size=QSize(32, 24));

QPixmap penStyleSwatch(const Qt::PenStyle style, const QColor &color,
                       const QSize &size)
{
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QPen pen(style);
    pen.setColor(color);
    pen.setWidth(3);
    painter.setPen(pen);
    const int Y = size.height() / 2;
    painter.drawLine(0, Y, size.width(), Y);
    painter.end();
    return pixmap;
}


OptionsForm::OptionsForm(QWidget *parent)
    : QDialog(parent)
{
    QSettings settings;

    QPushButton *highlightColorButton = new QPushButton(
            tr("Highlight &Color..."));
    highlightColorLabel = new QLabel();
    QPixmap pixmap(60, 3);
    highlight_color = settings.value("HighlightColor", Qt::yellow)
                                     .value<QColor>();
    pixmap.fill(highlight_color);
    highlightColorLabel->setPixmap(pixmap);
    QLabel *lineStyleLabel = new QLabel(tr("&Line Style:"));
    lineStyleComboBox = new QComboBox();
    updateLineStyleCombobox();
    lineStyleLabel->setBuddy(lineStyleComboBox);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok|QDialogButtonBox::Cancel);

    QGridLayout *layout = new QGridLayout();
    layout->addWidget(highlightColorButton, 0, 0);
    layout->addWidget(highlightColorLabel, 0, 1);
    layout->addWidget(lineStyleLabel, 1, 0);
    layout->addWidget(lineStyleComboBox, 1, 1);
    layout->addWidget(buttonBox, 3, 0, 1, 2);
    setLayout(layout);
    highlightColorButton->setFocus();

    connect(highlightColorButton, SIGNAL(clicked()),
            this, SLOT(setColor()));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    setWindowTitle(tr("DiffPDF - Options"));
}


void OptionsForm::updateLineStyleCombobox()
{
    int index = lineStyleComboBox->currentIndex();
    lineStyleComboBox->clear();
    lineStyleComboBox->addItem(QIcon(penStyleSwatch(Qt::SolidLine,
            highlight_color)), tr("Solid Line"), Qt::SolidLine);
    lineStyleComboBox->addItem(QIcon(penStyleSwatch(Qt::DashLine,
            highlight_color)), tr("Dashed Line"), Qt::DashLine);
    lineStyleComboBox->addItem(QIcon(penStyleSwatch(Qt::DotLine,
            highlight_color)), tr("Dotted Line"), Qt::DotLine);
    lineStyleComboBox->addItem(QIcon(penStyleSwatch(Qt::DashDotLine,
            highlight_color)), tr("Dash-Dotted Line"), Qt::DashDotLine);
    lineStyleComboBox->addItem(QIcon(penStyleSwatch(Qt::DashDotDotLine,
            highlight_color)), tr("Dash-Dot-Dotted Line"),
            Qt::DashDotDotLine);
    if (index == -1) {
        QSettings settings;
        index = lineStyleComboBox->findData(settings.value("LineStyle"));
        if (index == -1)
            index = 0;
    }
    lineStyleComboBox->setCurrentIndex(index);
}


void OptionsForm::setColor()
{
    QPixmap pixmap(60, 3);
    QColor color = QColorDialog::getColor(highlight_color, this);
    if (color.isValid()) {
        highlight_color = color;
        pixmap.fill(highlight_color);
        highlightColorLabel->setPixmap(pixmap);
        updateLineStyleCombobox();
    }
}


void OptionsForm::accept()
{
    QSettings settings;
    settings.setValue("HighlightColor", highlight_color);
    settings.setValue("LineStyle", lineStyleComboBox->itemData(
                      lineStyleComboBox->currentIndex()));
    QDialog::accept();
}
