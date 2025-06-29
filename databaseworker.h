#ifndef DATABASEWORKER_H
#define DATABASEWORKER_H

#include <QObject>
#include <QThread>
#include <QSqlDatabase>
#include <QStringList>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QDomDocument>

class DataBaseWorker : public QObject
{
    Q_OBJECT
public:
    explicit DataBaseWorker(QObject *parent = nullptr);
    ~DataBaseWorker();

    // Метод для загрузки данных из БД в модель
  //  void LoadDataToModel(DataModel *model);
    // Метод для обновления данных в БД при изменении модели
  //  void UpdateData(const QVector<QVector<QVariant>> &newData);

    // Чтение данных из БД
    QVector<QVector<QVariant>> GetAllData();
    // Метод для загрузки данных из БД в модель
    void LoadDataFromDB();

public slots:
    void StartImport(const QString &folderPath);
    void CreateDatabase();
    void ClearDatabase();
    // Парсинг файлв json, в случае успеха вернет true, иначе false
    bool ParseJsonFile(const QString &filePath);
    // Парсинг файлв xml, в случае успеха вернет true, иначе false
    bool ParseXmlFile(const QString &filePath);

signals:
    void progressUpdated(int current, int total, const QStringList &errors);
    void importFinished(int totalFiles, QStringList errors);
    void dataLoaded(const QVector<QVector<QVariant>> &data);

private:
    QSqlDatabase db;
    QThread workerThread;
    QStringList errors;
    int currentFile;
    int totalFiles;

    void CreateTable();
    void InsertData(const QVariantMap &data);
    QVariantMap ParseJson(const QByteArray &data);
    QVariantMap ParseXml(const QDomElement &data);

};

#endif // DATABASEWORKER_H
