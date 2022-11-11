#include "fr24model.h"


#include <QDebug>
#include <QTimer>
#include <cmath>
#include <QDateTime>
#include <QColor>
#include <QBrush>



FR24Model::FR24Model(QObject *parent)
    : QAbstractTableModel(parent)
{

}

int FR24Model::rowCount(const QModelIndex & /*parent*/) const
{
   return m_fr24Data.size();
}

int FR24Model::columnCount(const QModelIndex & /*parent*/) const
{
    return 10;
}

QVariant FR24Model::data(const QModelIndex &index, int role) const
{
    auto row = index.row();
    auto col = index.column();
    auto & fr24data = m_fr24Data[row];

    if (role == Qt::DisplayRole){
        switch(col){
        case FM_UNIQUEID:
            return fr24data.getUID();
        case FM_LIVESTATUS:
            return fr24data.getLiveStatus();
        case FM_MODEL:
            return fr24data.getModel();
        case FM_AIRLINE:
            return fr24data.getAirline();
        case FM_CALLSIGN:
            return fr24data.getCallsign();
        case FM_REGISTRATION:
            return fr24data.getRegistration();
        case FM_DEPARTURETIME:
            return fr24data.getDepartureTime();
        case FM_ARRIVALTIME:
            return fr24data.getArrivalTime();
        case FM_NEARESTDATE:
            return fr24data.getNearestDate();
        case FM_ICAO:
            return fr24data.getICAO();
        default:
            return QVariant();
        }
    }else if (role == Qt::BackgroundRole) {
        QColor color = Qt::white;
        if(fr24data.isOutdated()){
            color = Qt::darkCyan;
        }else if (fr24data.getNearestDate() < QDateTime::currentDateTime()){
            color = Qt::darkGray;
        }
        return QBrush(color);
     }
    return QVariant();
}

QVariant FR24Model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch(section){
        case FM_UNIQUEID:
            return QString("UID");
        case FM_LIVESTATUS:
            return QString("Status");
        case FM_MODEL:
            return QString("Model");
        case FM_AIRLINE:
            return QString("Airline");
        case FM_CALLSIGN:
            return QString("Callsign");
        case FM_REGISTRATION:
            return QString("Registration");
        case FM_DEPARTURETIME:
            return QString("Departure");
        case FM_ARRIVALTIME:
            return QString("Arrival");
        case FM_NEARESTDATE:
            return QString("Nearest date");
        case FM_ICAO:
            return QString("ICAO");
        default:
            QVariant();
        }
    }
    return QVariant();
}

void FR24Model::refreshCraft(QVector<FR24Aircraft>& lst)
{
    beginResetModel();
    for(auto i = 0; i < lst.size(); i++){
        bool isFound = false;
        for (auto j = 0; (j < m_fr24Data.size()) && !isFound; j++){
            if(m_fr24Data[j].getUID() == lst[i].getUID()){
                m_fr24Data[j].update(lst[i]);

                isFound = true;
            }
        }
        if(!isFound){
            m_fr24Data.append(lst[i]);
        }
    }
    endResetModel();
}

void FR24Model::clearCraft()
{
    auto nowsec = QDateTime::currentSecsSinceEpoch();

    beginResetModel();
    m_fr24Data.erase(std::remove_if(m_fr24Data.begin(), m_fr24Data.end(), [this, nowsec](const FR24Aircraft & elt){
        return true;
    }), m_fr24Data.end());

    endResetModel();
}
