#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "fr24model.h"
#include "fr24proxymodel.h"
#include "networknotifier.h"

#include <QDateTime>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>

#define K_REFRESH_PERIOD_MS 60000

static const QString K_FR24_URL_TEMPLATE = "https://api.flightradar24.com/common/v1/airport.json?code=%1&plugin[]=&plugin-setting[schedule][mode]=%2&page=1&limit=100&plugin-setting[schedule][timestamp]=%3";

static QJsonDocument loadJson(QString fileName) {
    QFile jsonFile(fileName);
    jsonFile.open(QFile::ReadOnly);
    return QJsonDocument().fromJson(jsonFile.readAll());
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QSettings settings("setup.ini", QSettings::Format::IniFormat);
    if(settings.status() != QSettings::Status::NoError){
        qCritical() << "Setup.ini not loaded";
    }
    m_airport = settings.value("airport").toString();
    if(m_airport.isEmpty()){
        qCritical() << "airport is not set";
    }
    auto filters = loadJson("filters.json");
    if(filters.isEmpty()){
        qCritical() << "filters.json not loaded";
    }
    m_commonAirline = filters["common_airline"].toArray().toVariantList();
    m_commonAircraft = filters["common_aircraft"].toArray().toVariantList();
    m_commonShortcraft = filters["common_shortcraft"].toArray().toVariantList();

    m_model = new FR24Model(this);

    FR24ProxyModel *proxyModel = new FR24ProxyModel(this);
    proxyModel->setSourceModel(m_model);
    ui->tableView->setModel(proxyModel);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->sortByColumn(FR24Model::FM_NEARESTDATE, Qt::AscendingOrder);

    m_manager = new QNetworkAccessManager(this);

    // setup notifier
    initializeNotifier(settings);

    startRequest(10);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_manager;
}

void MainWindow::initializeNotifier(const QSettings &settings)
{
    m_notifierList.append(new TelegramNotifier(this));

    for(auto & n : m_notifierList){
        n->setup(settings, this);
    }
}

void MainWindow::refreshTimestamp()
{
    auto midnight = QDateTime::currentDateTime();
    midnight.setTime(QTime());
    m_midnightTimestamp = midnight.toSecsSinceEpoch();

    auto tomorrow = midnight.addDays(1);
    m_tomorrowTimestamp = tomorrow.toSecsSinceEpoch();
}

void MainWindow::notifyOnDelta()
{
    for(auto kv : m_fr24Map){
        if(!kv.isNotInteresting(m_commonAirline, m_commonAircraft, m_commonShortcraft)){
            if (m_previousFr24Map.count(kv.getUID()) == 0){
                qDebug() << "notify new aircraft" << kv.getUID();
                notify(kv);
            }else{
                auto & p = *m_previousFr24Map.find(kv.getUID());
                if(kv != p){
                    qDebug() << "notify updated aircraft" << kv.getUID() << ": " << kv.getDiff();
                    notify(kv);
                }
            }
        }
    }

    m_previousFr24Map = m_fr24Map;
    m_fr24Map.clear();
}

void MainWindow::startRequest(qint64 delay)
{
    qDebug() << "startRequest " << delay;
    QTimer::singleShot(delay, this, [this] () {

        notifyOnDelta();

        m_photoMap.clear();
        refreshTimestamp();
        QNetworkRequest reqArr;
        reqArr.setUrl(K_FR24_URL_TEMPLATE.arg(m_airport).arg("arrivals").arg(m_midnightTimestamp));
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
        if(m_photoMap.count(tmp.getRegistration()) != 0){
            tmp.getPhotoUrl() = m_photoMap[tmp.getRegistration()];
        }

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
                req.setUrl(K_FR24_URL_TEMPLATE.arg(m_airport).arg("departures").arg(m_midnightTimestamp));
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
                        || kv.isNotInteresting(m_commonAirline, m_commonAircraft, m_commonShortcraft)){
                    //
                }else{
                    lst.append(kv);
                }
            }
            getCraftModel()->clearCraft();
            getCraftModel()->refreshCraft(lst);

            startRequest(60000);
            break;
    };



    reply->deleteLater();
}

