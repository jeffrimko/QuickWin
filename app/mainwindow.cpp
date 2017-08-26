/*=============================================================*/
/* SECTION: Includes                                           */
/*=============================================================*/

#define _WIN32_WINNT 0x0600
#include <windows.h>
#include <psapi.h>
#include <QDesktopWidget>

#include <Qt>

#include <QKeyEvent>
#include <QMessageBox>
#include <QShortcut>
#include <QHash>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "cmdformatter.h"

/*=============================================================*/
/* SECTION: Local Data                                         */
/*=============================================================*/

/// Stores saved alias windows.
QHash<HWND, QString> gSavedWins;

/// The application version string.
QString gVerStr("0.5.0-alpha");

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
        model(new QStandardItemModel(0, 4, parent))
{
    // Set the window style.
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags | Qt::WindowStaysOnTopHint | Qt::ToolTip);

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
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Executable"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Alias"));
    sizePosMain();

    connect(ui->cmdText, SIGNAL(returnPressed()),
            this,SLOT(onTextEnter()));
    connect(ui->cmdText, SIGNAL(textChanged(const QString &)),
            this, SLOT(onTextChanged(const QString &)));
    connect(ui->winView, SIGNAL(activated(QModelIndex)),
            this, SLOT(onWitemActivate(QModelIndex)));

    // Register system-wide hotkey.
    HWND hwnd = (HWND)this->winId();
    RegisterHotKey(hwnd, 100, MOD_CONTROL | MOD_ALT, VK_SPACE);

    updateWinList(true);
}

void MainWindow::sizePosMain(void) {
    // Center window.
    QDesktopWidget *desktop = QApplication::desktop();
    int width = desktop->screenGeometry(desktop->primaryScreen()).width() * 0.6;
    int height = desktop->screenGeometry(desktop->primaryScreen()).height() * 0.6;
    setFixedSize(width, height);
    move(
            (desktop->screenGeometry(desktop->primaryScreen()).width() - width) / 2,
            (desktop->screenGeometry(desktop->primaryScreen()).height() - height) / 2
        );

    ui->winView->header()->resizeSection(0, width * 0.08);
    ui->winView->header()->resizeSection(1, width * 0.7);
}

void MainWindow::aboutMain(void) {
    QMessageBox::about(this,
            "About QuickWin",
            "QuickWin version `" + gVerStr + "`.\nVisit 'github.com/jeffrimko/QuickWin' for more information.");
}

void MainWindow::quitMain(void) {
    exit(0);
}

void MainWindow::updateWinList(bool clear_note) {
    model->removeRows(0, model->rowCount());
    witems.clear();
    EnumWindows(EnumWindowsProc, (LPARAM)this);
    for(int i = 0; i < witems.size(); i++) {
        model->insertRow(i);
        model->setData(model->index(i, 0), QString::number(witems[i].num));
        model->setData(model->index(i, 1), witems[i].title);
        model->setData(model->index(i, 2), witems[i].exec);
        model->setData(model->index(i, 3), gSavedWins[witems[i].handle]);
    }
    ui->winView->setCurrentIndex(proxy->index(0,0));
    if(clear_note) {
        ui->noteText->clear();
        ui->noteText->append("QuickWin " + gVerStr + " found " + QString::number(witems.size()) + " windows.");
    }
}

