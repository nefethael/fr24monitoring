#include "fr24aircraft.h"

#define K_NA_STR "N/A"

FR24Aircraft::FR24Aircraft(QJsonValue flight, UpdateType updateType)
{
    m_liveStatus = flight["status"]["text"].toString();
    if(m_liveStatus.isEmpty()){
        m_liveStatus = K_NA_STR;
    }

    m_model = flight["aircraft"]["model"]["text"].toString();
    if((m_model.isEmpty())){
        m_model = flight["aircraft"]["model"]["code"].toString();
        if (m_model.isEmpty()){
            m_model = K_NA_STR;
        }
    }

    m_airline = flight["airline"]["name"].toString();
    if(m_airline.isEmpty()){
        m_airline = K_NA_STR;
    }

    m_callsign = flight["identification"]["number"]["default"].toString();
    if(m_callsign.isEmpty()){
        m_callsign = flight["identification"]["callsign"].toString();
        if(m_callsign.isEmpty()){
            m_callsign = K_NA_STR;
        }
    }

    m_registration = flight["aircraft"]["registration"].toString();
    if(m_registration.isEmpty()){
        m_registration = K_NA_STR;
    }

    m_uniqueId = m_registration;
    if(m_uniqueId.isEmpty() || (m_uniqueId == K_NA_STR)){
        m_uniqueId = m_callsign;
    }
    if(m_uniqueId.isEmpty() || (m_uniqueId == K_NA_STR)){
        m_uniqueId = m_model;

        qDebug() << QString("------------------------------------------------------------ %1 ------------------------------------------------------------").arg(flight.toString());
    }

    if(updateType == ARRIVAL){
        auto estArr = flight["time"]["estimated"]["arrival"].toInteger();
        auto schArr = flight["time"]["scheduled"]["arrival"].toInteger();
        if(schArr != 0) m_arrivalTime = QDateTime::fromSecsSinceEpoch(schArr);
        if(estArr != 0) m_arrivalTime = QDateTime::fromSecsSinceEpoch(estArr);
    }else{
        auto estDep = flight["time"]["estimated"]["departure"].toInteger();
        auto schDep = flight["time"]["scheduled"]["departure"].toInteger();
        if(schDep != 0) m_departureTime = QDateTime::fromSecsSinceEpoch(schDep);
        if(estDep != 0) m_departureTime = QDateTime::fromSecsSinceEpoch(estDep);
    }

    m_icao = flight["aircraft"]["hex"].toString(K_NA_STR);

    refreshNearestDate();
}

void FR24Aircraft::debug()
{
        qDebug() << QString("status=%1 model=%2 airline=%3 callsign=%4 registration=%5 icao=%6 arrival=%7 departure=%8 UID=%9")
                    .arg(m_liveStatus)
                    .arg(m_model)
                    .arg(m_airline)
                    .arg(m_callsign)
                    .arg(m_registration)
                    .arg(m_icao)
                    .arg(m_arrivalTime.toString())
                    .arg(m_departureTime.toString())
                    .arg(m_uniqueId);
}

void FR24Aircraft::update(FR24Aircraft & other, UpdateType updateType)
{
    if(!other.getLiveStatus().isEmpty() && (other.getLiveStatus() != K_NA_STR)){
        if (m_liveStatus != other.getLiveStatus()){
            m_liveStatus = other.getLiveStatus();
        }
    }

    if(!other.getModel().isEmpty() && (other.getModel() != K_NA_STR)){
        if (m_model != other.getModel()){
            m_model = other.getModel();
        }
    }

    if(!other.getAirline().isEmpty() && (other.getAirline() != K_NA_STR)){
        if(m_airline != other.getAirline()){
            m_airline = other.getAirline();
        }
    }

    if(!other.getCallsign().isEmpty() && (other.getCallsign() != K_NA_STR)){
        if(m_callsign != other.getCallsign()){
            m_callsign = other.getCallsign();
        }
    }

    if(!other.getRegistration().isEmpty() && (other.getRegistration() != K_NA_STR)){
        if(m_registration != other.getRegistration()){
            m_registration = other.getRegistration();
        }
    }

    if(updateType == ARRIVAL){
        if(other.getArrivalTime().isValid()){
            if(m_arrivalTime != other.getArrivalTime()){
                m_arrivalTime = other.getArrivalTime();
            }
        }
    }else{
        if(other.getDepartureTime().isValid()){
            if(m_departureTime != other.getDepartureTime()){
                m_departureTime = other.getDepartureTime();
            }
        }
    }

    refreshNearestDate();

    m_icao = other.getICAO();
}

