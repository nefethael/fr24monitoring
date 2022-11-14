#ifndef FR24MODEL_H
#define FR24MODEL_H

#include <QAbstractTableModel>
#include <QObject>

#include "fr24aircraft.h"

class FR24Model : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Column{
        FM_AIRLINE = 0,
        FM_MODEL,
        FM_REGISTRATION,
        FM_CALLSIGN,
        FM_LIVESTATUS,
        FM_ARRIVALTIME,
        FM_DEPARTURETIME,
        FM_ICAO,
        FM_NEARESTDATE,
        FM_UNIQUEID,
    };

    FR24Model(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void refreshCraft(QVector<FR24Aircraft>& c);
    void clearCraft();

private:
    QVector<FR24Aircraft> m_fr24Data;
};

#endif // FR24MODEL_H
