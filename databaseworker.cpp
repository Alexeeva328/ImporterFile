#include <QSqlQuery>
#include <QSqlError>
#include <QJsonObject>
#include <QJsonArray>

#include "databaseworker.h"

DataBaseWorker::DataBaseWorker(QObject *parent)
    : QObject(parent),
    currentFile(0),
    totalFiles(0)
{
    moveToThread(&workerThread);
    workerThread.start();
    CreateDatabase();
}

DataBaseWorker::~DataBaseWorker()
{
    workerThread.quit();
    workerThread.wait();
}
/*\brief Создание БД*/
void DataBaseWorker::CreateDatabase()
{
    QString dbPath = "data.db";
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);
    // Проверка существования БД
    if (!QFile::exists(dbPath))    
        qDebug()<< "БД не существует, создаем новую";

    if (!db.open()) {
        qDebug() << "Ошибка открытия БД: " << db.lastError();
        return;
    }
    CreateTable();

}
/*\brief Создание таблицы*/
void DataBaseWorker::CreateTable()
{
    QString query = "CREATE TABLE IF NOT EXISTS data ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "column1 TEXT, "
                    "column2 TEXT, "
                    "column3 TEXT, "
                    "column4 TEXT, "
                    "column5 TEXT, "
                    "column6 TEXT)";

    QSqlQuery qry;
    if (!qry.exec(query))
        qDebug() << "Ошибка создания таблицы: " << qry.lastError();
}
/*\brief Импорт из файлов json и xml*/
void DataBaseWorker::StartImport(const QString &folderPath)
{
    errors.clear();
    currentFile = 0;

    QDir dir(folderPath);

    if (!dir.exists()) {
        errors << "Указанная папка не существует";
        emit importFinished(totalFiles, errors);
        return;
    }

    totalFiles = dir.entryList(QStringList() << "*.json" << "*.xml").size();

    foreach(const QString &fileName, dir.entryList(QStringList() << "*.json" << "*.xml"))
    {
        QString filePath = dir.absoluteFilePath(fileName);

        if (!QFile::exists(filePath)) {
            errors << "Файл не существует: " + filePath;
            continue;
        }

        if (fileName.endsWith(".json")) {
            if (!ParseJsonFile(filePath)) {
                errors << "Ошибка парсинга JSON файла: " + filePath;
            }
        }
        else if (fileName.endsWith(".xml")) {
            if (!ParseXmlFile(filePath)) {
                errors << "Ошибка парсинга XML файла: " + filePath;
            }
        }
    }

    emit importFinished(totalFiles, errors);
}

/*\brief Парсинг данных из json файла*/
bool DataBaseWorker::ParseJsonFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        errors << "Ошибка чтения файла " + filePath;
        emit progressUpdated(++currentFile, totalFiles, errors);
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QVariantMap map = ParseJson(data);
    if (!map.isEmpty()) {
        InsertData(map);
    } else {
        errors << "Ошибка парсинга JSON в файле " + filePath;
    }
    emit progressUpdated(++currentFile, totalFiles, errors);
    return true;
}
/*\brief Парсинг данных из xml файла*/
bool DataBaseWorker::ParseXmlFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        errors << "Ошибка чтения файла " + filePath;
        emit progressUpdated(++currentFile, totalFiles, errors);
        return false;
    }
    QByteArray data = file.readAll();
    file.close();

    QDomDocument doc;
    if (doc.setContent(data)) {
        QDomElement root = doc.documentElement();

        if (root.tagName() != "root") {
            qDebug() << "Неверный корневой элемент XML";
            return false;
        }

        // Создаем карту для хранения данных
        QVariantMap map = ParseXml(root);
        if (!map.isEmpty()) {
            InsertData(map);
        } else {
            errors << "Ошибка парсинга XML в файле " + filePath;
        }
    } else {
        errors << "Ошибка загрузки XML файла " + filePath;
    }

    emit progressUpdated(++currentFile, totalFiles, errors);
    return true;
}

