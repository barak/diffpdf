/*
    Copyright (c) 2008-11 Qtrac Ltd. All rights reserved.
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
#include "optionsform.hpp"
#include "mainwindow.hpp"
#include "sequence_matcher.hpp"
#include <QApplication>
#include <QBoxLayout>
#include <QComboBox>
#include <QDockWidget>
#include <QEvent>
#include <QFileDialog>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPixmapCache>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSettings>
#include <QSpinBox>
#include <QSplitter>


TextBoxList getTextBoxes(PdfPage page)
{
    TextBoxList boxes;
    foreach (Poppler::TextBox *box, page->textList()) {
        PdfTextBox box_ptr(box);
        boxes.append(box_ptr);
    }
    return boxes;
}


MainWindow::MainWindow(const QString &filename1, const QString &filename2,
                       QWidget *parent)
    : QMainWindow(parent), currentPath("."),
      controlDockArea(Qt::RightDockWidgetArea),
      actionDockArea(Qt::RightDockWidgetArea), cancel(false)
{
    QSettings settings;
    pen.setStyle(Qt::NoPen);
    pen.setColor(Qt::red);
    pen = settings.value("Outline", pen).value<QPen>();
    brush.setColor(pen.color());
    brush.setStyle(Qt::SolidPattern);
    brush = settings.value("Fill", brush).value<QBrush>();
    showToolTips = settings.value("ShowToolTips", true).toBool();
    combineTextHighlighting = settings.value("CombineTextHighlighting",
            true).toBool();
    QPixmapCache::setCacheLimit(1000 *
            qBound(1, settings.value("CacheSizeMB", 25).toInt(), 100));

    createWidgets(filename1, filename2);
    createCentralArea();
    createDockWidgets();
    createConnections();

    restoreGeometry(settings.value("MainWindow/Geometry").toByteArray());
    controlDockLocationChanged(static_cast<Qt::DockWidgetArea>(
                settings.value("MainWindow/ControlDockArea",
                        static_cast<int>(controlDockArea)).toInt()));
    actionDockLocationChanged(static_cast<Qt::DockWidgetArea>(
                settings.value("MainWindow/ActionDockArea",
                        static_cast<int>(actionDockArea)).toInt()));
    restoreState(settings.value("MainWindow/State").toByteArray());
    controlDockWidget->resize(controlDockWidget->minimumSizeHint());
    actionDockWidget->resize(actionDockWidget->minimumSizeHint());

    setWindowTitle(tr("DiffPDF"));
    setWindowIcon(QIcon(":/icon.png"));

    QMetaObject::invokeMethod(this, "initialize", Qt::QueuedConnection,
            Q_ARG(QString, filename1),
            Q_ARG(QString, filename2));
}


void MainWindow::createWidgets(const QString &filename1,
                               const QString &filename2)
{
    setFile1Button = new QPushButton(tr("File #&1..."));
    setFile1Button->setToolTip(tr("<p>Choose the first (left hand) file "
                "to be compared."));
    filename1LineEdit = new QLineEdit;
    filename1LineEdit->setToolTip(tr("The first (left hand) file."));
    filename1LineEdit->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    filename1LineEdit->setMinimumWidth(100);
    filename1LineEdit->setText(filename1);
    setFile2Button = new QPushButton(tr("File #&2..."));
    setFile2Button->setToolTip(tr("<p>Choose the second (right hand) file "
                "to be compared."));
    filename2LineEdit = new QLineEdit;
    filename2LineEdit->setToolTip(tr("The second (right hand) file."));
    filename2LineEdit->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    filename2LineEdit->setMinimumWidth(100);
    filename2LineEdit->setText(filename2);
    comparePages1Label = new QLabel(tr("Pag&es:"));
    pages1LineEdit = new QLineEdit;
    comparePages1Label->setBuddy(pages1LineEdit);
    pages1LineEdit->setToolTip(tr("<p>Pages can be specified using ranges "
                "such as 1-10, and multiple ranges can be used, e.g., "
                "1-10, 12-15, 20, 22, 35-39. This makes it "
                "straighforward to compare similar documents where one "
                "has one or more additional pages.<p>For example, if "
                "file1.pdf has pages 1-30 and file2.pdf has pages 1-31 "
                "with the extra page being page 14, the two page ranges "
                "would be set to 1-30 for file1.pdf and 1-13, 15-31 for "
                "file2.pdf."));
    comparePages2Label = new QLabel(tr("Pa&ges:"));
    pages2LineEdit = new QLineEdit;
    comparePages2Label->setBuddy(pages2LineEdit);
    pages2LineEdit->setToolTip(pages1LineEdit->toolTip());
    compareButton = new QPushButton(tr("&Compare"));
    compareButton->setEnabled(false);
    compareButton->setToolTip(tr("<p>Click to compare (or re-compare) "
                "the documents&mdash;or to cancel a comparison that's "
                "in progress."));
    comparisonGroupBox = new QGroupBox(tr("Compare"));
#ifdef Q_WS_X11
    int left, top, right, bottom;
    comparisonGroupBox->getContentsMargins(&left, &top, &right, &bottom);
    const int height = QFontMetrics(comparisonGroupBox->font()).height();
    top = qMax(height / 2, top / 2);
    comparisonGroupBox->setContentsMargins(left, top, right, 0);
#endif
    compareAppearanceRadioButton = new QRadioButton(tr("&Appearance"));
    compareTextRadioButton = new QRadioButton(tr("&Text"));
    compareAppearanceRadioButton->setToolTip(
            tr("<p>If the <b>Text</b> comparison "
               "mode is chosen, then each page's text is compared. "
               "If the <b>Appearance</b> comparison mode is chosen "
               "then each page's visual appearance is compared. "
               "Comparing appearance can be slow for large documents "
               "and can also produce false positives."));
    compareTextRadioButton->setToolTip(
            compareAppearanceRadioButton->toolTip());
    compareTextRadioButton->setChecked(true);
    viewDiffLabel = new QLabel(tr("&View:"));
    viewDiffLabel->setToolTip(tr("<p>Shows each pair of pages which "
                "are different. The comparison is textual unless the "
                "<b>Appearance</b> comparison mode is chosen, in "
                "which case the comparison is done visually. "
                "Visual differences can occur if a paragraph is "
                "formated differently or if an embedded diagram or "
                "image has changed."));
    viewDiffComboBox = new QComboBox;
    viewDiffComboBox->addItem(tr("(Not viewing)"));
    viewDiffLabel->setBuddy(viewDiffComboBox);
    viewDiffComboBox->setToolTip(viewDiffLabel->toolTip());
    previousButton = new QPushButton("&Previous");
    previousButton->setToolTip(
            "<p>Navigate to the previous pair of pages.");
#if QT_VERSION >= 0x040600
    previousButton->setIcon(QIcon(":/left.png"));
#endif
    nextButton = new QPushButton("&Next");
    nextButton->setToolTip("<p>Navigate to the next pair of pages.");
#if QT_VERSION >= 0x040600
    nextButton->setIcon(QIcon(":/right.png"));
#endif
    zoomLabel = new QLabel(tr("&Zoom:"));
    zoomLabel->setToolTip(tr("<p>Determines the scale at which the "
                "pages are shown."));
    zoomSpinBox = new QSpinBox;
    zoomLabel->setBuddy(zoomSpinBox);
    zoomSpinBox->setRange(25, 400);
    zoomSpinBox->setSuffix(tr(" %"));
    zoomSpinBox->setSingleStep(25);
    QSettings settings;
    zoomSpinBox->setValue(settings.value("Zoom", 100).toInt());
    zoomSpinBox->setToolTip(zoomLabel->toolTip());
    statusLabel = new QLabel(tr("Choose files..."));
    statusLabel->setFrameStyle(QFrame::StyledPanel|QFrame::Sunken);
    statusLabel->setMaximumHeight(statusLabel->minimumSizeHint().height());
    optionsButton = new QPushButton(tr("&Options..."));
    optionsButton->setToolTip(tr("Click to customize the application."));
    helpButton = new QPushButton(tr("Help"));
    helpButton->setShortcut(tr("F1"));
    helpButton->setToolTip(tr("Click for bare bones help."));
    aboutButton = new QPushButton(tr("A&bout"));
    aboutButton->setToolTip(tr("Click for copyright and credits."));
    quitButton = new QPushButton(tr("&Quit"));
    quitButton->setToolTip(tr("Click to terminate the application."));
    page1Label = new QLabel;
    page1Label->setAlignment(Qt::AlignCenter);
    page1Label->setToolTip(tr("<p>Shows the first (left hand) document's "
                "page that corresponds to the page shown in the "
                "View Difference combobox."));
    page2Label = new QLabel;
    page2Label->setAlignment(Qt::AlignCenter);
    page2Label->setToolTip(tr("<p>Shows the second (right hand) "
                "document's page that corresponds to the page shown in "
                "the View Difference combobox."));
    logEdit = new QPlainTextEdit;

    foreach (QWidget *widget, QList<QWidget*>() << setFile1Button
            << filename1LineEdit << pages1LineEdit << page1Label
            << setFile2Button << filename2LineEdit << pages2LineEdit
            << page2Label << compareButton << compareAppearanceRadioButton
            << compareTextRadioButton
            << viewDiffLabel << viewDiffComboBox << zoomLabel
            << zoomSpinBox << optionsButton << helpButton << aboutButton
            << quitButton << logEdit << previousButton << nextButton)
        if (!widget->toolTip().isEmpty())
            widget->installEventFilter(this);
}


void MainWindow::createCentralArea()
{
    QHBoxLayout *topLeftLayout = new QHBoxLayout;
    topLeftLayout->addWidget(setFile1Button);
    topLeftLayout->addWidget(filename1LineEdit, 3);
    topLeftLayout->addWidget(comparePages1Label);
    topLeftLayout->addWidget(pages1LineEdit, 2);
    area1 = new QScrollArea;
    area1->setWidget(page1Label);
    area1->setWidgetResizable(true);
    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addLayout(topLeftLayout);
    leftLayout->addWidget(area1, 1);
    QWidget *leftWidget = new QWidget;
    leftWidget->setLayout(leftLayout);

    QHBoxLayout *topRightLayout = new QHBoxLayout;
    topRightLayout->addWidget(setFile2Button);
    topRightLayout->addWidget(filename2LineEdit, 3);
    topRightLayout->addWidget(comparePages2Label);
    topRightLayout->addWidget(pages2LineEdit, 2);
    area2 = new QScrollArea;
    area2->setWidget(page2Label);
    area2->setWidgetResizable(true);
    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addLayout(topRightLayout);
    rightLayout->addWidget(area2, 1);
    QWidget *rightWidget = new QWidget;
    rightWidget->setLayout(rightLayout);

    splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(leftWidget);
    splitter->addWidget(rightWidget);
    QSettings settings;
    splitter->restoreState(settings.value("MainWindow/ViewSplitter")
            .toByteArray());

    setCentralWidget(splitter);
}


void MainWindow::createDockWidgets()
{
    setDockOptions(QMainWindow::AnimatedDocks);
    QDockWidget::DockWidgetFeatures features =
            QDockWidget::DockWidgetMovable|
            QDockWidget::DockWidgetFloatable;

    controlDockWidget = new QDockWidget(tr("Controls"), this);
    controlDockWidget->setObjectName("Controls");
    controlDockWidget->setFeatures(features);
    controlLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    QHBoxLayout *compareLayout = new QHBoxLayout;
    compareLayout->addWidget(compareAppearanceRadioButton);
    compareLayout->addWidget(compareTextRadioButton);
    comparisonGroupBox->setLayout(compareLayout);
    controlLayout->addWidget(comparisonGroupBox);
    QHBoxLayout *viewLayout = new QHBoxLayout;
    viewLayout->addWidget(viewDiffLabel);
    viewLayout->addWidget(viewDiffComboBox);
    controlLayout->addLayout(viewLayout);
    QHBoxLayout *navigationLayout = new QHBoxLayout;
    navigationLayout->addWidget(previousButton);
    navigationLayout->addWidget(nextButton);
    controlLayout->addLayout(navigationLayout);
    QHBoxLayout *zoomLayout = new QHBoxLayout;
    zoomLayout->addWidget(zoomLabel);
    zoomLayout->addWidget(zoomSpinBox);
    controlLayout->addLayout(zoomLayout);
    controlLayout->addWidget(statusLabel);
    controlLayout->addStretch();
    QWidget *widget = new QWidget;
    widget->setLayout(controlLayout);
    controlDockWidget->setWidget(widget);
    addDockWidget(controlDockArea, controlDockWidget);

    actionDockWidget = new QDockWidget(tr("Actions"), this);
    actionDockWidget->setObjectName("Actions");
    actionDockWidget->setFeatures(features);
    actionLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    actionLayout->addWidget(compareButton);
    actionLayout->addWidget(optionsButton);
    QHBoxLayout *helpLayout = new QHBoxLayout;
    helpLayout->addWidget(helpButton);
    helpLayout->addWidget(aboutButton);
    actionLayout->addLayout(helpLayout);
    actionLayout->addWidget(quitButton);
    actionLayout->addStretch();
    widget = new QWidget;
    widget->setLayout(actionLayout);
    actionDockWidget->setWidget(widget);
    addDockWidget(actionDockArea, actionDockWidget);

    logDockWidget = new QDockWidget(tr("Log"), this);
    logDockWidget->setObjectName("Log");
    logDockWidget->setFeatures(features|QDockWidget::DockWidgetClosable);
    logDockWidget->setWidget(logEdit);
    addDockWidget(Qt::RightDockWidgetArea, logDockWidget);
}


void MainWindow::createConnections()
{
    connect(area1->verticalScrollBar(), SIGNAL(valueChanged(int)),
            area2->verticalScrollBar(), SLOT(setValue(int)));
    connect(area2->verticalScrollBar(), SIGNAL(valueChanged(int)),
            area1->verticalScrollBar(), SLOT(setValue(int)));
    connect(area1->horizontalScrollBar(), SIGNAL(valueChanged(int)),
            area2->horizontalScrollBar(), SLOT(setValue(int)));
    connect(area2->horizontalScrollBar(), SIGNAL(valueChanged(int)),
            area1->horizontalScrollBar(), SLOT(setValue(int)));

    connect(filename1LineEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(updateUi()));
    connect(filename2LineEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(updateUi()));

    connect(viewDiffComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateViews(int)));
    connect(viewDiffComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateUi()));
    connect(previousButton, SIGNAL(clicked()),
            this, SLOT(previousPages()));
    connect(nextButton, SIGNAL(clicked()), this, SLOT(nextPages()));
    connect(setFile1Button, SIGNAL(clicked()), this, SLOT(setFile1()));
    connect(setFile2Button, SIGNAL(clicked()), this, SLOT(setFile2()));
    connect(compareButton, SIGNAL(clicked()), this, SLOT(compare()));
    connect(zoomSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(updateViews()));
    connect(optionsButton, SIGNAL(clicked()), this, SLOT(options()));
    connect(helpButton, SIGNAL(clicked()), this, SLOT(help()));
    connect(aboutButton, SIGNAL(clicked()), this, SLOT(about()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(controlDockWidget,
            SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            this, SLOT(controlDockLocationChanged(Qt::DockWidgetArea)));
    connect(actionDockWidget,
            SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            this, SLOT(actionDockLocationChanged(Qt::DockWidgetArea)));
    connect(controlDockWidget, SIGNAL(topLevelChanged(bool)),
            this, SLOT(controlTopLevelChanged(bool)));
    connect(actionDockWidget, SIGNAL(topLevelChanged(bool)),
            this, SLOT(actionTopLevelChanged(bool)));
    connect(logDockWidget, SIGNAL(topLevelChanged(bool)),
            this, SLOT(logTopLevelChanged(bool)));
}


void MainWindow::initialize(const QString &filename1,
                            const QString &filename2)
{
    if (!filename1.isEmpty()) {
        setFile1(filename1);
        setFile2Button->setFocus();
        if (!filename2.isEmpty()) {
            setFile2(filename2);
            compare();
        }
    }
    else
        updateUi();
}


void MainWindow::updateUi()
{
    compareButton->setEnabled(!filename1LineEdit->text().isEmpty() &&
                              !filename2LineEdit->text().isEmpty());
    QPushButton *button = qobject_cast<QPushButton*>(focusWidget());
    bool enableNavigationButton = (button == previousButton ||
                                   button == nextButton);
    previousButton->setEnabled(viewDiffComboBox->count() > 1 &&
            viewDiffComboBox->currentIndex() > 0);
    nextButton->setEnabled(viewDiffComboBox->count() > 1 &&
            viewDiffComboBox->currentIndex() + 1 <
            viewDiffComboBox->count());
    if (enableNavigationButton && !(previousButton->isEnabled() &&
                                    nextButton->isEnabled())) {
        if (previousButton->isEnabled())
            previousButton->setFocus();
        else
            nextButton->setFocus();
    }
}


void MainWindow::controlDockLocationChanged(Qt::DockWidgetArea area)
{
    if (area == Qt::TopDockWidgetArea ||
        area == Qt::BottomDockWidgetArea)
        controlLayout->setDirection(QBoxLayout::LeftToRight);
    else
        controlLayout->setDirection(QBoxLayout::TopToBottom);
    controlDockArea = area;
}


void MainWindow::actionDockLocationChanged(Qt::DockWidgetArea area)
{
    if (area == Qt::TopDockWidgetArea ||
        area == Qt::BottomDockWidgetArea)
        actionLayout->setDirection(QBoxLayout::LeftToRight);
    else
        actionLayout->setDirection(QBoxLayout::TopToBottom);
    actionDockArea = area;
}


void MainWindow::controlTopLevelChanged(bool floating)
{
    controlLayout->setDirection(floating ? QBoxLayout::TopToBottom
                                         : QBoxLayout::LeftToRight);
    if (QWidget *widget = static_cast<QWidget*>(controlLayout->parent()))
        widget->setFixedSize(floating ? widget->minimumSizeHint()
                : QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    controlDockWidget->setWindowTitle(floating ? tr("DiffPDF - Controls")
                                               : tr("Controls"));
}


void MainWindow::actionTopLevelChanged(bool floating)
{
    actionLayout->setDirection(floating ? QBoxLayout::TopToBottom
                                        : QBoxLayout::LeftToRight);
    if (QWidget *widget = static_cast<QWidget*>(actionLayout->parent()))
        widget->setFixedSize(floating ? widget->minimumSizeHint()
                : QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    actionDockWidget->setWindowTitle(floating ? tr("DiffPDF - Actions")
                                              : tr("Actions"));
}


void MainWindow::logTopLevelChanged(bool floating)
{
    logDockWidget->setWindowTitle(floating ? tr("DiffPDF - Log")
                                           : tr("Log"));
}


void MainWindow::previousPages()
{
    int i = viewDiffComboBox->currentIndex();
    if (i > 0)
        viewDiffComboBox->setCurrentIndex(i - 1);
}


void MainWindow::nextPages()
{
    int i = viewDiffComboBox->currentIndex();
    if (i + 1 < viewDiffComboBox->count())
        viewDiffComboBox->setCurrentIndex(i + 1);
}


void MainWindow::updateViews(int index)
{
    if (index == 0) {
        page1Label->clear();
        page2Label->clear();
        return;
    }
    else if (index == -1)
        index = viewDiffComboBox->currentIndex();
    PagePair pair = viewDiffComboBox->itemData(index).value<PagePair>();
    if (pair.isNull())
        return;

    QString filename1 = filename1LineEdit->text();
    PdfDocument pdf1 = getPdf(filename1);
    if (!pdf1)
        return;
    PdfPage page1(pdf1->page(pair.left));
    if (!page1)
        return;

    QString filename2 = filename2LineEdit->text();
    PdfDocument pdf2 = getPdf(filename2);
    if (!pdf2)
        return;
    PdfPage page2(pdf2->page(pair.right));
    if (!page2)
        return;

    QString key = QString("%1:%2:%3").arg(index).arg(zoomSpinBox->value())
            .arg(static_cast<int>(compareTextRadioButton->isChecked()));
    QString key1 = QString("1:%1:%2:%3").arg(key).arg(pair.left)
            .arg(filename1);
    QString key2 = QString("2:%1:%2:%3").arg(key).arg(pair.right)
            .arg(filename2);
    updateViews(pdf1, page1, pdf2, page2, pair.hasVisualDifference,
                key1, key2);
}


void MainWindow::updateViews(const PdfDocument &pdf1,
        const PdfPage &page1, const PdfDocument &pdf2,
        const PdfPage &page2, bool hasVisualDifference,
        const QString &key1, const QString &key2)
{
    QPixmap pixmap1;
    QPixmap pixmap2;
#if QT_VERSION >= 0x040600
    if (!QPixmapCache::find(key1, &pixmap1) ||
        !QPixmapCache::find(key2, &pixmap2)) {
#else
    if (!QPixmapCache::find(key1, pixmap1) ||
        !QPixmapCache::find(key2, pixmap2)) {
#endif
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        const int DPI = static_cast<int>(DPI_FACTOR *
                (zoomSpinBox->value() / 100.0));
        bool compareText = compareTextRadioButton->isChecked();
        QImage plainImage1;
        QImage plainImage2;
        if (hasVisualDifference || !compareText) {
            plainImage1 = page1->renderToImage(DPI, DPI);
            plainImage2 = page2->renderToImage(DPI, DPI);
        }
        pdf1->setRenderHint(Poppler::Document::Antialiasing);
        pdf1->setRenderHint(Poppler::Document::TextAntialiasing);
        pdf2->setRenderHint(Poppler::Document::Antialiasing);
        pdf2->setRenderHint(Poppler::Document::TextAntialiasing);
        QImage image1 = page1->renderToImage(DPI, DPI);
        QImage image2 = page2->renderToImage(DPI, DPI);

        QPainterPath highlighted1;
        QPainterPath highlighted2;
        if (hasVisualDifference || !compareText)
            computeVisualHighlights(&highlighted1, &highlighted2,
                    plainImage1, plainImage2);
        else
            computeTextHighlights(&highlighted1, &highlighted2, page1,
                    page2, DPI);
        if (!highlighted1.isEmpty())
            paintOnImage(highlighted1, &image1);
        if (!highlighted2.isEmpty())
            paintOnImage(highlighted2, &image2);
        if (highlighted1.isEmpty() && highlighted2.isEmpty()) {
            QFont font("Helvetica", 14);
            font.setOverline(true);
            font.setUnderline(true);
            highlighted1.addText(DPI / 4, DPI / 4, font,
                                tr("DiffPDF: False Positive"));
            paintOnImage(highlighted1, &image1);
        }
        pixmap1 = QPixmap::fromImage(image1);
        pixmap2 = QPixmap::fromImage(image2);
        QPixmapCache::insert(key1, pixmap1);
        QPixmapCache::insert(key2, pixmap2);
        QApplication::restoreOverrideCursor();
    }
    page1Label->setPixmap(pixmap1);
    page2Label->setPixmap(pixmap2);
}


void MainWindow::computeTextHighlights(QPainterPath *highlighted1,
        QPainterPath *highlighted2, const PdfPage &page1,
        const PdfPage &page2, const int DPI)
{
    QRectF rect1;
    QRectF rect2;
    QSettings settings;
    const int OVERLAP = settings.value("Overlap", 5).toInt();
    const bool COMBINE = settings.value("CombineTextHighlighting", true)
            .toBool();
    TextBoxList list1 = getTextBoxes(page1);
    TextBoxList list2 = getTextBoxes(page2);
    QStringList words1;
    QStringList words2;
    foreach (const PdfTextBox &box, list1)
        words1 << box->text();
    foreach (const PdfTextBox &box, list2)
        words2 << box->text();
    SequenceMatcher matcher(words1, words2);
    RangesPair rangesPair = computeRanges(&matcher);
    rangesPair = invertRanges(rangesPair.first, words1.count(),
                              rangesPair.second, words2.count());
    foreach (int index, rangesPair.first)
        addHighlighting(&rect1, highlighted1, list1[index], OVERLAP,
                        DPI, COMBINE);
    if (!rect1.isNull() && !rangesPair.first.isEmpty())
        highlighted1->addRect(rect1);
    foreach (int index, rangesPair.second)
        addHighlighting(&rect2, highlighted2, list2[index], OVERLAP,
                        DPI, COMBINE);
    if (!rect2.isNull() && !rangesPair.second.isEmpty())
        highlighted2->addRect(rect2);
}


void MainWindow::addHighlighting(QRectF *bigRect,
        QPainterPath *highlighted, const PdfTextBox &box,
        const int OVERLAP, const int DPI, const bool COMBINE)
{
    QRectF rect = box->boundingBox();
    scaleRect(DPI, &rect);
    if (COMBINE && rect.adjusted(-OVERLAP, -OVERLAP, OVERLAP, OVERLAP)
        .intersects(*bigRect))
        *bigRect = bigRect->united(rect);
    else {
        highlighted->addRect(*bigRect);
        *bigRect = rect;
    }
}


void MainWindow::computeVisualHighlights(QPainterPath *highlighted1,
        QPainterPath *highlighted2, const QImage &plainImage1,
        const QImage &plainImage2)
{
    QSettings settings;
    const int SQUARE_SIZE = settings.value("SquareSize", 10).toInt();
    QRect target;
    for (int x = 0; x < plainImage1.width(); x += SQUARE_SIZE) {
        for (int y = 0; y < plainImage1.height(); y += SQUARE_SIZE) {
            QRect rect(x, y, SQUARE_SIZE, SQUARE_SIZE);
            QImage temp1 = plainImage1.copy(rect);
            QImage temp2 = plainImage2.copy(rect);
            if (temp1 != temp2) {
                if (rect.adjusted(-1, -1, 1, 1).intersects(target))
                    target = target.united(rect);
                else {
                    highlighted1->addRect(target);
                    highlighted2->addRect(target);
                    target = rect;
                }
            }
        }
    }
    if (!target.isNull()) {
        highlighted1->addRect(target);
        highlighted2->addRect(target);
    }
}


void MainWindow::paintOnImage(const QPainterPath &path, QImage *image)
{
    QPen pen_(pen);
    QBrush brush_(brush);
    QColor color = pen.color();
    color.setAlpha(32); // semi-transparent for painting (stored as solid)
    pen_.setColor(color);
    brush_.setColor(color);

    QPainter painter(image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(pen_);
    painter.setBrush(brush_);

    QSettings settings;
    const int SQUARE_SIZE = settings.value("SquareSize", 10).toInt();
    const double RULE_WIDTH = settings.value("RuleWidth", 1.5).toDouble();
    QRectF rect = path.boundingRect();
    if (rect.width() < SQUARE_SIZE && rect.height() < SQUARE_SIZE) {
        rect.setHeight(SQUARE_SIZE);
        rect.setWidth(SQUARE_SIZE);
        painter.drawRect(rect);
        if (!qFuzzyCompare(RULE_WIDTH, 0.0)) {
            painter.setPen(QPen(pen.color()));
            painter.drawRect(0, rect.y(), RULE_WIDTH, rect.height());
        }
    }
    else {
        QPainterPath path_(path);
        path_.setFillRule(Qt::WindingFill);
        painter.drawPath(path_);
        if (!qFuzzyCompare(RULE_WIDTH, 0.0)) {
            painter.setPen(QPen(pen.color()));
            QList<QPolygonF> polygons = path_.toFillPolygons();
            foreach (const QPolygonF &polygon, polygons) {
                const QRectF rect = polygon.boundingRect();
                painter.drawRect(0, rect.y(), RULE_WIDTH, rect.height());
            }
        }
    }
    painter.end();
}


void MainWindow::closeEvent(QCloseEvent*)
{
    QSettings settings;
    settings.setValue("MainWindow/Geometry", saveGeometry());
    settings.setValue("MainWindow/State", saveState());
    settings.setValue("MainWindow/ControlDockArea",
                      static_cast<int>(controlDockArea));
    settings.setValue("MainWindow/ActionDockArea",
                      static_cast<int>(actionDockArea));
    settings.setValue("MainWindow/ViewSplitter", splitter->saveState());
    settings.setValue("ShowToolTips", showToolTips);
    settings.setValue("CombineTextHighlighting", combineTextHighlighting);
    settings.setValue("Zoom", zoomSpinBox->value());
    settings.setValue("Outline", pen);
    settings.setValue("Fill", brush);
    QMainWindow::close();
}


bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::ToolTip && !showToolTips)
        return true;
    return QMainWindow::eventFilter(object, event);
}


void MainWindow::setFile1(QString filename)
{
    if (filename.isEmpty())
        filename = QFileDialog::getOpenFileName(this,
                tr("DiffPDF - Choose File #1"), currentPath,
                tr("PDF files (*.pdf)"));
    if (!filename.isEmpty()) {
        if (filename == filename2LineEdit->text()) {
            QMessageBox::warning(this, tr("DiffPDF - Error"),
                    tr("Cannot compare a file to itself."));
            return;
        }
        filename1LineEdit->setText(filename);
        updateUi();
        int page_count = writeFileInfo(filename);
        pages1LineEdit->setText(tr("1-%1").arg(page_count));
        currentPath = QFileInfo(filename).canonicalPath();
        setFile2Button->setFocus();
        if (filename2LineEdit->text().isEmpty())
            statusLabel->setText(tr("Choose second file"));
        else
            statusLabel->setText(tr("Ready to compare"));
    }
}


void MainWindow::setFile2(QString filename)
{
    if (filename.isEmpty())
        filename = QFileDialog::getOpenFileName(this,
                tr("DiffPDF - Choose File #2"), currentPath,
                tr("PDF files (*.pdf)"));
    if (!filename.isEmpty()) {
        if (filename == filename1LineEdit->text()) {
            QMessageBox::warning(this, tr("DiffPDF - Error"),
                    tr("Cannot compare a file to itself."));
            return;
        }
        filename2LineEdit->setText(filename);
        updateUi();
        int page_count = writeFileInfo(filename);
        pages2LineEdit->setText(tr("1-%1").arg(page_count));
        currentPath = QFileInfo(filename).canonicalPath();
        compareButton->setFocus();
        if (filename1LineEdit->text().isEmpty())
            statusLabel->setText(tr("Choose first file"));
        else
            statusLabel->setText(tr("Ready to compare"));
    }
}


PdfDocument MainWindow::getPdf(const QString &filename)
{
    PdfDocument pdf(Poppler::Document::load(filename));
    if (!pdf)
        QMessageBox::warning(this, tr("DiffPDF - Error"),
                tr("Cannot load '%1'.").arg(filename));
    else if (pdf->isLocked()) {
        QMessageBox::warning(this, tr("DiffPDF - Error"),
                tr("Cannot read a locked PDF ('%1').").arg(filename));
#if QT_VERSION >= 0x040600
        pdf.clear();
#else
        pdf.reset();
#endif
    }
    return pdf;
}


int MainWindow::writeFileInfo(const QString &filename)
{
    int page_count = 0;
    PdfDocument pdf = getPdf(filename);
    if (!pdf)
        return page_count;
    writeLine(tr("<b>%1</b>").arg(filename));
    foreach (const QString &key, pdf->infoKeys()) {
        if (key == "CreationDate" || key == "ModDate")
            continue;
        writeLine(tr("%1: %2.").arg(key).arg(pdf->info(key)));
    }
    QDateTime created = pdf->date("CreationDate");
    QDateTime modified = pdf->date("ModDate");
    if (created != modified)
        writeLine(tr("Created: %1, last modified %2.")
                  .arg(created.toString())
                  .arg(modified.toString()));
    else
        writeLine(tr("Created: %1.").arg(created.toString()));
    page_count = pdf->numPages();
    writeLine(tr("Page count: %1.").arg(page_count));
    if (page_count > 0) {
        const double PointToMM = 0.3527777777;
        PdfPage page1(pdf->page(0));
        QSize size = page1->pageSize();
        writeLine(tr("Page size: %1pt x %2pt (%3mm x %4mm).")
                  .arg(size.width()).arg(size.height())
                  .arg(qRound(size.width() * PointToMM))
                  .arg(qRound(size.height() * PointToMM)));
    }
    return page_count;
}


void MainWindow::writeLine(const QString &text)
{
    logEdit->appendHtml(text);
    logEdit->ensureCursorVisible();
}


void MainWindow::writeError(const QString &text)
{
    logEdit->appendHtml(tr("<font color=red>%1</font>").arg(text));
    logEdit->ensureCursorVisible();
}


QList<int> MainWindow::getPageList(int which, PdfDocument pdf)
{
    // Poppler has 0-based page numbers; the UI has 1-based page numbers
    QLineEdit *pagesEdit = (which == 1 ? pages1LineEdit : pages2LineEdit);
    bool error = false;
    QList<int> pages;
    QString page_string = pagesEdit->text();
    page_string = page_string.replace(QRegExp("\\s+"), "");
    QStringList page_list = page_string.split(",");
    bool ok;
    foreach (const QString &page, page_list) {
        int hyphen = page.indexOf("-");
        if (hyphen > -1) {
            int p1 = page.left(hyphen).toInt(&ok);
            if (!ok || p1 < 1) {
                error = true;
                break;
            }
            int p2 = page.mid(hyphen + 1).toInt(&ok);
            if (!ok || p2 < 1 || p2 <= p1) {
                error = true;
                break;
            }
            for (int p = p1; p <= p2; ++p) {
                if (p > pdf->numPages())
                    break;
                pages.append(p - 1);
            }
        }
        else {
            int p = page.toInt(&ok);
            if (ok && p > 0 && p <= pdf->numPages())
                pages.append(p - 1);
            else {
                error = true;
                break;
            }
        }
    }
    if (error) {
        pages.clear();
        writeError(tr("Failed to understand page range '%1'.")
                   .arg(pagesEdit->text()));
        pagesEdit->setText(tr("1-%1").arg(pdf->numPages()));
        for (int page = 0; page < pdf->numPages(); ++page)
            pages.append(page);
    }
    return pages;
}


void MainWindow::compare()
{
    if (compareButton->text() == tr("&Cancel")) {
        cancel = true;
        compareButton->setText(tr("&Compare"));
        compareButton->setEnabled(true);
        return;
    }
    cancel = false;
    QString filename1 = filename1LineEdit->text();
    PdfDocument pdf1 = getPdf(filename1);
    if (!pdf1)
        return;
    QString filename2 = filename2LineEdit->text();
    PdfDocument pdf2 = getPdf(filename2);
    if (!pdf2) {
        return;
    }

    comparePrepareUi();
    QTime time;
    time.start();
    QPair<int, int> pair = comparePages(filename1, pdf1, filename2, pdf2);
    compareUpdateUi(pair, time.elapsed());
}


void MainWindow::comparePrepareUi()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    compareButton->setText(tr("&Cancel"));
    compareButton->setEnabled(true);
    compareButton->setFocus();
    viewDiffComboBox->clear();
    viewDiffComboBox->addItem(tr("(Not viewing)"));
    statusLabel->setText(tr("Ready"));
}


QPair<int, int> MainWindow::comparePages(const QString &filename1,
        const PdfDocument &pdf1, const QString &filename2,
        const PdfDocument &pdf2)
{
    QList<int> pages1 = getPageList(1, pdf1);
    QList<int> pages2 = getPageList(2, pdf2);
    int total = qMin(pages1.count(), pages2.count());
    int number = 0;
    int index = 0;
    while (!pages1.isEmpty() && !pages2.isEmpty()) {
        int p1 = pages1.takeFirst();
        PdfPage page1(pdf1->page(p1));
        if (!page1) {
            writeError(tr("Failed to read page %1 from '%2'.")
                          .arg(p1 + 1).arg(filename1));
            continue;
        }
        int p2 = pages2.takeFirst();
        PdfPage page2(pdf2->page(p2));
        if (!page2) {
            writeError(tr("Failed to read page %1 from '%2'.")
                          .arg(p2 + 1).arg(filename2));
            continue;
        }
        writeLine(tr("Comparing: %1 vs. %2.").arg(p1 + 1).arg(p2 + 1));
        QApplication::processEvents();
        if (cancel) {
            writeError(tr("Cancelled."));
            break;
        }
        Difference difference = getTheDifference(page1, page2);
        if (difference != NoDifference) {
            QVariant v;
            v.setValue(PagePair(p1, p2, difference == VisualDifference));
            viewDiffComboBox->addItem(tr("%1 vs. %2 %3 %4")
                    .arg(p1 + 1).arg(p2 + 1).arg(QChar(0x2022))
                    .arg(++index), v);
        }
        statusLabel->setText(tr("Comparing %1/%2").arg(++number)
                                                  .arg(total));
    }
    return qMakePair(number, total);
}


void MainWindow::compareUpdateUi(const QPair<int, int> &pair,
        const int millisec)
{
    int differ = viewDiffComboBox->count() - 1;
    if (!cancel) {
        if (millisec > 1000)
            writeLine(QString("Completed in %1 seconds.")
            .arg(millisec / 1000.0, 0, 'f', 2));
        if (viewDiffComboBox->count() > 1) {
            if (viewDiffComboBox->count() == 2)
                writeLine(tr("<font color=brown>Files differ on 1 page "
                            "(%1 page%2 compared).</font>")
                        .arg(pair.first)
                        .arg(pair.first == 1 ? tr(" was") : tr("s were")));
            else
                writeLine(tr("<font color=brown>Files differ on %1 pages "
                            "(%2 page%3 compared).</font>")
                            .arg(differ).arg(pair.first)
                            .arg(pair.first == 1 ? tr(" was")
                                                 : tr("s were")));
            viewDiffComboBox->setFocus();
            viewDiffComboBox->setCurrentIndex(1);
        }
        else {
            writeLine(tr("The PDFs appear to be the same."));
            const QString message("<p style='font-size: xx-large;"
                    "color: darkgreen'>"
                    "DiffPDF: The PDFs appear to be the same.</p>");
            page1Label->setText(message);
            page2Label->setText(message);
        }
    }

    compareButton->setText(tr("&Compare"));
    statusLabel->setText(tr("%1 differ%2 %3/%4 compared").arg(differ)
            .arg(differ == 1 ? "s" : "").arg(pair.first).arg(pair.second));
    updateUi();
    if (!cancel)
        viewDiffComboBox->setFocus();
    QApplication::restoreOverrideCursor();
}


MainWindow::Difference MainWindow::getTheDifference(PdfPage page1,
                                                    PdfPage page2)
{
    TextBoxList list1 = getTextBoxes(page1);
    TextBoxList list2 = getTextBoxes(page2);
    if (list1.count() != list2.count())
        return TextualDifference;
    for (int i = 0; i < list1.count(); ++i)
        if (list1[i]->text() != list2[i]->text())
            return TextualDifference;

    if (compareAppearanceRadioButton->isChecked()) {
        QImage image1 = page1->renderToImage();
        QImage image2 = page2->renderToImage();
        if (image1 != image2)
            return VisualDifference;
    }
    return NoDifference;
}


void MainWindow::options()
{
    QSettings settings;
    qreal ruleWidth = settings.value("RuleWidth", 1.5).toDouble();
    int cacheSize = QPixmapCache::cacheLimit() / 1000;
    OptionsForm form(&pen, &brush, &ruleWidth, &showToolTips,
                     &combineTextHighlighting, &cacheSize, this);
    if (form.exec()) {
        settings.setValue("RuleWidth", ruleWidth);
        settings.setValue("CombineTextHighlighting",
                          combineTextHighlighting);
        settings.setValue("CacheSizeMB", cacheSize);
        QPixmapCache::clear();
        QPixmapCache::setCacheLimit(1000 * cacheSize);
        updateViews();
    }
}


void MainWindow::help()
{
    QMessageBox::information(this, tr("DiffPDF - Help"),
    tr("<p>To learn how to use DiffPDF click the <b>File #1</b> button "
    "to choose one PDF file and then the <b>File #2</b> button to choose "
    "another (ideally very similar) PDF file, then click the "
    "<b>Compare</b> button to perform the comparison, and when "
    "that's finished, navigate through the pairs of differing pages "
    "using the <b>View</b> combobox. "
    "<p>The Controls, Actions, and Log are in dock widgets&mdash;these "
    "can be dragged into other dock areas (in which case they will "
    "reshape themselves as necessary), or dragged to float free. "
    "The Log can also be closed; right click a dock area and check "
    "the Log checkbox to open it again. Hover the mouse for tooltips."
    "<p>Although a GUI program, if run from a console with two PDF "
    "files listed on the command line, DiffPDF will start up and "
    "immediately compare them in Text mode. "));
}


void MainWindow::about()
{
    static const QString version("1.2.2");

    QMessageBox::about(this, tr("DiffPDF - About"),
    tr("<p><b>DiffPDF</a> %1</b> by Mark Summerfield."
    "<p>Copyright &copy; 2008-11 "
    "<a href=\"http://www.qtrac.eu\">Qtrac</a> Ltd. All rights reserved."
    "<p>This program compares the text or the visual appearance of "
    "each page in two PDF files. It was inspired by an idea "
    "from Jasmin Blanchette, and incorporates many of his suggestions. "
    "Thanks also to Steven Lee for building Windows versions, "
    "Dirk Loss for building Mac OS X versions, and to David Paleino."
    "<p>To learn how to use the program click the <b>Help</b> button "
    "or press <b>F1</b>."
    "<p>If you like DiffPDF and you develop software you might like "
    "my books:<ul>"
    "<li><a href=\"http://www.qtrac.eu/aqpbook.html\">"
    "Advanced Qt Programming</a></li>"
    "<li><a href=\"http://www.qtrac.eu/pyqtbook.html\">"
    "Rapid GUI Programming with Python and Qt</a></li>"
    "<li><a href=\"http://www.qtrac.eu/py3book.html\">"
    "Programming in Python 3</a></li>"
    "<li><a href=\"http://www.qtrac.eu/gobook.html\">"
    "Programming in Go</a></li>"
    "</ul>"
    "<hr><p>This program is free software: you can redistribute it "
    "and/or modify it under the terms of the GNU General Public License "
    "as published by the Free Software Foundation, either version 2 of "
    "the License, or (at your option), any "
    "later version. This program is distributed in the hope that it will "
    "be useful, but WITHOUT ANY WARRANTY; without even the implied "
    "warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. "
    "See the GNU General Public License (in file <tt>gpl-2.0.txt</tt>) "
    "for more details.").arg(version));
}
