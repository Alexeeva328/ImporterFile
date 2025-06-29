#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableView>

#include "datamodel.h"
#include "databaseworker.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
class DataModel;
class DataBaseWorker;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    /* \brief Экспорт в XML */
    void ExportToXml(QAbstractItemModel *model, const QString &fileName);
    /* \brief Экспорт в JSON с обработкой ошибок */
    void ExportToJson(QAbstractItemModel *model, const QString &fileName);

private slots:
    /* \brief Импорт файлов*/
    void Import();
    /* \brief Очистка таблицы */
    void Clear();
    /* \brief Обновление индикатора загрузки */
    void UpdateProgress(int current, int total,const QStringList &errors);
    /* \brief Обработка результатов импорта */
    void ImportFinished(int totalFiles, QStringList errors);
    /* \brief Вывод информации об ошибках импорта */
    void ShowErrorDialog(QStringList errors);
    /* \brief Вывод информации об успехе импорта */
    void ShowSuccessMessage();
    /* \brief Вывод сообщений об ошибках экспорта */
    void ShowErrorMessage(const QString& message);

    /* \brief Показать контекстное меню в позиции курсора*/
    void ShowContextMenu(const QPoint &pos);
    /* \brief Удалить строку из модели */
    void DeleteRecord();
    /* \brief Редактировать ячейки */
    void EditRecord();
    /* \brief Сохранить данные в JSON/XML-файл */
    void ExportRecord() ;

private:
    Ui::MainWindow *ui = nullptr;
    DataModel *model = nullptr;
    DataBaseWorker *dbWorker = nullptr;
    QMenu *contextMenu = nullptr;
};
#endif // MAINWINDOW_H
