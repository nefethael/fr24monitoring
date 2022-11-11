#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "fr24model.h"
#include "fr24proxymodel.h"

#include <QDateTime>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#define K_REFRESH_PERIOD_MS 60000

static const QString K_FR24_URL_TEMPLATE = "https://api.flightradar24.com/common/v1/airport.json?code=%1&plugin[]=&plugin-setting[schedule][mode]=%2&page=1&limit=100&plugin-setting[schedule][timestamp]=%3";
static const QStringList K_COMMON_AIRCRAFT = {"Airbus A319-111", "Boeing 737-8AS", "Airbus A320-214", "Embraer E190STD", "Airbus A321-212", "Mitsubishi CRJ-1000", "Airbus A320-271N", "Embraer E195-E2", "Embraer E190LR", "Boeing 737-7K2", "Airbus A319-112", "Airbus A320-251N", "Mitsubishi CRJ-1000EL", "Airbus A321-111", "Airbus A320-216", "Embraer E170STD", "Boeing 737-8K2", "Embraer E190-E2", "Airbus A318-111", "Airbus A320-232", "Boeing 737 MAX 8-200"};
static const QStringList K_COMMON_AIRLINE = {"Air France", "KLM", "Ryanair", "easyJet", "Lufthansa", "Vueling", "Volotea", "Chalair Aviation", "Iberia Regional", "Iberia Express", "Helvetic Airways", "Finistair", "Royal Air Maroc"};


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_model = new FR24Model(this);

    FR24ProxyModel *proxyModel = new FR24ProxyModel(this);
    proxyModel->setSourceModel(m_model);
    ui->tableView->setModel(proxyModel);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->sortByColumn(FR24Model::FM_NEARESTDATE, Qt::AscendingOrder);

    m_manager = new QNetworkAccessManager(this);

    startRequest(10);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_manager;
}

void MainWindow::refreshTimestamp()
{
    auto midnight = QDateTime::currentDateTime();
    midnight.setTime(QTime());
    m_midnightTimestamp = midnight.toSecsSinceEpoch();

    auto tomorrow = midnight.addDays(1);
    m_tomorrowTimestamp = tomorrow.toSecsSinceEpoch();
}

void MainWindow::startRequest(qint64 delay)
{
    qDebug() << "startRequest " << delay;
    QTimer::singleShot(delay, this, [this] () {
        m_fr24Map.clear();
        m_photoMap.clear();
        refreshTimestamp();
        QNetworkRequest reqArr;
        reqArr.setUrl(K_FR24_URL_TEMPLATE.arg("bod").arg("arrivals").arg(m_midnightTimestamp));
        auto* reply = m_manager->get(reqArr);
        connect(reply, &QNetworkReply::finished, this, [this]{ replyFinished(FR24Aircraft::ARRIVAL);});
    });
}

FR24Model* MainWindow::getCraftModel()
{
    return dynamic_cast<FR24Model*>(dynamic_cast<FR24ProxyModel*>(ui->tableView->model())->sourceModel());
}

void MainWindow::replyFinished(FR24Aircraft::UpdateType updateType)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    auto replyBytes = reply->readAll();
    auto json = QJsonDocument::fromJson(replyBytes);

    QString reqStr = "arrivals";
    if(updateType == FR24Aircraft::DEPARTURE){
        reqStr = "departures";
    }
    QJsonValue data = json["result"]["response"]["airport"]["pluginData"]["schedule"][reqStr]["data"];

    QJsonValue photo = json["result"]["response"]["aircraftImages"];
    for (QJsonValueRef img : photo.toArray()){
        auto imgUrl = img.toObject().value("images")["thumbnails"][0]["src"].toString();
        auto registration = img.toObject().value("registration").toString();
        m_photoMap[registration]=imgUrl;
    }

    for (auto elt : data.toArray()){
        auto flight = elt.toObject().value("flight");

        FR24Aircraft tmp(flight, updateType);

        if(m_fr24Map.contains(tmp.getUID())){
            m_fr24Map.find(tmp.getUID())->update(tmp);
        }else{
            m_fr24Map.insert(tmp.getUID(), tmp);
        }
    }

    switch(updateType){
        case FR24Aircraft::ARRIVAL:
            qDebug() << "next is departures";
            QTimer::singleShot(10, this, [this] () {
                QNetworkRequest req;
                req.setUrl(K_FR24_URL_TEMPLATE.arg("bod").arg("departures").arg(m_midnightTimestamp));
                auto* reply = m_manager->get(req);
                connect(reply, &QNetworkReply::finished, this, [this]{ replyFinished(FR24Aircraft::DEPARTURE);});
            });
            break;

        case FR24Aircraft::DEPARTURE:
            qDebug() << "next is ARRIVAL";

            QVector<FR24Aircraft> lst;
            for(auto kv : m_fr24Map){
                if( kv.isOutdated(m_midnightTimestamp, m_tomorrowTimestamp, false)
                        || kv.getLiveStatus().contains("Canceled")
                        || kv.isNotInteresting(K_COMMON_AIRLINE, K_COMMON_AIRCRAFT)){
                    //
                }else{
                    lst.append(kv);
                }
            }
            getCraftModel()->refreshCraft(lst);

            startRequest(60000);
            break;
    };



    reply->deleteLater();
}

