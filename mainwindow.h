#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QNetworkAccessManager>
#include <QTimer>

#include "fr24aircraft.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class FR24Model;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    inline FR24Model* getCraftModel();
    void replyFinished(FR24Aircraft::UpdateType updateType);
    void refreshTimestamp();
    void startRequest(qint64 delay);

    QNetworkAccessManager *m_manager = nullptr;
    QMap<QString, QString> m_photoMap ;
    QMap<QString, FR24Aircraft> m_fr24Map;
    FR24Model * m_model;

    qint64 m_midnightTimestamp;
    qint64 m_tomorrowTimestamp;
};
#endif // MAINWINDOW_H