void MainWindow::checkSavedWins(void) {
    if(gSavedWins.size()) {
        QHashIterator<HWND, QString> i(gSavedWins);
        while (i.hasNext()) {
            i.next();
            bool win_okay = false;
            for(int j = 0; j < witems.size(); j++) {
                if(witems[j].handle == i.key()) {
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
    static QString prev_ptrn = "";
    cmds_t cmds;
    QString ptrn = "";
    format_cmds(cmds, text.toStdString());
    if("" != cmds["number"]) {
        proxy->setFilterKeyColumn(0);
        ptrn = QString::fromStdString(cmds["number"]);
    } else if("" != cmds["executable"]) {
        proxy->setFilterKeyColumn(2);
        ptrn = QString::fromStdString(cmds["executable"]);
    } else if(cmds.find("get") != cmds.end()) {
        proxy->setFilterKeyColumn(3);
        if("" != cmds["get"]) {
            ptrn = QString::fromStdString(cmds["get"]);
        } else {
            ptrn = "?";
        }
    } else {
        proxy->setFilterKeyColumn(1);
        ptrn = QString::fromStdString(cmds["title"]);
    }
    if(prev_ptrn != ptrn) {
        proxy->setFilterRegExp(QRegExp(ptrn, Qt::CaseInsensitive, QRegExp::Wildcard));
        ui->winView->setCurrentIndex(proxy->index(0,0));
    }
    prev_ptrn = ptrn;
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
            case Qt::Key_E: mv = SELMOVE_EXE; break;
            case Qt::Key_S: onWitemActivate(ui->winView->currentIndex());
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
    } else if(SELMOVE_EXE == mv) {
        QString exec = proxy->data(proxy->index(row,2)).toString();
        int idx = row + 1;
        while(idx != row) {
            int rc = proxy->rowCount();
            if(idx >= rc) {
                idx = 0;
            }
            QString rexec = proxy->data(proxy->index(idx,2)).toString();
            if(exec == rexec) {
                break;
            }
            idx++;
        }
        ui->winView->setCurrentIndex(proxy->index(idx,0));
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
    bool stay = false;
    cmds_t cmds;
    QString text = ui->cmdText->text();
    format_cmds(cmds, text.toStdString());

    if(cmds.find("help") != cmds.end()) {
        QString help = "Commands: ";
        for(std::string cmd : list_cmd_types()) {
            help += QString::fromStdString(cmd);
            help += " ";
        }
        ui->noteText->append(help);
        stay = true;
    }
    if(cmds.find("delete") != cmds.end()) {
        gSavedWins.clear();
        ui->noteText->append("Aliases deleted.");
        stay = true;
    }
    if(cmds.find("set") != cmds.end()) {
        uint num = getSelWinNum();
        if("" != cmds["set"]) {
            QString name = QString::fromStdString(cmds["set"]);
            gSavedWins[witems[num].handle] = name;
            ui->noteText->append("Set " + QString::number(num+1) + " to alias '" + name + "'.");
        } else {
            gSavedWins.remove(witems[num].handle);
            ui->noteText->append("Removed alias from " + QString::number(num+1) + ".");
        }
        stay = true;
    }

    if(!stay) {
        onWitemActivate(ui->winView->currentIndex());
    } else {
        ui->cmdText->clear();
        updateWinList(false);
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
    sizePosMain();
    show();
    setWindowState( (windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    raise();  // for MacOS
    activateWindow(); // for Windows
    ui->cmdText->clear();
    ui->cmdText->setFocus();
}

// Callback for `EnumWindows` that lists out all window names.
BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam) {
    WinItem witem;
    MainWindow *mainwin = (MainWindow*)lParam;
    DWORD pid;

    LPWSTR buff[1024];
    if( (IsWindowVisible(hWnd)) &&
        (IsAltTabWindow(hWnd)) &&
        ((HWND)mainwin->winId() != hWnd) )
    {
        GetWindowText(hWnd, (LPWSTR)buff, sizeof(buff));
        witem.title = QString::fromWCharArray((const LPWSTR)buff);
        witem.handle = hWnd;
        witem.num = mainwin->witems.size() + 1;
        GetWindowThreadProcessId(hWnd, &pid);
        // NOTE: This call is needed to allow access to metadata of the
        // process. If original handle is used to for metadata read attempt, it
        // will likely result in an error.
        HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        // NOTE: Using `GetProcessImageFileName()` here fixes the 32/64-bit
        // error.
        if(GetProcessImageFileName(handle, (LPWSTR)buff, sizeof(buff))) {
            witem.exec = QString::fromWCharArray((const LPWSTR)buff);
            witem.exec = witem.exec.mid(witem.exec.lastIndexOf("\\") + 1);
        } else {
            witem.exec = QString("NA");
        }
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
        updateWinList(true);
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

