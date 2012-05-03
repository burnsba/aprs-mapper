#ifndef DETAILEDPACKETSINFO_H
#define DETAILEDPACKETSINFO_H

#include <QDialog>
#include "database.h"

namespace Ui {
    class detailedPacketsInfo;
}

class detailedPacketsInfo : public QDialog
{
    Q_OBJECT

public:
    explicit detailedPacketsInfo(QWidget *parent = 0);
    detailedPacketsInfo(QString callsign, Database *db_ref);
    ~detailedPacketsInfo();

private:
    Ui::detailedPacketsInfo *ui;
    Database *db;
};

#endif // DETAILEDPACKETSINFO_H
