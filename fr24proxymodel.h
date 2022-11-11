#ifndef FR24PROXYMODEL_H
#define FR24PROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QObject>

class FR24ProxyModel : public QSortFilterProxyModel
{
public:
    explicit FR24ProxyModel(QObject *parent = nullptr);
    bool filterAcceptsRow(int source_row,
                          const QModelIndex &source_parent) const override;
public slots:
    void setRegistrationFilter(QString filter);
    void resetFilter();

private:
    QString m_registrationFilter;
};


#endif // FR24PROXYMODEL_H
