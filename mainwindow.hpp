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

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <tr1/memory>
#include <poppler-qt4.h>
#include <QList>
#include <QMainWindow>


class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QSplitter;
class QTextBrowser;

typedef std::tr1::shared_ptr<Poppler::Document> PdfDocument;
typedef std::tr1::shared_ptr<Poppler::Page> PdfPage;
typedef std::tr1::shared_ptr<Poppler::TextBox> PdfTextBox;
typedef QList<PdfTextBox> TextBoxList;


TextBoxList getTextBoxes(PdfPage page);


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(const QString &filename1, const QString &filename2,
               QWidget *parent=0);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void setFile1(QString filename=QString());
    void setFile2(QString filename=QString());
    void compare();
    void options();
    void about();

    void updateUi();
    void updateViews(int index=-1);

private:
    enum Difference {NoDifference, TextualDifference, VisualDifference};

    int writeFileInfo(const QString &filename);
    void writeLine(const QString &text);
    void writeError(const QString &text);
    PdfDocument getPdf(const QString &filename);
    QList<int> getPageList(int which, PdfDocument pdf);
    Difference comparePages(PdfPage page1, PdfPage page2);
    QRect minimumSizedRect(const QRect &rect);
    void paintRectsOnImage(const QRegion &region, QImage *image);

    QLabel *file1Label;
    QLabel *file2Label;
    QLineEdit *pages1LineEdit;
    QLineEdit *pages2LineEdit;
    QCheckBox *compareAppearanceCheckBox;
    QTextBrowser *resultsBrowser;
    QPushButton *setFile2Button;
    QPushButton *compareButton;
    QComboBox *viewDiffComboBox;
    QSpinBox *zoomSpinBox;
    QLabel *page1Label;
    QLabel *page2Label;
    QSplitter *topSplitter;
    QSplitter *bottomSplitter;

    QString current_path;
    bool cancel;
};

#endif // MAINWINDOW_HPP

