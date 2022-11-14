#include "networknotifier.h"
#include <QNetworkReply>
#include "mainwindow.h"

Notifier::Notifier(QObject * parent) : QObject(parent)
{
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, &QNetworkAccessManager::finished, this, &Notifier::replyFinished);
}

Notifier::~Notifier()
{
    delete m_manager;
}

void TelegramNotifier::setup(const QSettings & settings, MainWindow * origin)
{
    m_telegramChat = settings.value("telegram_chat").toString();
    m_telegramToken = settings.value("telegram_token").toString();

    if(m_telegramToken.isEmpty()){
        qInfo() << "No Telegram information, don't notify!";
    }else{
        connect(origin, &MainWindow::notify, this, &TelegramNotifier::sendNotification);
    }
}

bool TelegramNotifier::sendNotification(const FR24Aircraft & craft)
{
    QString text = QString("Airline:`%1` \\| Model:*%2* \\| Reg:`%3` \\| Callsign:`%4` \\| ETA:`%5` \\| ETD:`%6`")
            .arg(craft.getAirline())
            .arg(craft.getModel())
            .arg(craft.getRegistration())
            .arg(craft.getCallsign())
            .arg(craft.getArrivalTime().toString("hh:mm(dd)"))
            .arg(craft.getDepartureTime().toString("hh:mm(dd)"));

    if(!craft.getDiff().isEmpty()){
        text.append(QString(" \\| Updating:%1").arg(craft.getDiff()));
    }

    if(!craft.getICAO().isEmpty() && craft.getICAO() != "N/A"){
        text.append(QString(" \\| [%1](https://globe.adsbexchange.com/?icao=%1)").arg(craft.getICAO()));
    }

    if(!craft.getPhotoUrl().isEmpty()){
        text.prepend(QString("[](%1) \\| ").arg(craft.getPhotoUrl().replace("_", "\\\\_")));
    }

    QString str = QString("{\"chat_id\":\"%1\", \"text\": \"%2\", \"disable_web_page_preview\": \"false\", \"parse_mode\": \"Markdown\" }")
        .arg(m_telegramChat)
        .arg(text);

    QNetworkRequest req;
    req.setUrl(QString("https://api.telegram.org/bot%1/sendMessage").arg(m_telegramToken));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    m_manager->post(req, str.toUtf8());

    return true;
}

void Notifier::replyFinished(QNetworkReply *reply)
{
    if (reply->operation() == QNetworkAccessManager::PostOperation){
        qInfo() << "Post OK for" << reply->request().url();
    }
    reply->deleteLater();
}

