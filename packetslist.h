#ifndef PACKETSLIST_H
#define PACKETSLIST_H

#include <QDialog>
#include <QQueue>
#include <QAbstractButton>
#include "database.h"

namespace Ui {
    class packetsList;
}

class packetsList : public QDialog
{
    Q_OBJECT

public:
    explicit packetsList(QWidget *parent = 0);
    packetsList(QQueue<QString> packets, Database *db);
    ~packetsList();

private:
    Ui::packetsList *ui;
    void closeEvent(QCloseEvent *);

    Database *db; // need to propogate this pointer on down to the detailed packet info dialog

private slots:
    void on_packetsList_rejected();
    void on_packetsList_finished(int result);
    void on_buttonBox_rejected();
    void on_buttonBox_clicked(QAbstractButton* button);
    void on_buttonBox_accepted();
    void reject();

signals:
    void closingNow();
};

#endif // PACKETSLIST_H
