#ifndef NETWORKNOTIFIER_H
#define NETWORKNOTIFIER_H

#include <QNetworkAccessManager>
#include <QSettings>
#include <QMutex>
#include <QTimer>
#include "fr24aircraft.h"

class MainWindow;

class Notifier : public QObject
{
    Q_OBJECT
public:
    Notifier(QObject * parent = nullptr);
    virtual ~Notifier();

    virtual bool sendNotification(const FR24Aircraft & craft) = 0;
    virtual void setup(const QSettings & settings, MainWindow * origin) = 0;

protected:
    void replyFinished(QNetworkReply *reply);
    QNetworkAccessManager *m_manager = nullptr;
};

class TelegramNotifier : public Notifier
{
    Q_OBJECT
public:
    TelegramNotifier(QObject * parent = nullptr);
    ~TelegramNotifier();

    virtual bool sendNotification(const FR24Aircraft & craft) override;
    virtual void setup(const QSettings & settings, MainWindow * origin) override;
private:
    QString m_telegramChat;
    QString m_telegramToken;

    QList<QString> m_pendingRequests;
    QMutex m_mutex;
    QTimer *m_timer = nullptr;
};

#endif // NETWORKNOTIFIER_H
