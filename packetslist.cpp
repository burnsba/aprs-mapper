#include "packetslist.h"
#include "ui_packetslist.h"

#include "detailedpacketsinfo.h"

packetsList::packetsList(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::packetsList)
{
    ui->setupUi(this);

    this->setWindowTitle("Select packet to view more info");
}

packetsList::packetsList(QQueue<QString> packets, Database *db_ref) :
    ui(new Ui::packetsList)
{
    ui->setupUi(this);

    this->setWindowTitle("Show details for...");

    foreach(QString s, packets)
    {
        ui->listPackets->addItem(s);
    }

    db = db_ref;
}

packetsList::~packetsList()
{
    emit closingNow();
    delete ui;
}

void packetsList::on_buttonBox_accepted()
{
    if (!ui->listPackets->currentItem())
        return;

    QString callsign = ui->listPackets->currentItem()->text();
    if (callsign != "")
    {
        QQueue<Packet> packets = db->GetPacketInfo(callsign);

        int count = packets.count();
        if (count > 0)
        {
            detailedPacketsInfo *d = new detailedPacketsInfo(callsign, db);
            d->show();
        }
    }
    emit closingNow();
    this->close();
}

void packetsList::closeEvent(QCloseEvent *)
{
    emit closingNow();
}

void packetsList::on_buttonBox_clicked(QAbstractButton* button)
{
    emit closingNow();
}

void packetsList::on_buttonBox_rejected()
{
    // if the user hits cancel
    emit closingNow();
    this->close();
}

void packetsList::on_packetsList_finished(int result)
{
    emit closingNow();
}

void packetsList::on_packetsList_rejected()
{
    emit closingNow();
}

void packetsList::reject()
{
    // if the user presses escape
    emit closingNow();
    this->close();
}
