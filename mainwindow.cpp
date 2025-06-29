#include <QTableView>
#include <QFileDialog>
#include <QProgressBar>
#include <QMessageBox>
#include <QXmlStreamWriter>
#include <QJsonArray>
#include <QJsonObject>
#include <QTableView>

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "datamodel.h"
#include "databaseworker.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Инициализация модели
    model = new DataModel(this);
    ui->tableView->setModel(model);

    // Создание отдельного потока для БД
    dbWorker = new DataBaseWorker();
    connect(dbWorker,&DataBaseWorker::progressUpdated,this,&MainWindow::UpdateProgress);
    connect(dbWorker,&DataBaseWorker::importFinished,this,&MainWindow::ImportFinished);
    connect(dbWorker, &DataBaseWorker::dataLoaded, model, &DataModel::SetData);

    // Импорт данных
    connect(ui->btn_import,&QPushButton::clicked,this, &MainWindow::Import);
    // Очистка таблицы
    connect(ui->btn_clear,&QPushButton::clicked,this, &MainWindow::Clear);
    // Обработка нажатия клавиши
   /* connect(ui->tableView, &QTableView::editTriggered, [=](const QModelIndex &index) {
        // Проверяем, нужно ли сохранять данные
        if (index.isValid()) {
            ui->tableView->model()->setData(index, index.data(), Qt::EditRole);
        }
    });*/

    // Создаем контекстное меню
    contextMenu = new QMenu(this);
    // Настройка контекстного меню
    contextMenu->addAction("Удалить", this, SLOT(DeleteRecord()));
    contextMenu->addAction("Редактировать", this, SLOT(EditRecord()));
    contextMenu->addAction("Экспортировать", this, SLOT(ExportRecord()));
    // Настраиваем политику контекстного меню для таблицы
    ui-> tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableView, &QTableView::customContextMenuRequested,
            this, &MainWindow::ShowContextMenu);

    // Создание БД при первом запуске
    if(!QFile::exists("data.db")) {dbWorker->CreateDatabase();}
}

MainWindow::~MainWindow()
{
    delete dbWorker;
    delete contextMenu;
    delete ui;
}
/* \brief Очистка таблицы*/
void MainWindow::Clear()
{
    ui->progressBar->setValue(0);
    model->Clear();
    dbWorker->ClearDatabase();
}
/* \brief Импортировать данные*/
void MainWindow::Import()
{
    ui->progressBar->setValue(0);
    QString dir = QFileDialog::getExistingDirectory(this, "Выбрать папку");
    if (!dir.isEmpty()) {
        dbWorker->StartImport(dir);
    }
}

/*void MainWindow::ShowContextMenu(const QPoint &pos)
{
    tableView->contextMenu()->exec(tableView->mapToGlobal(pos));
}*/

void MainWindow::UpdateProgress(int current, int total, const QStringList &errors)
{
   ui->progressBar->setValue((current * 100) / total);
}
/* \brief Обработка результатов импорта */
void MainWindow::ImportFinished(int totalFiles, QStringList errors) {
    qDebug() << "Импорт завершен!";
    // Загружаем данные из БД в модель
   // QVector<QVector<QVariant>> data = dbWorker->GetAllData();
    //model->SetData(data);
    // Загружаем данные из БД после завершения импорта
    dbWorker->LoadDataFromDB();

    qDebug() << "Всего обработано файлов:" << totalFiles;
    int errorsCount = errors.count();
    qDebug() << "Ошибок:" << errorsCount;

    if (errorsCount > 0) {
        ShowErrorDialog(errors);
    } else {
        ShowSuccessMessage();
    }
}
/* \brief Вывод информации об ошибках */
void MainWindow::ShowErrorDialog(QStringList errors) {
    QString strErrs = "";
    int errorsCount = errors.count();
    for (int i=0;i<errorsCount;i++)
        strErrs+="\n Ошибка в файле:"+errors[i];

    QMessageBox::warning(
        nullptr,
        "Ошибки при импорте",
        QString("Обнаружено ошибок при импорте: %1. \n %2").arg(errorsCount)
        .arg(strErrs)
        );
}

