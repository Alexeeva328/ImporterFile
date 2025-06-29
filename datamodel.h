#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <QAbstractTableModel>
#include <QVariant>
#include <QVector>

class DataModel:public QAbstractTableModel
{
    Q_OBJECT

public:
    DataModel(QObject *parent = nullptr);
    ~DataModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void SetData(const QVector<QVector<QVariant>>&data_);
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    void Clear();
    /*\brief Удаление строки */
    bool removeRows(int row, int count, const QModelIndex &parent);
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    QVector<QVector<QVariant>> m_data;
    QStringList m_headers;

};

#endif // DATAMODEL_H
