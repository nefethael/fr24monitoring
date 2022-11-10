#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDateTime>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#define K_REFRESH_PERIOD_MS 60000

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    static const QString K_FR24_URL_TEMPLATE = "https://api.flightradar24.com/common/v1/airport.json?code=%1&plugin[]=&plugin-setting[schedule][mode]=%2&page=1&limit=100&plugin-setting[schedule][timestamp]=%3";
    static const QStringList K_COMMON_AIRCRAFT = {"Airbus A319-111", "Boeing 737-8AS", "Airbus A320-214", "Embraer E190STD", "Airbus A321-212", "Mitsubishi CRJ-1000", "Airbus A320-271N", "Embraer E195-E2", "Embraer E190LR", "Boeing 737-7K2", "Airbus A319-112", "Airbus A320-251N", "Mitsubishi CRJ-1000EL", "Airbus A321-111", "Airbus A320-216", "Embraer E170STD", "Boeing 737-8K2", "Embraer E190-E2", "Airbus A318-111", "Airbus A320-232", "Boeing 737 MAX 8-200"};
    static const QStringList K_COMMON_AIRLINE = {"Air France", "KLM", "Ryanair", "easyJet", "Lufthansa", "Vueling", "Volotea", "Chalair Aviation", "Iberia Regional", "Iberia Express", "Helvetic Airways", "Finistair", "Royal Air Maroc"};


    auto midnight = QDateTime::currentDateTime();
    midnight.setTime(QTime());
    auto midnightTimestamp = midnight.toSecsSinceEpoch();
    qDebug() << midnightTimestamp;
    auto tomorrow = midnight.addDays(1);
    auto tomorrowTimestamp = tomorrow.toSecsSinceEpoch();
    qDebug() << tomorrowTimestamp;

    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, &QNetworkAccessManager::finished, this, &MainWindow::replyFinished);

    auto execFR24Req = [midnightTimestamp, this](){
        if(!m_manager) {
            qInfo() << "Manager is not ready yet.";
        }

        QNetworkRequest reqArr;
        reqArr.setUrl(K_FR24_URL_TEMPLATE.arg("bod").arg("arrivals").arg(midnightTimestamp));
        m_manager->get(reqArr);

        QNetworkRequest reqDep;
        reqDep.setUrl(K_FR24_URL_TEMPLATE.arg("bod").arg("departures").arg(midnightTimestamp));
        m_manager->get(reqDep);
    };

    m_reqTimer = new QTimer(this);
    connect(m_reqTimer, &QTimer::timeout, this, [execFR24Req](){
        execFR24Req();
    });

    execFR24Req();
    m_reqTimer->start(K_REFRESH_PERIOD_MS);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_manager;
}

void MainWindow::replyFinished(QNetworkReply *reply)
{
    auto replyBytes = reply->readAll();
    auto json = QJsonDocument::fromJson(replyBytes);

    QJsonValue data = json["result"]["response"]["airport"]["pluginData"]["schedule"]["arrivals"]["data"];
    bool isArrival = true;
    QJsonValue photo = json["result"]["response"]["aircraftImages"];
    if(data.isUndefined()){
        data = json["result"]["response"]["airport"]["pluginData"]["schedule"]["departures"]["data"];
        isArrival = false;
    }

    for (QJsonValueRef img : photo.toArray()){
        auto imgUrl = img.toObject().value("images")["thumbnails"][0]["src"].toString();
        auto registration = img.toObject().value("registration").toString();
        m_photoMap[registration]=imgUrl;
    }

    for (auto elt : data.toArray()){
        auto flight = elt.toObject().value("flight");

        FR24Aircraft tmp(flight, isArrival);
        if(m_fr24Map.contains(tmp.getUID())){
            m_fr24Map.find(tmp.getUID())->update(tmp, isArrival);
        }else{
            m_fr24Map.insert(tmp.getUID(), tmp);
        }
    }

    for (auto kv : m_fr24Map){
        kv.debug();
    }

    reply->deleteLater();
}

