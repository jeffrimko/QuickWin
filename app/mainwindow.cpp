// TODO: help command

/*=============================================================*/
/* SECTION: Includes                                           */
/*=============================================================*/

#define _WIN32_WINNT 0x0600
#include <windows.h>
#include <QDesktopWidget>

#include <Qt>

#include <QKeyEvent>
#include <QMessageBox>
#include <QShortcut>
#include <QHash>
#include "mainwindow.h"
#include "ui_mainwindow.h"

/*=============================================================*/
/* SECTION: Local Data                                         */
/*=============================================================*/

/// Stores saved alias windows.
QHash<QString, HWND> gSavedWins;

/// The application version string.
QString gVerStr("0.1.0");

/*=============================================================*/
/* SECTION: Local Prototypes                                   */
/*=============================================================*/

static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam);
static BOOL IsAltTabWindow(HWND hwnd);

/*=============================================================*/
/* SECTION: Function Definitions                               */
/*=============================================================*/

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow),
        proxy(new QSortFilterProxyModel(parent)),
        model(new QStandardItemModel(0, 2, parent))
{
    // Set the window style.
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags | Qt::WindowStaysOnTopHint | Qt::ToolTip);

    // Center window.
    QDesktopWidget *desktop = QApplication::desktop();
    int width = desktop->width() * 0.6;
    int height = desktop->height() * 0.6;
    setFixedSize(width, height);
    move((desktop->width() - width) / 2, (desktop->height() - height) / 2);

    // Set up system tray.
    trayIconMenu = new QMenu(this);
    aboutAction = new QAction(tr("&About"), this);
    quitAction = new QAction(tr("&Quit"), this);
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(aboutMain()));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(quitMain()));
    trayIconMenu->addAction(aboutAction);
    trayIconMenu->addAction(quitAction);
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setToolTip(QString("QuickWin"));
    trayIcon->setIcon(QIcon("icon.png"));
    trayIcon->show();

    // Set up UI items.
    ui->setupUi(this);
    proxy->setSourceModel(model);
    ui->winView->setModel(proxy);
    proxy->setFilterKeyColumn(1);
    ui->winView->setSortingEnabled(true);
    ui->winView->sortByColumn(0, Qt::AscendingOrder);
    ui->winView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Number"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Title"));

    connect(ui->findText, SIGNAL(returnPressed()),
            this,SLOT(onTextEnter()));
    connect(ui->findText, SIGNAL(textChanged(const QString &)),
            this, SLOT(onTextChanged(const QString &)));
    connect(ui->winView, SIGNAL(activated(QModelIndex)),
            this, SLOT(onWitemActivate(QModelIndex)));

    // Register system-wide hotkey.
    HWND hwnd = (HWND)this->winId();
    RegisterHotKey(hwnd, 100, MOD_CONTROL, VK_SPACE);

    updateWinList();
    showMain();
}

void MainWindow::aboutMain(void) {
    QMessageBox::about(this,
            "About QuickWin",
            "QuickWin version `" + gVerStr + "`.\nVisit 'github.com/jeffrimko/QuickWin' for more information.");
            // QString("About QuickWin"),
            // QString("QuickWin " + gVerStr + "\n<a href='github.com/jeffrimko/QuickWin'>Home Page</a>"));
}

void MainWindow::quitMain(void) {
    exit(0);
}

void MainWindow::updateWinList(void)
{
    model->removeRows(0, model->rowCount());
    witems.clear();
    EnumWindows(EnumWindowsProc, (LPARAM)this);
    for(int i = 0; i < witems.size(); i++) {
        model->insertRow(i);
        model->setData(model->index(i, 0), QString::number(witems[i].num));
        model->setData(model->index(i, 1), witems[i].title);
    }
    ui->winView->setCurrentIndex(proxy->index(0,0));
    ui->noteText->clear();
    ui->noteText->append("QuickWin " + gVerStr + " found " + QString::number(witems.size()) + " windows.");
}

void MainWindow::checkSavedWins(void) {
    if(gSavedWins.size()) {
        QHashIterator<QString, HWND> i(gSavedWins);
        while (i.hasNext()) {
            i.next();
            bool win_okay = false;
            for(int j = 0; j < witems.size(); j++) {
                if(witems[j].handle == i.value()) {
                    win_okay = true;
                    break;
                }
            }
            if(false == win_okay) {
                gSavedWins.remove(i.key());
            }
        }
    }
}

