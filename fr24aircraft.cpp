#include "fr24aircraft.h"

#define K_NA_STR "N/A"

FR24Aircraft::FR24Aircraft(QJsonValue flight, bool isArrival)
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
    if(m_uniqueId.isEmpty() || (m_uniqueId == "N/A")){
        m_uniqueId = m_callsign;
    }
    if(m_uniqueId.isEmpty() || (m_uniqueId == "N/A")){
        m_uniqueId = m_model;

        qDebug() << QString("------------------------------------------------------------ %1 ------------------------------------------------------------").arg(flight.toString());
    }

    if(isArrival){
        m_estimatedArrival = QDateTime::fromSecsSinceEpoch(flight["time"]["estimated"]["arrival"].toInteger());
        m_scheduledArrival = QDateTime::fromSecsSinceEpoch(flight["time"]["scheduled"]["arrival"].toInteger());
        m_isArrCmplt = true;
    }else{
        m_estimatedDeparture = QDateTime::fromSecsSinceEpoch(flight["time"]["estimated"]["departure"].toInteger());
        m_scheduledDeparture = QDateTime::fromSecsSinceEpoch(flight["time"]["scheduled"]["departure"].toInteger());
        m_isDepCmplt = true;
    }

    m_icao = flight["aircraft"]["hex"].toString("N/A");
}

void FR24Aircraft::debug()
{
        qDebug() << QString("status=%1 model=%2 airline=%3 callsign=%4 registration=%5 icao=%6 estArr=%7 schArr=%8 estArr=%9 schArr=%10 UID=%11 validity=%12")
                    .arg(m_liveStatus)
                    .arg(m_model)
                    .arg(m_airline)
                    .arg(m_callsign)
                    .arg(m_registration)
                    .arg(m_icao)
                    .arg(m_estimatedArrival.toString())
                    .arg(m_scheduledArrival.toString())
                    .arg(m_estimatedDeparture.toString())
                    .arg(m_scheduledArrival.toString())
                    .arg(m_uniqueId)
                    .arg(isValid());
}

void FR24Aircraft::update(FR24Aircraft & other, bool isArrival)
{
    if (m_liveStatus != other.getLiveStatus()){
        m_liveStatus = other.getLiveStatus();
    }

    if (m_model != other.getModel()){
        m_model = other.getModel();
    }

    if(m_airline != other.getAirline()){
        m_airline = other.getAirline();
    }

    if(m_callsign != other.getCallsign()){
        m_callsign = other.getCallsign();
    }

    if(m_registration != other.getRegistration()){
        m_registration = other.getRegistration();
    }

    if(isArrival){
        m_estimatedArrival = other.getEstimatedArrival();
        m_scheduledArrival = other.getScheduledArrival();
        m_isArrCmplt = true;
    }else{
        m_estimatedDeparture = other.getEstimatedDeparture();
        m_scheduledDeparture = other.getScheduledDeparture();
        m_isDepCmplt = true;
    }
    m_icao = other.getICAO();
}
