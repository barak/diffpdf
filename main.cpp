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

#include "mainwindow.hpp"
#include <QApplication>
#include <QIcon>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
#ifdef Q_WS_MAC
    app.setCursorFlashTime(0);
#endif
    app.setOrganizationName("Qtrac Ltd.");
    app.setOrganizationDomain("qtrac.eu");
    app.setApplicationName("DiffPDF");
    app.setWindowIcon(QIcon(":/icon.png"));

    QString filename1;
    QString filename2;
    if (argc > 1) {
        filename1 = argv[1];
        if (!filename1.toLower().endsWith(".pdf"))
            filename1.clear();
        if (argc > 2) {
            filename2 = argv[2];
            if (!filename2.toLower().endsWith(".pdf"))
                filename2.clear();
            else if (filename1.isEmpty()) {
                filename1 = filename2;
                filename2.clear();
            }
        }
    }
    MainWindow window(filename1, filename2);
    window.show();
    return app.exec();
}

