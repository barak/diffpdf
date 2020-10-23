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
#include "optionsform.hpp"
#include "mainwindow.hpp"
#include <QtGui>


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
    : QMainWindow(parent), current_path("."), cancel(false)
{
    QSettings settings;

    QPushButton *setFile1Button = new QPushButton(tr("File #&1..."));
    setFile1Button->setToolTip(tr("<p>Choose the first (left hand) file "
                "to be compared."));
    file1Label = new QLabel();
    file1Label->setToolTip(tr("The first (left hand) file."));
    file1Label->setMinimumWidth(100);
    file1Label->setFrameStyle(QFrame::StyledPanel|QFrame::Sunken);
    file1Label->setText(filename1);
    setFile2Button = new QPushButton(tr("File #&2..."));
    setFile2Button->setToolTip(tr("<p>Choose the second (right hand) file "
                "to be compared."));
    file2Label = new QLabel();
    file2Label->setToolTip(tr("The second (right hand) file."));
    file2Label->setMinimumWidth(100);
    file2Label->setFrameStyle(QFrame::StyledPanel|QFrame::Sunken);
    file2Label->setText(filename2);
    QLabel *comparePages1Label = new QLabel(tr("&Pages:"));
    pages1LineEdit = new QLineEdit();
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
    QLabel *comparePages2Label = new QLabel(tr("Pag&es:"));
    pages2LineEdit = new QLineEdit();
    comparePages2Label->setBuddy(pages2LineEdit);
    pages2LineEdit->setToolTip(pages1LineEdit->toolTip());
    compareAppearanceCheckBox = new QCheckBox(tr("Compare &Appearance"));
    compareAppearanceCheckBox->setChecked(
            settings.value("MainWindow/Appearance").toBool());
    compareAppearanceCheckBox->setToolTip(tr("<p>If this checkbox is "
                "<i>checked</i> then each page's text is compared "
                "and if no differences are detected then the pages' "
                "appearance's are compared. If this checkbox is "
                "<i>unchecked</i> only the text is compared. "
                "Note that using this option can be slow for large "
                "documents."));
    compareButton = new QPushButton(tr("&Compare"));
    compareButton->setEnabled(false);
    compareButton->setToolTip(tr("<p>Click to compare (or re-compare) "
                "the documents."));
    QLabel *viewDiffLabel = new QLabel(tr("&View Difference:"));
    viewDiffLabel->setToolTip(tr("<p>Shows each pair of pages which "
                "are different. The comparison is textual unless the "
                "<b>Compare Appearance</b> checkbox is checked, in "
                "which case the comparison is done visually. "
                "Visual differences can occur if a paragraph is "
                "formated differently or if an embedded diagram or "
                "image has changed."));
    viewDiffComboBox = new QComboBox();
    viewDiffComboBox->addItem(tr("(Not viewing)"));
    viewDiffLabel->setBuddy(viewDiffComboBox);
    viewDiffComboBox->setToolTip(viewDiffLabel->toolTip());
    QLabel *zoomLabel = new QLabel(tr("&Zoom:"));
    zoomLabel->setToolTip(tr("<p>Determines the scale at which the "
                "pages are shown."));
    zoomSpinBox = new QSpinBox();
    zoomLabel->setBuddy(zoomSpinBox);
    zoomSpinBox->setRange(25, 400);
    zoomSpinBox->setSuffix(tr(" %"));
    zoomSpinBox->setSingleStep(25);
    zoomSpinBox->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    zoomSpinBox->setValue(settings.value("Zoom", 100).toInt());
    zoomSpinBox->setToolTip(zoomLabel->toolTip());
    QPushButton *optionsButton = new QPushButton(tr("&Options..."));
    optionsButton->setToolTip(tr("Click to customize the application."));
    QPushButton *aboutButton = new QPushButton(tr("A&bout"));
    aboutButton->setToolTip(tr("Click for copyright and credits."));
    QPushButton *quitButton = new QPushButton(tr("&Quit"));
    quitButton->setToolTip(tr("Click to terminate the application."));
    page1Label = new QLabel();
    page1Label->setAlignment(Qt::AlignCenter);
    page1Label->setToolTip(tr("<p>Shows the first (left hand) document's "
                "page that correponds to the page shown in the "
                "View Difference combobox."));
    page2Label = new QLabel();
    page2Label->setAlignment(Qt::AlignCenter);
    page2Label->setToolTip(tr("<p>Shows the second (right hand) "
                "document's page that correponds to the page shown in "
                "the View Difference combobox."));
    resultsBrowser = new QTextBrowser();
    resultsBrowser->setToolTip(tr("<p>Shows a log of information about "
                "the documents and the progress of comparisons."));

    QGridLayout *topLayout = new QGridLayout();
    topLayout->addWidget(setFile1Button, 0, 0);
    topLayout->addWidget(file1Label, 0, 1);
    topLayout->addWidget(comparePages1Label, 0, 2);
    topLayout->addWidget(pages1LineEdit, 0, 3);
    topLayout->addWidget(setFile2Button, 1, 0);
    topLayout->addWidget(file2Label, 1, 1);
    topLayout->addWidget(comparePages2Label, 1, 2);
    topLayout->addWidget(pages2LineEdit, 1, 3);
    topLayout->addWidget(compareAppearanceCheckBox, 0, 4);
    topLayout->addWidget(viewDiffLabel, 1, 4);
    topLayout->addWidget(viewDiffComboBox, 1, 5, 1, 2);
    topLayout->addWidget(zoomLabel, 0, 5);
    topLayout->addWidget(zoomSpinBox, 0, 6);
    topLayout->addWidget(compareButton, 0, 7);
    topLayout->addWidget(optionsButton, 0, 8);
    topLayout->addWidget(aboutButton, 1, 7);
    topLayout->addWidget(quitButton, 1, 8);
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(topLayout);
    topSplitter = new QSplitter(Qt::Horizontal);
    QScrollArea *area1 = new QScrollArea();
    area1->setWidget(page1Label);
    area1->setWidgetResizable(true);
    topSplitter->addWidget(area1);
    QScrollArea *area2 = new QScrollArea();
    area2->setWidget(page2Label);
    area2->setWidgetResizable(true);
    topSplitter->addWidget(area2);
    topSplitter->restoreState(settings.value("MainWindow/TopSplitter")
                              .toByteArray());
    bottomSplitter = new QSplitter(Qt::Vertical);
    bottomSplitter->addWidget(topSplitter);
    bottomSplitter->addWidget(resultsBrowser);
    bottomSplitter->restoreState(settings.value(
                "MainWindow/BottomSplitter").toByteArray());
    layout->addWidget(bottomSplitter);
    QWidget *widget = new QWidget();
    widget->setLayout(layout);
    setCentralWidget(widget);

    connect(area1->verticalScrollBar(), SIGNAL(valueChanged(int)),
            area2->verticalScrollBar(), SLOT(setValue(int)));
    connect(area2->verticalScrollBar(), SIGNAL(valueChanged(int)),
            area1->verticalScrollBar(), SLOT(setValue(int)));
    connect(area1->horizontalScrollBar(), SIGNAL(valueChanged(int)),
            area2->horizontalScrollBar(), SLOT(setValue(int)));
    connect(area2->horizontalScrollBar(), SIGNAL(valueChanged(int)),
            area1->horizontalScrollBar(), SLOT(setValue(int)));
    connect(viewDiffComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateViews(int)));
    connect(setFile1Button, SIGNAL(clicked()), this, SLOT(setFile1()));
    connect(setFile2Button, SIGNAL(clicked()), this, SLOT(setFile2()));
    connect(compareButton, SIGNAL(clicked()), this, SLOT(compare()));
    connect(zoomSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(updateViews()));
    connect(optionsButton, SIGNAL(clicked()), this, SLOT(options()));
    connect(aboutButton, SIGNAL(clicked()), this, SLOT(about()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));

    restoreGeometry(settings.value("MainWindow/Geometry").toByteArray());
    setWindowTitle(tr("DiffPDF"));
    setWindowIcon(QIcon(":/icon.png"));
    if (!filename1.isEmpty()) {
        setFile1(filename1);
        setFile2Button->setFocus();
        if (!filename2.isEmpty()) {
            setFile2(filename2);
            QTimer::singleShot(0, this, SLOT(compare()));
        }
    }
    else
        updateUi();
}


