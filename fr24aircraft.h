#ifndef FR24AIRCRAFT_H
#define FR24AIRCRAFT_H

#include <QString>
#include <QJsonValue>
#include <QDateTime>

class FR24Aircraft
{
public:
    FR24Aircraft(QJsonValue flight, bool isArrival);
    void update(FR24Aircraft & other, bool isArrival);

    void debug();

    inline QString   getUID() { return m_uniqueId; }
    inline QString   getLiveStatus(){ return m_liveStatus;}
    inline QString   getModel(){ return m_model;}
    inline QString   getAirline(){ return m_airline;}
    inline QString   getCallsign(){ return m_callsign;}
    inline QString   getRegistration(){ return m_registration;}
    inline QDateTime getEstimatedDeparture(){ return m_estimatedDeparture;}
    inline QDateTime getScheduledDeparture(){ return m_scheduledDeparture;}
    inline QDateTime getEstimatedArrival(){ return m_estimatedArrival;}
    inline QDateTime getScheduledArrival(){ return m_scheduledArrival;}
    inline QString   getICAO(){ return m_icao;}

    inline bool isValid() { return m_isArrCmplt && m_isDepCmplt; }

private:
    QString m_uniqueId;

    QString m_liveStatus;
    QString m_model;
    QString m_airline;
    QString m_callsign;
    QString m_registration;
    QDateTime m_estimatedDeparture;
    QDateTime m_scheduledDeparture;
    QDateTime m_estimatedArrival;
    QDateTime m_scheduledArrival;
    QString m_icao;

    bool m_isArrCmplt = false;
    bool m_isDepCmplt = false;
};

#endif // FR24AIRCRAFT_H
