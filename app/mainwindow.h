/*=============================================================*/
/* DEVELOPED 2015, REVISED 2015, Jeff Rimko.                   */
/*=============================================================*/

#ifndef _MAINWINDOW_H_  // Define include guard.
#define _MAINWINDOW_H_

/*=============================================================*/
/* SECTION: Includes                                           */
/*=============================================================*/

#include <QMainWindow>
#include <QStandardItemModel>
#include <QAction>
#include <QSortFilterProxyModel>
#include <QSystemTrayIcon>
#include <QMenu>

/*=============================================================*/
/* SECTION: Namespaces                                         */
/*=============================================================*/

namespace Ui {
class MainWindow;
}

/*=============================================================*/
/* SECTION: Global Definitions                                 */
/*=============================================================*/

struct WinItem {
    uint num;
    QString title;
    QString exec;
    QString alias;
    HWND handle;
};

enum SelMove {
    SELMOVE_NO,
    SELMOVE_UP,
    SELMOVE_DWN,
    SELMOVE_TOP,
    SELMOVE_BTM,
    SELMOVE_MID
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QList<WinItem> witems;
    explicit MainWindow(QWidget *parent=0);
    ~MainWindow();
    Ui::MainWindow *ui;
    QStandardItemModel *model;
    QSortFilterProxyModel *proxy;

private slots:
    void onTextEnter();
    void onWitemActivate(QModelIndex index);
    void keyPressEvent(QKeyEvent *event);
    void onTextChanged(const QString &text);
    void quitMain(void);
    void aboutMain(void);

private:
    virtual bool winEvent(MSG *message, long *result);
    virtual void windowActivationChange(bool state);
    void updateWinList(void);
    void onHotkey(void);
    void sizePosMain(void);
    void showMain(void);
    void showWin(HWND handle);
    void moveSel(SelMove mv);
    uint getSelWinNum(void);
    void checkSavedWins(void);
    void listAlias(void);
    void delAlias(void);
    void setAlias(QString name, uint wnum);
    void getAlias(QString name);

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    QAction *aboutAction;
    QAction *quitAction;
};

#endif  // #ifndef include guard
