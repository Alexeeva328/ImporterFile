#include "datamodel.h"

DataModel::DataModel(QObject *parent):QAbstractTableModel(parent) {
// Инициализация заголовков
    m_headers<<"texteditor"<<"fileformats"<<"encoding"<<"hasintellisense"<<"hasintellisense"<<"cancompile";
}
DataModel::~DataModel()
{
    m_data.clear();
    m_headers.clear();
}

/*\brief Узнать количество строк */
int DataModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_data.size();
}
/*\brief Узнать количество столбцов */
int DataModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_headers.size();
}
/*\brief Получить данные */
QVariant DataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    // qDebug() << "Запрос данных для строки:" << index.row()<< "столбец:" << index.column();

    if (role == Qt::DisplayRole) {
        return m_data[index.row()][index.column()];
    }
    return QVariant();
}
/*\brief Получить заголовки столбцов */
QVariant DataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return m_headers[section];
    }
    return QVariant();
}
/*\brief Установить данные */
void DataModel::SetData(const QVector<QVector<QVariant>> &data_)
{
    beginResetModel();
    m_data = data_;
    endResetModel();
}
/*bool DataModel::SetData(const QModelIndex &index, const QVariant &value, int role) {
    if (role != Qt::EditRole)
        return false;
    if (!index.isValid())
        return false;

    m_data[index.row()][index.column()] = value;
    emit dataChanged(index, index, {role});
    return true;
}*/
bool DataModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    // 1. Проверка валидности индекса
    if (!index.isValid()) {
        qDebug() << "Неверный индекс";
        return false;
    }

    // 2. Проверка границ данных
    if (index.row() >= m_data.size() || index.column() >= m_data[0].size()) {
        qDebug() << "Выход за пределы данных";
        return false;
    }

    // 3. Обработка различных ролей
    switch (role) {
    case Qt::EditRole:
        // 4. Сохранение данных
        m_data[index.row()][index.column()] = value;
        break;
    case Qt::DisplayRole:
        // Если нужно обрабатывать отображение
        return false;
    default:
        return false;
    }

    // 5. Уведомление о изменении
    emit dataChanged(index, index, {role});
    return true;
}

/*\brief Очистить модель */
void DataModel::Clear()
{
    beginResetModel();
    m_data.clear();
    endResetModel();
}
Qt::ItemFlags DataModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

/*\brief Удаление строки */
bool DataModel::removeRows(int row, int count, const QModelIndex &parent) {
    beginRemoveRows(parent, row, row + count - 1);
    for (int i = 0; i < count; ++i) {
        m_data.remove(row);
    }
    endRemoveRows();
    return true;
}