void MainWindow::onTextChanged(const QString &text) {
    static bool cmd_mode = false;
    int cidx = text.indexOf(QString(";"));
    cmd_mode = (cidx > -1);
    if(!cmd_mode) {
        proxy->setFilterRegExp(QRegExp(text, Qt::CaseInsensitive,
                                            QRegExp::FixedString));
        ui->winView->setCurrentIndex(proxy->index(0,0));
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    bool hide_win = (event->key() == Qt::Key_Escape);
    bool quit_app = false;

    SelMove mv = SELMOVE_NO;
    if(event->modifiers() && Qt::ControlModifier) {
        switch(event->key()) {
            case Qt::Key_H: mv = SELMOVE_TOP; break;
            case Qt::Key_L: mv = SELMOVE_BTM; break;
            case Qt::Key_K: mv = SELMOVE_UP; break;
            case Qt::Key_J: mv = SELMOVE_DWN; break;
            case Qt::Key_M: mv = SELMOVE_MID; break;
            case Qt::Key_C: hide_win = true; break;
#ifndef QT_NO_DEBUG
            case Qt::Key_X: quit_app = true; break;
#endif
        }
    } else {
        switch(event->key()) {
            case Qt::Key_Home: mv = SELMOVE_TOP; break;
            case Qt::Key_End: mv = SELMOVE_BTM; break;
            case Qt::Key_Up: mv = SELMOVE_UP; break;
            case Qt::Key_Down: mv = SELMOVE_DWN; break;
        }
    }
    moveSel(mv);

    if(hide_win) {
        hide();
    }
    // TODO: Debug only! Remove later...
    if(quit_app) {
        quitMain();
    }
}

void MainWindow::moveSel(SelMove mv) {
    // Handle selection movement.
    int row = ui->winView->currentIndex().row();
    if((SELMOVE_UP == mv) | (SELMOVE_DWN == mv)) {
        if(SELMOVE_UP == mv) {
            if(row == 0) {
                row = proxy->rowCount() - 1;
            } else {
                row -= 1;
            }
        } else {
            row += 1;
            if(row >= proxy->rowCount()) {
                row = 0;
            }
        }
        ui->winView->setCurrentIndex(proxy->index(row,0));
    } else if(SELMOVE_TOP == mv) {
        ui->winView->setCurrentIndex(proxy->index(0,0));
    } else if(SELMOVE_MID == mv) {
        ui->winView->setCurrentIndex(proxy->index(proxy->rowCount()/2,0));
    } else if(SELMOVE_BTM == mv) {
        ui->winView->setCurrentIndex(proxy->index(proxy->rowCount()-1,0));
    }
}

void MainWindow::showWin(HWND handle) {
    if(IsIconic(handle)) {
        ShowWindow(handle, SW_RESTORE);
    }
    SetForegroundWindow(handle);
}

void MainWindow::onWitemActivate(QModelIndex index) {
    if(proxy->rowCount() == 0)
        return;
    showWin(witems[getSelWinNum()].handle);
}

uint MainWindow::getSelWinNum(void) {
    uint row = ui->winView->currentIndex().row();
    uint num = proxy->data(proxy->index(row, 0)).toInt() - 1;
    return(num);
}

void MainWindow::onTextEnter()
{
    QString text = ui->findText->text();
    int cidx = text.indexOf(QString(";"));
    bool cmd_mode = (cidx > -1);

    if(!cmd_mode) {
        onWitemActivate(ui->winView->currentIndex());
    } else {
        // Handle special commands.
        text = text.mid(cidx + 1);
        if(text.size() == 0)
            return;
        if(text[0] == QChar('s')) {
            // COMMAND: Set alias to selected window.
            text = text.mid(1);
            if(text.size() > 0) {
                uint num = getSelWinNum();
                gSavedWins[text] = witems[num].handle;
                ui->noteText->append("Set " + QString::number(num+1) + " to alias '" + text + "'.");
                ui->findText->clear();
            }
        } else if(text[0] == QChar('g')) {
            // COMMAND: Goto aliased window.
            showWin(gSavedWins[text.mid(1)]);
        } else if(text[0] == QChar('a')) {
            // COMMAND: List aliases.
            if(gSavedWins.size()) {
                QHashIterator<QString, HWND> i(gSavedWins);
                QString alias("");
                uint num = 0;
                while (i.hasNext()) {
                    i.next();
                    alias.append(" '");
                    alias.append(i.key());
                    alias.append("'");
                    num++;
                }
                ui->noteText->append("Found " + QString::number(num) + " aliases:" + alias);
            } else {
                ui->noteText->append("No aliases.");
            }
            ui->findText->clear();
        } else if(text[0] == QChar('d')) {
            // COMMAND: Delete aliases.
            ui->noteText->append("Aliases deleted.");
            gSavedWins.clear();
            ui->findText->clear();
        }
    }
}

bool MainWindow::winEvent( MSG * message, long * result )  {
    if(message->message == 786) {
        onHotkey();
    }
    return(false);
}

void MainWindow::onHotkey(void) {
    if((HWND)this->winId() == GetActiveWindow()) {
        // Already active window.
        moveSel(SELMOVE_DWN);
    }
    else {
        showMain();
    }
}

void MainWindow::showMain(void) {
    show();
    setWindowState( (windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    raise();  // for MacOS
    activateWindow(); // for Windows
    ui->findText->clear();
    ui->findText->setFocus();
}

// Callback for `EnumWindows` that lists out all window names.
BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam) {
    WinItem witem;
    MainWindow *mainwin = (MainWindow*)lParam;
    LPWSTR buff[255];
    if( (IsWindowVisible(hWnd)) &&
        (IsAltTabWindow(hWnd)) &&
        ((HWND)mainwin->winId() != hWnd) )
    {
        GetWindowText(hWnd, (LPWSTR)buff, 254);
        witem.title = QString::fromWCharArray((const LPWSTR)buff);
        witem.handle = hWnd;
        witem.num = mainwin->witems.size() + 1;
        mainwin->witems.append(witem);
    }
    return TRUE;
}

void MainWindow::windowActivationChange(bool state) {
    if(state) {
        // Lost focus.
        hide();
    } else {
        // In focus.
        updateWinList();
        checkSavedWins();
        showMain();
    }
}

BOOL IsAltTabWindow(HWND hwnd)
{
    long wndStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    if(GetWindowTextLength(hwnd) == 0)
        return false;

    // Ignore desktop window.
    if (hwnd == GetShellWindow())
        return(false);

    if(wndStyle & WS_EX_TOOLWINDOW)
        return(false);

    // Start at the root owner
    HWND hwndWalk = GetAncestor(hwnd, GA_ROOTOWNER);

    // See if we are the last active visible popup
    HWND hwndTry;
    while ((hwndTry = GetLastActivePopup(hwndWalk)) != hwndTry)
    {
        if (IsWindowVisible(hwndTry))
            break;
        hwndWalk = hwndTry;
    }
    return hwndWalk == hwnd;
}