bool FR24Aircraft::isOutdated() const
{
    auto midnight = QDateTime::currentDateTime();
    midnight.setTime(QTime());
    auto midnightTimestamp = midnight.toSecsSinceEpoch();

    auto tomorrow = midnight.addDays(1);
    auto tomorrowTimestamp = tomorrow.toSecsSinceEpoch();

    return isOutdated(midnightTimestamp, tomorrowTimestamp, true);
}

bool FR24Aircraft::isOutdated(qint64 midnightTimestamp, qint64 tomorrowTimestamp, bool skipTomorrow) const
{
    auto mdn = QDateTime::fromSecsSinceEpoch(midnightTimestamp);
    auto tmw = QDateTime::fromSecsSinceEpoch(tomorrowTimestamp);

    if(m_arrivalTime.isValid()){
        if((m_arrivalTime < mdn) || (skipTomorrow && m_arrivalTime > tmw)){
            return true;
        }
    }

    if(m_departureTime.isValid()){
        if((m_departureTime < mdn) || (skipTomorrow && m_departureTime > tmw)){
            return true;
        }
    }

    return false;
}

bool FR24Aircraft::isNotInteresting(QList<QVariant> airline, QList<QVariant> aircraft, QList<QVariant> shortcraft)
{
    if(airline.indexOf(m_airline) != -1){
        if((aircraft.indexOf(m_model) != -1) || (shortcraft.indexOf(m_model) != -1)){
            return true;
        }
    }
    return false;
}

void FR24Aircraft::refreshNearestDate()
{
    if((m_arrivalTime.isValid()) && (m_departureTime.isValid())){
        auto now = QDateTime::currentDateTime().toSecsSinceEpoch();
        auto p1n = qAbs(m_departureTime.toSecsSinceEpoch() - now);
        auto p2n = qAbs(m_departureTime.toSecsSinceEpoch() - now);
        if(p1n < p2n){
            m_nearestDate = m_arrivalTime;
        }else{
            m_nearestDate = m_departureTime;
        }
    }else if (m_arrivalTime.isValid()){
        m_nearestDate = m_arrivalTime;
    }else{
        m_nearestDate = m_departureTime;
    }
}


bool operator!=(FR24Aircraft& d1, FR24Aircraft& d2) // Call by reference
{
    auto & diff = d1.getDiff();
    diff = "";
    bool isDiff = false;

    if (d2.getUID()  != d1.getUID()){
        isDiff = true;
        diff.append(QString("[UID:%1!=%2]").arg(d1.getUID()).arg(d2.getUID()));
    }

    if (d2.getLiveStatus()  != d1.getLiveStatus()){
        // don't trigger notification for that
        diff.append(QString("[STS:%1!=%2]").arg(d1.getLiveStatus()).arg(d2.getLiveStatus()));
    }

    if (d2.getModel()  != d1.getModel()){
        isDiff = true;
        diff.append(QString("[MDL:%1!=%2]").arg(d1.getModel()).arg(d2.getModel()));
    }

    if (d2.getAirline()  != d1.getAirline()){
        isDiff = true;
        diff.append(QString("[LIN:%1!=%2]").arg(d1.getAirline()).arg(d2.getAirline()));
    }

    if (d2.getCallsign()  != d1.getCallsign()){
        isDiff = true;
        diff.append(QString("[CAL:%1!=%2]").arg(d1.getCallsign()).arg(d2.getCallsign()));
    }

    if (d2.getRegistration()  != d1.getRegistration()){
        isDiff = true;
        diff.append(QString("[REG:%1!=%2]").arg(d1.getRegistration()).arg(d2.getRegistration()));
    }

    if (qAbs(d2.getDepartureTime().secsTo(d1.getDepartureTime())) > 3600){
        isDiff = true;
        diff.append(QString("[DEP:%1!=%2]").arg(d1.getDepartureTime().toString()).arg(d2.getDepartureTime().toString()));
    }

    if (qAbs(d2.getArrivalTime().secsTo(d1.getArrivalTime())) > 3600){
        isDiff = true;
        diff.append(QString("[ARR:%1!=%2]").arg(d1.getArrivalTime().toString()).arg(d2.getArrivalTime().toString()));
    }

    if (d2.getICAO()  != d1.getICAO()){
        isDiff = true;
        diff.append(QString("[ICA:%1!=%2]").arg(d1.getICAO()).arg(d2.getICAO()));
    }

    return isDiff;
}