void MainWindow::updateUi()
{
    compareButton->setEnabled(!file1Label->text().isEmpty() &&
                              !file2Label->text().isEmpty());
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
    PagePair pp = viewDiffComboBox->itemData(index).value<PagePair>();
    if (pp.isNull())
        return;

    QString filename1 = file1Label->text();
    PdfDocument pdf1 = getPdf(filename1);
    if (!pdf1)
        return;
    PdfPage page1(pdf1->page(pp.left));
    if (!page1)
        return;

    QString filename2 = file2Label->text();
    PdfDocument pdf2 = getPdf(filename2);
    if (!pdf2)
        return;
    PdfPage page2(pdf2->page(pp.right));
    if (!page2)
        return;

    int dpi = static_cast<int>(DPI_FACTOR *
                               (zoomSpinBox->value() / 100.0));
    QImage plainImage1;
    QImage plainImage2;
    if (pp.visual_difference || compareAppearanceCheckBox->isChecked()) {
        plainImage1 = page1->renderToImage(dpi, dpi);
        plainImage2 = page2->renderToImage(dpi, dpi);
    }
    pdf1->setRenderHint(Poppler::Document::Antialiasing);
    pdf1->setRenderHint(Poppler::Document::TextAntialiasing);
    pdf2->setRenderHint(Poppler::Document::Antialiasing);
    pdf2->setRenderHint(Poppler::Document::TextAntialiasing);
    QImage image1 = page1->renderToImage(dpi, dpi);
    QImage image2 = page2->renderToImage(dpi, dpi);

    QRegion highlighted1;
    QRegion highlighted2;
    // Always show textual differences
    QRectF rect1;
    QRectF rect2;
    QSettings settings;
    const int OVERLAP = settings.value("Overlap", 5).toInt();
    TextBoxList list1 = getTextBoxes(page1);
    TextBoxList list2 = getTextBoxes(page2);
    while (!list1.isEmpty() && !list2.isEmpty()) {
        PdfTextBox box1 = list1.takeFirst();
        PdfTextBox box2 = list2.takeFirst();
        if (box1->text().simplified() != box2->text().simplified()) {
            QRectF rect = box1->boundingBox();
            scaleRect(dpi, &rect);
            if (rect.adjusted(-OVERLAP, -OVERLAP, OVERLAP, OVERLAP)
                .intersects(rect1))
                rect1 = rect1.united(rect);
            else {
                highlighted1 = highlighted1.united(rect1.toRect());
                rect1 = rect;
            }
            rect = box2->boundingBox();
            scaleRect(dpi, &rect);
            if (rect.adjusted(-OVERLAP, -OVERLAP, OVERLAP, OVERLAP)
                .intersects(rect2))
                rect2 = rect2.united(rect);
            else {
                highlighted2 = highlighted2.united(rect2.toRect());
                rect2 = rect;
            }
        }
        if (!rect1.isNull()) {
            highlighted1 = highlighted1.united(rect1.toRect());
        }
        if (!rect2.isNull()) {
            highlighted2 = highlighted2.united(rect2.toRect());
        }
    }
    const int SQUARE_SIZE = settings.value("SquareSize", 10).toInt();
    if (pp.visual_difference || compareAppearanceCheckBox->isChecked()) {
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
                        highlighted1 = highlighted1.united(target);
                        highlighted2 = highlighted2.united(target);
                        target = rect;
                    }
                }
            }
        }
        if (!target.isNull()) {
            highlighted1 = highlighted1.united(target);
            highlighted2 = highlighted2.united(target);
        }
    }
    paintRectsOnImage(highlighted1, &image1);
    paintRectsOnImage(highlighted2, &image2);
    page1Label->setPixmap(QPixmap::fromImage(image1));
    page2Label->setPixmap(QPixmap::fromImage(image2));
}


