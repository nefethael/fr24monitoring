#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QNetworkAccessManager>
#include <QTimer>

#include "fr24aircraft.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;


    void replyFinished(QNetworkReply *reply);
    QNetworkAccessManager *m_manager = nullptr;
    QTimer *m_reqTimer = nullptr;
    QMap<QString, QString> m_photoMap ;
    QMap<QString, FR24Aircraft> m_fr24Map;
};
#endif // MAINWINDOW_H
