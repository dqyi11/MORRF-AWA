#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QPixmap>
#include <QProgressBar>
#include "morrf_awa_viz/morrfvisualizer.h"

class ConfigObjDialog;
class MORRF;

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    MORRFVisualizer * mpViz;

    bool planPath(QString config_filename, QString paths_filename, QString weight_filename = "", QString log_filename="");
protected:
    void createMenuBar();
    void createActions();
    bool openMap(QString filename);

    void keyPressEvent(QKeyEvent *event);
    void updateStatus();

    bool setupPlanning(QString filename);
    bool exportPaths(QString filename);
    void initMORRF();

    bool loadConfiguration(QString filename);
    bool saveConfiguration(QString filename);

private:
    void updateTitle();

    QMenu*        mpFileMenu;
    QAction*      mpOpenAction;
    QAction*      mpSaveAction;
    QAction*      mpExportAction;
    QMenu*        mpEditMenu;
    QAction*      mpLoadMapAction;
    QAction*      mpLoadObjAction;
    QAction*      mpRunAction;
    QMenu*        mpContextMenu;
    QAction*      mpAddStartAction;
    QAction*      mpAddGoalAction;
    QLabel*       mpStatusLabel;
    QProgressBar* mpStatusProgressBar;
    QPixmap*      mpMap;

    QAction*      mpDumpWeightAction;
    QAction*      mpResetAction;
    QAction*      mpSaveLogFileAction;

    QPoint mCursorPoint;

    ConfigObjDialog * mpConfigObjDialog;
    MORRF           * mpMORRF;


private slots:
    void contextMenuRequested(QPoint point);
    void onOpen();
    void onSave();
    void onExport();
    void onLoadMap();
    void onLoadObj();
    void onRun();
    void onAddStart();
    void onAddGoal();

    void onDumpWeight();
    void onReset();
    void onSaveLogFile();
};

#endif // MAINWINDOW_H