/* \brief Вывод информации об успехе импорта */
void MainWindow::ShowSuccessMessage() {
    QMessageBox::information(
        nullptr,
        "Успех",
        "Импорт успешно завершен!"
        );
}

/* \brief Показать контекстное меню в позиции курсора*/
void MainWindow::ShowContextMenu(const QPoint &pos)
{
 // ui->tableView->contextMenu()->exec(ui->tableView->mapToGlobal(pos));
    // Получаем индекс ячейки под курсором
    QModelIndex index =ui-> tableView->indexAt(pos);
    if (index.isValid()) {
        // Показываем меню в позиции курсора
        contextMenu->exec(ui->tableView->mapToGlobal(pos));
    }
}
/* \brief Удалить строку из модели */
void MainWindow::DeleteRecord() {
    // Получаем текущий выбранный индекс
    QModelIndex index = ui->tableView->currentIndex();
    if (index.isValid()) {
        // Удаляем строку из модели
       // QAbstractItemModel *model =ui-> tableView->model();
       //model->removeRow(index.row());
       ui->tableView->model()->removeRow(index.row());
       //model-> RemoveRows(index.row(),ui->tableView->);
    }
}
/* \brief Редактировать ячейки */
void MainWindow::EditRecord() {
    // Получаем текущий выбранный индекс
    QModelIndex index = ui->tableView->currentIndex();
    if (index.isValid()) {
        // Начинаем редактирование ячейки
       ui->tableView->edit(index);
    }
}
/* \brief Сохранить данные в JSON/XML-файл */
void MainWindow::ExportRecord() {
    // Получаем текущий выбранный индекс
    QModelIndex index = ui->tableView->currentIndex();
    if (index.isValid()) {
        // Открываем диалог сохранения файла
        QString fileName = QFileDialog::getSaveFileName(
            this, "Сохранить файл", "", "JSON файлы (*.json);;XML файлы (*.xml)");
        if (!fileName.isEmpty()) {
            QAbstractItemModel *model = ui->tableView->model();
            if (fileName.endsWith(".json")) {
                ExportToJson(model, fileName);
            } else if (fileName.endsWith(".xml")) {
                ExportToXml(model, fileName);
            }
        }
    }
}
/* \brief Экспорт в XML */
void MainWindow::ExportToXml(QAbstractItemModel *model, const QString &fileName) {
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        QXmlStreamWriter xmlWriter(&file);
        xmlWriter.setAutoFormatting(true);
        xmlWriter.writeStartDocument();
        xmlWriter.writeStartElement("table");

        for (int row = 0; row < model->rowCount(); ++row) {
            xmlWriter.writeStartElement("row");
            for (int col = 0; col < model->columnCount(); ++col) {
                QModelIndex index = model->index(row, col);
                xmlWriter.writeStartElement(model->headerData(col, Qt::Horizontal).toString());
                xmlWriter.writeCharacters(index.data().toString());
                xmlWriter.writeEndElement();  // закрываем тег столбца
            }
            xmlWriter.writeEndElement();  // закрываем тег строки
        }

        xmlWriter.writeEndElement();  // закрываем тег table
        xmlWriter.writeEndDocument();
        file.close();
    }
}
/* \brief Вывод сообщений об ошибках экспорта */
void MainWindow::ShowErrorMessage(const QString& message) {
    QMessageBox::critical(this, "Ошибка", message);
}

/* \brief Экспорт в JSON с обработкой ошибок */
void MainWindow::ExportToJson(QAbstractItemModel *model, const QString &fileName) {
    try {
        QJsonArray jsonArray;
        for (int row = 0; row < model->rowCount(); ++row) {
            QJsonObject rowObject;
            for (int col = 0; col < model->columnCount(); ++col) {
                QModelIndex index = model->index(row, col);
                rowObject[model->headerData(col, Qt::Horizontal).toString()] =
                    index.data().toString();
            }
            jsonArray.append(rowObject);
        }

        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            ShowErrorMessage("Не удалось открыть файл для записи");
            return;
        }

        QJsonDocument doc(jsonArray);
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    } catch (...) {
        ShowErrorMessage("Произошла ошибка при экспорте данных");
    }
}