void MainWindow::paintRectsOnImage(const QRegion &region, QImage *image)
{
    QSettings settings;
    QColor color = settings.value("HighlightColor", Qt::yellow)
                                  .value<QColor>();
    QPen pen(Qt::PenStyle(settings.value("LineStyle", Qt::SolidLine)
                                         .toInt()));
    pen.setColor(color);
    color.setAlpha(32); // semi-transparent version of the color
    QBrush brush(color);
    QPainter painter(image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(pen);
    painter.setBrush(brush);
    foreach (QRect rect, region.rects())
        painter.drawRect(minimumSizedRect(rect));
    painter.end();
}


QRect MainWindow::minimumSizedRect(const QRect &rect)
{
    const int MINIMUM_HEIGHT = 4;
    const int MINIMUM_WIDTH = 8;

    QRect r(rect);
    if (r.width() < MINIMUM_WIDTH) {
        r.setWidth(MINIMUM_WIDTH);
        r.moveLeft(MINIMUM_WIDTH / 2);
    }
    if (r.height() < MINIMUM_HEIGHT) {
        r.setHeight(MINIMUM_HEIGHT);
        r.setY(r.y() - (MINIMUM_HEIGHT / 2));
    }
    return r;
}


void MainWindow::closeEvent(QCloseEvent *)
{
    QSettings settings;
    settings.setValue("MainWindow/Geometry", saveGeometry());
    settings.setValue("MainWindow/Appearance",
                      compareAppearanceCheckBox->isChecked());
    settings.setValue("MainWindow/TopSplitter",
                      topSplitter->saveState());
    settings.setValue("MainWindow/BottomSplitter",
                      bottomSplitter->saveState());
    settings.setValue("Zoom", zoomSpinBox->value());
    QMainWindow::close();
}


void MainWindow::setFile1(QString filename)
{
    if (filename.isEmpty())
        filename = QFileDialog::getOpenFileName(this,
                tr("DiffPDF - Choose File #1"), current_path,
                tr("PDF files (*.pdf)"));
    if (!filename.isEmpty()) {
        if (filename == file2Label->text()) {
            QMessageBox::warning(this, tr("DiffPDF - Error"),
                    tr("Cannot compare a file to itself."));
            return;
        }
        file1Label->setText(filename);
        updateUi();
        int page_count = writeFileInfo(filename);
        pages1LineEdit->setText(tr("1-%1").arg(page_count));
        current_path = QFileInfo(filename).canonicalPath();
        setFile2Button->setFocus();
    }
}


void MainWindow::setFile2(QString filename)
{
    if (filename.isEmpty())
        filename = QFileDialog::getOpenFileName(this,
                tr("DiffPDF - Choose File #2"), current_path,
                tr("PDF files (*.pdf)"));
    if (!filename.isEmpty()) {
        if (filename == file1Label->text()) {
            QMessageBox::warning(this, tr("DiffPDF - Error"),
                    tr("Cannot compare a file to itself."));
            return;
        }
        file2Label->setText(filename);
        updateUi();
        int page_count = writeFileInfo(filename);
        pages2LineEdit->setText(tr("1-%1").arg(page_count));
        current_path = QFileInfo(filename).canonicalPath();
        compareButton->setFocus();
    }
}


PdfDocument MainWindow::getPdf(const QString &filename)
{
    PdfDocument pdf(Poppler::Document::load(filename));
    if (!pdf)
        writeError(tr("Failed to load '%1'").arg(filename));
    else if (pdf->isLocked()) {
        writeError(tr("Cannot read a locked PDF"));
        pdf.reset();
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
    return page_count;
}


void MainWindow::writeLine(const QString &text)
{
    resultsBrowser->append(text);
    resultsBrowser->ensureCursorVisible();
}


void MainWindow::writeError(const QString &text)
{
    resultsBrowser->append(tr("<font color=red>%1</font>").arg(text));
    resultsBrowser->ensureCursorVisible();
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
    QString filename1 = file1Label->text();
    PdfDocument pdf1 = getPdf(filename1);
    if (!pdf1)
        return;
    QString filename2 = file2Label->text();
    PdfDocument pdf2 = getPdf(filename2);
    if (!pdf2) {
        return;
    }
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    compareButton->setText(tr("&Cancel"));
    compareButton->setEnabled(true);
    compareButton->setFocus();
    viewDiffComboBox->clear();
    viewDiffComboBox->addItem(tr("(Not viewing)"));
    QList<int> pages1 = getPageList(1, pdf1);
    QList<int> pages2 = getPageList(2, pdf2);
    int minimum = qMin(pages1.count(), pages2.count());
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
        Difference diff = comparePages(page1, page2);
        if (diff != NoDifference) {
            QVariant v;
            v.setValue(PagePair(p1, p2, diff == VisualDifference));
            viewDiffComboBox->addItem(tr("%1 vs. %2").arg(p1 + 1)
                                                     .arg(p2 + 1), v);
        }
    }
    if (!cancel) {
        if (viewDiffComboBox->count() > 1) {
            if (viewDiffComboBox->count() == 2)
                writeLine(tr("<font color=brown>Files differ on 1 page "
                            "(%1 page%2 compared).</font>")
                        .arg(minimum)
                        .arg(minimum == 1 ? tr(" was") : tr("s were")));
            else
                writeLine(tr("<font color=brown>Files differ on %1 pages "
                            "(%2 page%3 compared).</font>")
                            .arg(viewDiffComboBox->count() - 1)
                            .arg(minimum)
                            .arg(minimum == 1 ? tr(" was")
                                              : tr("s were")));
            viewDiffComboBox->setFocus();
            viewDiffComboBox->setCurrentIndex(1);
        }
        else
            writeLine(tr("No differences detected."));
    }
    compareButton->setText(tr("&Compare"));
    updateUi();
    if (!cancel)
        viewDiffComboBox->setFocus();
    QApplication::restoreOverrideCursor();
}


MainWindow::Difference MainWindow::comparePages(PdfPage page1,
                                                PdfPage page2)
{
    TextBoxList list1 = getTextBoxes(page1);
    TextBoxList list2 = getTextBoxes(page2);
    while (!list1.isEmpty() && !list2.isEmpty()) {
        PdfTextBox box1 = list1.takeFirst();
        PdfTextBox box2 = list2.takeFirst();
        if (box1->text().simplified() != box2->text().simplified())
            return TextualDifference;
    }
    if (compareAppearanceCheckBox->isChecked()) {
        QImage image1 = page1->renderToImage();
        QImage image2 = page2->renderToImage();
        if (image1 != image2)
            return VisualDifference;
    }
    return NoDifference;
}


void MainWindow::options()
{
    OptionsForm form(this);
    form.exec();
}


void MainWindow::about()
{
    static const QString version("0.4.0");

    QMessageBox::about(this, tr("DiffPDF - About"),
    tr("<p><b>DiffPDF</a> %1</b> by Mark Summerfield."
    "<p>Copyright &copy; 2008-10 <a href=\"http://www.qtrac.eu\">Qtrac</a> "
    "Ltd. All rights reserved."
    "<p>This program compares the text (and optionally the appearance) "
    "of each page in two PDF files. It was inspired by an idea "
    "from Jasmin Blanchette, and incorporates many of his suggestions."
    "<p>To learn how to use the program click the <b>File #1</b> button "
    "to choose one PDF file and then the <b>File #2</b> button to choose "
    "another (ideally very similar) PDF file, then click the "
    "<b>Compare</b> button to see the results! "
    "You can also read the tooltips."
    "<p>Although a GUI program, if run from a console with two PDF "
    "files listed on the command line, DiffPDF will start up and "
    "immediately compare them. "
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

