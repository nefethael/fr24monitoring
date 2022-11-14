#ifndef FR24AIRCRAFT_H
#define FR24AIRCRAFT_H

#include <QString>
#include <QJsonValue>
#include <QDateTime>

class FR24Aircraft
{
public:
    enum UpdateType{
        ARRIVAL,
        DEPARTURE,
        FULL
    };
    FR24Aircraft(QJsonValue flight, UpdateType updateType = FULL);
    void update(FR24Aircraft & other, UpdateType updateType = FULL);

    void debug();

    inline QString   getUID() const { return m_uniqueId; } const
    inline QString   getLiveStatus() const { return m_liveStatus;} const
    inline QString   getModel() const { return m_model;} const
    inline QString   getAirline() const { return m_airline;} const
    inline QString   getCallsign() const { return m_callsign;} const
    inline QString   getRegistration() const { return m_registration;} const
    inline QDateTime getDepartureTime() const { return m_departureTime;} const
    inline QDateTime getArrivalTime()const { return m_arrivalTime;} const
    inline QDateTime getNearestDate() const { return m_nearestDate; } const
    inline QString   getICAO() const { return m_icao;}

    inline QString   getDiff() const { return m_diff;}
    inline QString   getPhotoUrl() const { return m_photoUrl;}
    inline QString & getDiff() { return m_diff;};
    inline QString & getPhotoUrl() { return m_photoUrl;};

    bool isOutdated() const;
    bool isOutdated(qint64 midnightTimestamp, qint64 tomorrowTimestamp, bool skipTomorrow=true) const;
    bool isNotInteresting(const QList<QVariant> airline, const QList<QVariant> aircraft, QList<QVariant> shortcraft);
    void refreshNearestDate();

    friend bool operator!=(FR24Aircraft&, FR24Aircraft&);

private:
    QString m_uniqueId;

    QString m_liveStatus;
    QString m_model;
    QString m_airline;
    QString m_callsign;
    QString m_registration;
    QDateTime m_departureTime;
    QDateTime m_arrivalTime;
    QDateTime m_nearestDate;
    QString m_icao;

    QString m_diff;
    QString m_photoUrl;
};

#endif // FR24AIRCRAFT_H
