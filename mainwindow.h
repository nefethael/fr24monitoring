#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QNetworkAccessManager>
#include <QTimer>
#include <QSettings>
#include <QJsonDocument>

#include "fr24aircraft.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class FR24Model;
class Notifier;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void notify(const FR24Aircraft & craft);

private:
    Ui::MainWindow *ui;

    inline FR24Model* getCraftModel();
    void replyFinished(FR24Aircraft::UpdateType updateType);
    void refreshTimestamp();
    void startRequest(qint64 delay);
    void notifyOnDelta();
    void initializeNotifier(const QSettings &settings);

    QNetworkAccessManager *m_manager = nullptr;
    QMap<QString, QString> m_photoMap ;
    QMap<QString, FR24Aircraft> m_fr24Map;
    QMap<QString, FR24Aircraft> m_previousFr24Map;
    FR24Model * m_model;
    QList<Notifier*> m_notifierList;

    qint64 m_midnightTimestamp;
    qint64 m_tomorrowTimestamp;

    QString m_silentStart;
    QString m_airport;
    QList<QVariant> m_commonAirline;
    QList<QVariant> m_commonAircraft;
    QList<QVariant> m_commonShortcraft;
    QList<QVariant> m_cargoAirline;
};
#endif // MAINWINDOW_H
