#include "fr24proxymodel.h"
#include "fr24model.h"

FR24ProxyModel::FR24ProxyModel(QObject *parent)
    : QSortFilterProxyModel{parent}
{

}

static bool filterAcceptColumn(const QString filter, int row, FR24Model::Column col, FR24Model* model, const QModelIndex &parent)
{
    if(!filter.isEmpty()){
        QString tmpFilter = filter;
        QModelIndex idx = model->index(row, col, parent);
        bool negative = filter.at(0) == '!';
        if(negative){
            tmpFilter = tmpFilter.right(1);
        }
        auto contains = (model->data(idx).toString().contains(tmpFilter, Qt::CaseInsensitive));
        if((!negative && !contains) || (negative && contains)){
            return false;
        }
    }
    return true;
}

bool FR24ProxyModel::filterAcceptsRow(int row, const QModelIndex &parent) const
{
    FR24Model* mdl = dynamic_cast<FR24Model*>(sourceModel());

    if (!filterAcceptColumn(m_registrationFilter, row, FR24Model::FM_REGISTRATION, mdl, parent )){
        return false;
    }

    return true;
}


void FR24ProxyModel::setRegistrationFilter(QString filter)
{
    if (m_registrationFilter != filter){
        m_registrationFilter = filter;
        invalidateFilter();
    }
}

void FR24ProxyModel::resetFilter()
{
    m_registrationFilter = "";
    invalidateFilter();
}
