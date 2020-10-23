#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP
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

#if QT_VERSION >= 0x040600
#include <QSharedPointer>
#else
#include <tr1/memory>
#endif
#include <poppler-qt4.h>
#include <QBrush>
#include <QList>
#include <QMainWindow>
#include <QPen>

class QBoxLayout;
class QComboBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QScrollArea;
class QSpinBox;
class QSplitter;

#if QT_VERSION >= 0x040600
typedef QSharedPointer<Poppler::Document> PdfDocument;
typedef QSharedPointer<Poppler::Page> PdfPage;
typedef QSharedPointer<Poppler::TextBox> PdfTextBox;
#else
typedef std::tr1::shared_ptr<Poppler::Document> PdfDocument;
typedef std::tr1::shared_ptr<Poppler::Page> PdfPage;
typedef std::tr1::shared_ptr<Poppler::TextBox> PdfTextBox;
#endif
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
    bool eventFilter(QObject *object, QEvent *event);

private slots:
    void setFile1(QString filename=QString());
    void setFile2(QString filename=QString());
    void compare();
    void options();
    void about();
    void initialize(const QString &filename1, const QString &filename2);
    void updateUi();
    void updateViews(int index=-1);
    void controlDockLocationChanged(Qt::DockWidgetArea area);
    void actionDockLocationChanged(Qt::DockWidgetArea area);
    void controlTopLevelChanged(bool floating);
    void actionTopLevelChanged(bool floating);
    void logTopLevelChanged(bool floating);

private:
    enum Difference {NoDifference, TextualDifference, VisualDifference};
    enum Comparison {TextOld, Text, Appearance};

    void createWidgets(const QString &filename1, const QString &filename2);
    void createCentralArea();
    void createDockWidgets();
    void createConnections();
    QPair<int, int> comparePages(const QString &filename1,
            const PdfDocument &pdf1, const QString &filename2,
            const PdfDocument &pdf2);
    void comparePrepareUi();
    void compareUpdateUi(const QPair<int, int> &pair);
    int writeFileInfo(const QString &filename);
    void writeLine(const QString &text);
    void writeError(const QString &text);
    PdfDocument getPdf(const QString &filename);
    QList<int> getPageList(int which, PdfDocument pdf);
    Difference getTheDifference(PdfPage page1, PdfPage page2);
    void paintOnImage(const QPainterPath &path, QImage *image);
    void updateViews(const PdfDocument &pdf1, const PdfPage &page1,
            const PdfDocument &pdf2, const PdfPage &page2,
            bool hasVisualDifference, const QString &key1,
            const QString &key2);
    void computeTextHighlights(QPainterPath *highlighted1,
            QPainterPath *highlighted2, const PdfPage &page1,
            const PdfPage &page2, const int DPI);
    void computeTextHighlightsOld(QPainterPath *highlighted1,
            QPainterPath *highlighted2, const PdfPage &page1,
            const PdfPage &page2, const int DPI);
    void computeVisualHighlights(QPainterPath *highlighted1,
        QPainterPath *highlighted2, const QImage &plainImage1,
        const QImage &plainImage2);
    void addHighlighting(QRectF *bigRect, QPainterPath *highlighted,
            const PdfTextBox &box, const int OVERLAP, const int DPI,
            const bool COMBINE=true);

    QPushButton *setFile1Button;
    QLineEdit *filename1LineEdit;
    QLabel *comparePages1Label;
    QLineEdit *pages1LineEdit;
    QLabel *page1Label;
    QScrollArea *area1;
    QPushButton *setFile2Button;
    QLineEdit *filename2LineEdit;
    QLabel *comparePages2Label;
    QLineEdit *pages2LineEdit;
    QLabel *page2Label;
    QScrollArea *area2;
    QLabel *comparisonLabel;
    QComboBox *comparisonComboBox;
    QLabel *viewDiffLabel;
    QPushButton *compareButton;
    QComboBox *viewDiffComboBox;
    QLabel *statusLabel;
    QLabel *zoomLabel;
    QSpinBox *zoomSpinBox;
    QPushButton *optionsButton;
    QPushButton *aboutButton;
    QPushButton *quitButton;
    QPlainTextEdit *logEdit;
    QSplitter *splitter;
    QBoxLayout *controlLayout;
    QDockWidget *controlDockWidget;
    QBoxLayout *actionLayout;
    QDockWidget *actionDockWidget;
    QDockWidget *logDockWidget;

    QBrush brush;
    QPen pen;
    QString currentPath;
    Qt::DockWidgetArea controlDockArea;
    Qt::DockWidgetArea actionDockArea;
    bool cancel;
    bool showToolTips;
    bool combineTextHighlighting;
};

#endif // MAINWINDOW_HPP