QVariantMap DataBaseWorker::ParseJson(const QByteArray &data)
{
    QVariantMap result;
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isNull()) {
        qDebug() << "Неверный формат JSON";
        return result;
    }

    if (doc.isObject()) {
        // Получаем корневой объект
        QJsonObject rootObj = doc.object().value("root").toObject();
        // Проверяем наличие всех необходимых полей
        if (!rootObj.contains("texteditor") ||
            !rootObj.contains("fileformats") ||
            !rootObj.contains("encoding") ||
            !rootObj.contains("hasintellisense") ||
            !rootObj.contains("hasplugins") ||
            !rootObj.contains("cancompile")) {
            qDebug() << "Отсутствуют обязательные поля в JSON";
            return result;
        }

        // Парсим значения с учетом их типов
        result.insert("column1", rootObj.value("texteditor").toString());
        result.insert("column2", rootObj.value("fileformats").toString());

        // Обработка массива encoding
        QJsonArray encodingArray = rootObj.value("encoding").toArray();
        QStringList encodings;
        for (const QJsonValue &value : encodingArray) {
            encodings << value.toString();
        }
        result.insert("column3", encodings.join(',')); // объединяем в строку через запятую

        // Булевы значения
        result.insert("column4", rootObj.value("hasintellisense").toBool());
        result.insert("column5", rootObj.value("hasplugins").toBool());

        // cancompile может быть строкой "false", обрабатываем это
        QString canCompileStr = rootObj.value("cancompile").toString();
        result.insert("column6", (canCompileStr == "true"));
    }

    return result;
}
QVariantMap DataBaseWorker::ParseXml(const QDomElement &element)
{
    QVariantMap result;
   // qDebug()<<QString("elementsByTagName firstChild = %1").arg(element.elementsByTagName("texteditor").at(0).firstChild().nodeValue());

    // Используем elementsByTagName для получения значений
    result.insert("column1",
                  element.elementsByTagName("texteditor").at(0).firstChild().nodeValue());

    result.insert("column2",
                  element.elementsByTagName("fileformats").at(0).firstChild().nodeValue());

    result.insert("column3",
                  element.elementsByTagName("encoding").at(0).firstChild().nodeValue());

    // Для булевых значений добавляем преобразование
    QString intellisenseStr =
        element.elementsByTagName("hasintellisense").at(0).firstChild().nodeValue();
    result.insert("column4", (intellisenseStr == "true"));

    QString pluginsStr =
        element.elementsByTagName("hasplugins").at(0).firstChild().nodeValue();
    result.insert("column5", (pluginsStr == "true"));

    QString compileStr =
        element.elementsByTagName("cancompile").at(0).firstChild().nodeValue();
    result.insert("column6", (compileStr == "true"));

    return result;
}
void DataBaseWorker::InsertData(const QVariantMap &data)
{
    QSqlQuery query;
    query.prepare("INSERT INTO data (column1, column2, column3, column4, column5, column6) "
                  "VALUES (:column1, :column2, :column3, :column4, :column5, :column6)");

    query.bindValue(":column1", data.value("column1"));
    query.bindValue(":column2", data.value("column2"));
    query.bindValue(":column3", data.value("column3"));
    query.bindValue(":column4", data.value("column4"));
    query.bindValue(":column5", data.value("column5"));
    query.bindValue(":column6", data.value("column6"));

    if (!query.exec()) {
        qDebug() << "Ошибка вставки данных: " << query.lastError();
    }
}
void DataBaseWorker::ClearDatabase()
{
    QSqlQuery query;
    query.exec("DELETE FROM data");
}

/* \brief Чтение данных из БД */
QVector<QVector<QVariant>> DataBaseWorker::GetAllData() {
    QVector<QVector<QVariant>> result;
    QSqlQuery query("SELECT * FROM data");

    while (query.next()) {
        QVector<QVariant> row;
        row << query.value(1) << query.value(2) << query.value(3)
            << query.value(4) << query.value(5) << query.value(6);
        result.append(row);
    }

    return result;
}
/*\brief Метод для загрузки данных из БД в модель*/
void DataBaseWorker::LoadDataFromDB() {
    if (!db.isOpen()) {
        qDebug() << "БД не открыта";
        return;
    }

    QSqlQuery query("SELECT * FROM data");
    QVector<QVector<QVariant>> data;

    while (query.next()) {
        QVector<QVariant> row;
        row << query.value(1) << query.value(2) << query.value(3)
            << query.value(4) << query.value(5) << query.value(6);
        data.append(row);
    }

    emit dataLoaded(data);
}
/*\brief Метод для обновления данных в БД при изменении модели*/
/*void DataBaseWorker::UpdateData(const QVector<QVector<QVariant>> &newData)
{
    QSqlQuery query;
    query.exec("DELETE FROM data");

    for (const QVector<QVariant> &row : newData) {
        query.prepare("INSERT INTO data (column1, column2, column3) "
                      "VALUES (:column1, :column2, :column3)");

        query.bindValue(":column1", row[0]);
        query.bindValue(":column2", row[1]);
        query.bindValue(":column3", row[2]);

        if (!query.exec()) {
            qDebug() << "Ошибка обновления данных: " << query.lastError();
        }
    }
}*/
