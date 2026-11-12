/*
    This program or module is free software: you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 2 of
    the License, or (at your option) any later version. This program is
    distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
    for more details.
*/

// Tests that a closed dock window can always be reopened, driving MainWindow
// on Qt's offscreen platform:
//
//   docktest [DATA_DIRECTORY]
//
// The directory is accepted so that this runs like the other window tests,
// and ignored: no PDF is needed to open and close panels.
//
// What is being guarded.  The only way the program offers to reopen a closed
// dock window is the checkbox list from QMainWindow::createPopupMenu().  Qt
// puts that list up for a right click on a dock window, a toolbar or the gap
// between docks, and not for one on the central widget, so before
// MainWindow::contextMenuEvent() existed, closing the last docked window left
// the whole window covered by the central widget and the list out of reach.
// The panels could not be reopened at all, and because the layout is saved on
// exit, they stayed gone across restarts.
//
// So both tests here click where it used to fail, over the central widget,
// with nothing left in the dock areas.  The first empties them the way a user
// can in one session; the second starts a window on a saved layout in which
// every panel is hidden, which is the state a user is actually left in, since
// the layout is saved on exit.
//
// The settings go to an INI file in a temporary directory, so the user's own
// are left alone.

#include "../mainwindow.hpp"
#include <QAction>
#include <QApplication>
#include <QContextMenuEvent>
#include <QDockWidget>
#include <QMenu>
#include <QSettings>
#include <QTemporaryDir>
#include <QTextStream>
#include <QTimer>

static QTextStream out(stdout);
static int failures = 0;

static void check(bool ok, const QString &what)
{
    if (!ok) {
        out << "FAIL: " << what << "\n";
        ++failures;
    }
}

// Every dock window, by the object name createDockWidgets() gives it
static const char *DOCKS[] = {"Controls", "Actions", "Margins", "PageBox",
                              "Zoning", "Log"};

static QDockWidget *dock(MainWindow *window, const QString &name)
{
    return window->findChild<QDockWidget *>(name);
}

// Controls and Actions are created without DockWidgetClosable, and Qt 6
// refuses close() for a dock window that lacks it, so these two cannot be
// closed from the window at all.
static bool isClosable(QDockWidget *d)
{
    return d->features().testFlag(QDockWidget::DockWidgetClosable);
}

// Empty the dock areas the way a user can: close the four closable panels,
// and drag the two that will not close out of the window.  Nothing is left
// docked, so the central widget covers the window and there is no dock
// window, no toolbar and no gap between docks anywhere in it.
static void emptyTheDockAreas(MainWindow *window)
{
    for (const char *name : DOCKS) {
        QDockWidget *d = dock(window, name);
        if (!d)
            continue;
        if (isClosable(d))
            d->close();
        else
            d->setFloating(true);
    }
}

// Every panel hidden, which restoreState() can produce even though close()
// cannot: a layout saved by an older version, or by Qt 5, which did close
// these two.  Used to check what a restart does with such a layout.
static void hideEveryDock(MainWindow *window)
{
    for (const char *name : DOCKS)
        if (QDockWidget *d = dock(window, name))
            d->hide();
}

// The point the tests right click: the middle of the central widget, which
// with every dock hidden is the whole window.
static QPoint centralPoint(MainWindow *window)
{
    QWidget *central = window->centralWidget();
    return central->mapTo(window, central->rect().center());
}

// True if pos really does land on the central widget, so that a passing test
// cannot be one that happened to click a dock window instead.
static bool overCentralWidget(MainWindow *window, const QPoint &pos)
{
    for (QWidget *w = window->childAt(pos); w; w = w->parentWidget())
        if (w == window->centralWidget())
            return true;
    return false;
}

// The menu the window puts up for a right click at pos, or nullptr.
// createPopupMenu() parents it to the window, so it is a direct child.
static QMenu *popupAt(MainWindow *window, const QPoint &pos)
{
    QContextMenuEvent event(QContextMenuEvent::Mouse, pos,
                            window->mapToGlobal(pos));
    QApplication::sendEvent(window, &event);
    return window->findChild<QMenu *>(QString(),
                                      Qt::FindDirectChildrenOnly);
}

// Right click over the central widget and check the list that comes up
// offers every dock window, then tick Log back on.
static void checkEveryDockCanBeReopened(MainWindow *window, const QString &when)
{
    const QPoint pos = centralPoint(window);
    check(overCentralWidget(window, pos),
          QString("%1: the click lands on the central widget").arg(when));

    QMenu *menu = popupAt(window, pos);
    check(menu != nullptr,
          QString("%1: right clicking the central widget opens the list")
                  .arg(when));
    if (!menu)
        return;

    for (const char *name : DOCKS) {
        QDockWidget *d = dock(window, name);
        check(d && menu->actions().contains(d->toggleViewAction()),
              QString("%1: the list offers %2").arg(when).arg(name));
    }
    delete menu;

    QDockWidget *log = dock(window, "Log");
    if (!log)
        return;
    log->toggleViewAction()->trigger();
    check(log->isVisible(),
          QString("%1: Log comes back when its checkbox is ticked")
                  .arg(when));
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QTemporaryDir settingsDir;
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
                       settingsDir.path());
    app.setOrganizationName("disparPDF-docktest");
    app.setApplicationName("disparPDF");
    // Nothing should open a message box, but if one does, close it rather
    // than wait for ever
    QTimer closer;
    QObject::connect(&closer, &QTimer::timeout, [] {
        if (QWidget *modal = QApplication::activeModalWidget()) {
            out << "FAIL: a message box opened: " << modal->windowTitle()
                << "\n";
            ++failures;
            modal->close();
        }
    });
    closer.start(100);

    StartupParameters parameters;
    Status status;

    // Closing every panel in one session, and reopening them in it
    {
        MainWindow window(DebugOff, CompareAppearance, QString(), QString(),
                          "en", &parameters, &status);
        window.show();
        QApplication::processEvents();

        for (const char *name : DOCKS)
            check(dock(&window, name) != nullptr,
                  QString("%1 exists").arg(name));

        emptyTheDockAreas(&window);
        QApplication::processEvents();
        for (const char *name : DOCKS) {
            QDockWidget *d = dock(&window, name);
            check(d && (isClosable(d) ? !d->isVisible() : d->isFloating()),
                  QString("%1 leaves the dock area").arg(name));
        }

        checkEveryDockCanBeReopened(&window, "with the dock areas empty");

        // Leave every panel hidden, so the saved layout is the broken one
        hideEveryDock(&window);
        window.close();
    }

    // The same, in a new window that has restored that saved layout.  This
    // is the state the bug actually left behind: every panel gone, and gone
    // again at every start.
    {
        MainWindow window(DebugOff, CompareAppearance, QString(), QString(),
                          "en", &parameters, &status);
        window.show();
        QApplication::processEvents();

        QDockWidget *log = dock(&window, "Log");
        check(log && !log->isVisible(),
              "a closed panel is still closed after a restart");

        checkEveryDockCanBeReopened(&window, "after a restart");
        window.close();
    }

    if (!failures)
        out << "docktest: all tests passed\n";
    return failures ? 1 : 0;
}
