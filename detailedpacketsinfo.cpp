#include "detailedpacketsinfo.h"
#include "ui_detailedpacketsinfo.h"

#include <QQueue>
#include "packettypes.h"
#include "profile.h"

detailedPacketsInfo::detailedPacketsInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::detailedPacketsInfo)
{
    ui->setupUi(this);
}

detailedPacketsInfo::~detailedPacketsInfo()
{
    delete ui;
}


detailedPacketsInfo::detailedPacketsInfo(QString callsign, Database *db_ref) :
        ui(new Ui::detailedPacketsInfo)
{

    db = db_ref;
    QQueue<Packet> packets = db->GetPacketInfo(callsign);

    int count = packets.count();
    if (count <= 0)
    {
        this->close();
        return;
    }
    else
    {
        ui->setupUi(this);
    }

    ui->tableDetailedInfo->setHorizontalHeaderLabels(QStringList() << tr("Timestamp") <<
                                                     tr("Destination") <<
                                                     tr("Data") <<
                                                     tr("Latitude") <<
                                                     tr("Longitude") <<
                                                     tr(Profile::isUnitsEnglish() == true ? "Altitude (ft)" : "Altitude (m)") <<
                                                     tr("Path") <<
                                                     tr("Payload") <<
                                                     tr("Message") <<
                                                     tr("Course") <<
                                                     tr(Profile::isUnitsEnglish() == true ? "Speed (mi/hr)" : "Speed (m/s)") <<
                                                     tr(Profile::isUnitsEnglish() == true ? "Temperature (° F)" : "Temperature (° C)") <<
                                                     tr("Pressure") <<
                                                     tr(Profile::isUnitsEnglish() == true ? "Wind Speed (mi/hr)" : "Wind Speed (m/s)") <<
                                                     tr("Wind direction") <<
                                                     tr(Profile::isUnitsEnglish() == true ? "Wind gust (mi/hr)" : "Wind gust (m/s)") <<
                                                     tr(Profile::isUnitsEnglish() == true ? "Rain 1hr (in)" : "Rain 1hr (mm)") <<
                                                     tr(Profile::isUnitsEnglish() == true ? "Rain 24hr (in)" : "Rain 24hr (mm)") <<
                                                     tr(Profile::isUnitsEnglish() == true ? "Rain midnight (in)" : "Rain midnight (mm)") <<
                                                     tr(Profile::isUnitsEnglish() == true ? "Snow 24hr (in)" : "Snow 24hr (mm)") <<
                                                     tr("Raincount") <<
                                                     tr("Humidity") <<
                                                     tr("Luminosity")
                                                     );


    Packet latest = packets.last();

    QString text_to_show = "";
    text_to_show += "Item: " + latest.source + QChar(13) + QChar(10);

    if (latest.timestamp != QDateTime())
    {
        text_to_show += "Timestamp: " + latest.timestamp.toString("yyyy-MM-dd, HH:mm:ss") + QChar(13) + QChar(10);
        int difference = QDateTime::currentDateTime().secsTo(latest.timestamp);
        difference = abs(difference);
        text_to_show += "(";
        text_to_show += QString::number(difference / 86400) + " days, ";
        text_to_show += QString::number((difference / 3600) % 24) + ":";
        text_to_show += QString::number((difference / 60) % 60) + ":";
        text_to_show += QString::number(difference % 60) + " ago)" + QChar(13) + QChar(10);
    }


    if  (latest.altitude != 0 && latest.altitude == latest.altitude) // (NaN == NaN) returns false
        text_to_show += "Altitude: " + (Profile::isUnitsEnglish() == true ? QString::number(latest.altitude) + " ft" : QString::number(latest.altitude * 0.3048) + " m") + QChar(13) + QChar(10);
    //if (latest.getSpeed()!= 0)
        text_to_show += "Speed: " + (Profile::isUnitsEnglish() == true ? QString::number(latest.speed * 2.23693629) + " mi/hr" : QString::number(latest.speed) + " m/s") + QChar(13) + QChar(10);
    //if (latest.course != 0)
        text_to_show += "Course: " + QString::number(latest.course) + "° (" + bearingToCompassPoint(latest.course) + ")" + QChar(13) + QChar(10);
    if (latest.source != "")
        text_to_show += "Source: " + latest.source + QChar(13) + QChar(10);
    if (latest.payload != "")
        text_to_show += "Payload: " + latest.payload + QChar(13) + QChar(10);
    if (latest.destination != "")
        text_to_show += "Destination: " + latest.destination + QChar(13) + QChar(10);
    if (latest.path != "")
        text_to_show += "Path: " + latest.path + QChar(13) + QChar(10);
    if (latest.message != "")
        text_to_show += "Message: " + latest.message + QChar(13) + QChar(10);
    if (latest.temp == latest.temp)
        text_to_show += "Temperature: " + (Profile::isUnitsEnglish() == true ? QString::number((latest.temp * 9/5) + 32) + "° F" : QString::number(latest.temp) + "° C") + QChar(13) + QChar(10);
    if (latest.pressure == latest.pressure)
        text_to_show += "Pressure: " + QString::number(latest.pressure) + " mbar" + QChar(13) + QChar(10);
    if (latest.wind_speed == latest.wind_speed)
        text_to_show += "Wind speed: " + (Profile::isUnitsEnglish() == true ? QString::number(latest.wind_speed * 2.23693629) + " mi/hr" : QString::number(latest.wind_speed) + " m/s") + QChar(13) + QChar(10);
    if (latest.wind_direction > 0)
        text_to_show += "Wind direction: " + QString::number(latest.wind_direction) + "°" + QChar(13) + QChar(10);
    if (latest.wind_gust == latest.wind_gust)
        text_to_show += "Wind gust: " + (Profile::isUnitsEnglish() == true ? QString::number(latest.wind_gust * 2.23693629) + " mi/hr" : QString::number(latest.wind_gust) + " m/s") + QChar(13) + QChar(10);
    if (latest.rain_1h == latest.rain_1h)
        text_to_show += "Rain (1hr): " + (Profile::isUnitsEnglish() == true ? QString::number(latest.rain_1h * 0.0393700787) + " in" : QString::number(latest.rain_1h) + " mm") + QChar(13) + QChar(10);
    if (latest.rain_24h == latest.rain_24h)
        text_to_show += "Rain (24hr): " + (Profile::isUnitsEnglish() == true ? QString::number(latest.rain_24h * 0.0393700787) + " in" : QString::number(latest.rain_24h) + " mm") + QChar(13) + QChar(10);
    if (latest.rain_midnight == latest.rain_midnight)
        text_to_show += "Rain (midnight): " + (Profile::isUnitsEnglish() == true ? QString::number(latest.rain_midnight * 0.0393700787) + " in" : QString::number(latest.rain_midnight) + " mm") + QChar(13) + QChar(10);
    if (latest.snow_24h == latest.snow_24h)
        text_to_show += "Snow (24hr): " + (Profile::isUnitsEnglish() == true ? QString::number(latest.snow_24h * 0.0393700787) + " in" : QString::number(latest.snow_24h) + " mm") + QChar(13) + QChar(10);
    if (latest.raincount> 0)
        text_to_show += "Raincount: " + QString::number(latest.raincount) + QChar(13) + QChar(10);
    if (latest.humidity > 0)
        text_to_show += "Humidity: " + QString::number(latest.humidity) + QChar(13) + QChar(10);

    ui->textLatestInfo->setText(text_to_show);

    int row_count = 0;
    if (count > 0)
    {
        QTableWidgetItem *i;
        foreach(Packet p, packets)
        {
            int col_count = 0;
            ui->tableDetailedInfo->insertRow(row_count);

            i = new QTableWidgetItem();
            i->setText(p.timestamp.toString("yyyy-MM-dd HH:mm:ss"));
            ui->tableDetailedInfo->setItem(row_count, col_count, i);
            col_count++;

            i = new QTableWidgetItem();
            i->setText(p.destination);
            ui->tableDetailedInfo->setItem(row_count, col_count, i);
            col_count++;

            i = new QTableWidgetItem();
            i->setText(p.data);
            ui->tableDetailedInfo->setItem(row_count, col_count, i);
            col_count++;

            i = new QTableWidgetItem();
            i->setData(Qt::DisplayRole, p.latitude);
            ui->tableDetailedInfo->setItem(row_count, col_count, i);
            col_count++;

            i = new QTableWidgetItem();
            i->setData(Qt::DisplayRole, p.longitude);
            ui->tableDetailedInfo->setItem(row_count, col_count, i);
            col_count++;

            i = new QTableWidgetItem();
            i->setData(Qt::DisplayRole, (Profile::isUnitsEnglish() == true ? p.altitude : p.altitude * 0.3048));
            ui->tableDetailedInfo->setItem(row_count, col_count, i);
            col_count++;

            i = new QTableWidgetItem();
            i->setText(p.path);
            ui->tableDetailedInfo->setItem(row_count, col_count, i);
            col_count++;

            i = new QTableWidgetItem();
            i->setText(p.payload);
            ui->tableDetailedInfo->setItem(row_count, col_count, i);
            col_count++;

            i = new QTableWidgetItem();
            i->setText(p.message);
            ui->tableDetailedInfo->setItem(row_count, col_count, i);
            col_count++;

            i = new QTableWidgetItem();
            i->setData(Qt::DisplayRole, p.course);
            ui->tableDetailedInfo->setItem(row_count, col_count, i);
            col_count++;

            i = new QTableWidgetItem();
            i->setData(Qt::DisplayRole, (Profile::isUnitsEnglish() == true ? p.speed * 2.23693629 : p.speed));
            ui->tableDetailedInfo->setItem(row_count, col_count, i);
            col_count++;

            i = new QTableWidgetItem();
            i->setData(Qt::DisplayRole, (Profile::isUnitsEnglish() == true ? (p.temp * 9/5) + 32 : p.temp));
            ui->tableDetailedInfo->setItem(row_count, col_count, i);
            col_count++;

            i = new QTableWidgetItem();
            i->setData(Qt::DisplayRole, p.pressure);
            ui->tableDetailedInfo->setItem(row_count, col_count, i);
            col_count++;

            i = new QTableWidgetItem();
            i->setData(Qt::DisplayRole, (Profile::isUnitsEnglish() == true ? p.wind_speed * 2.23693629 : p.wind_speed));
            ui->tableDetailedInfo->setItem(row_count, col_count, i);
            col_count++;

            i = new QTableWidgetItem();
            i->setData(Qt::DisplayRole, p.wind_direction);
            ui->tableDetailedInfo->setItem(row_count, col_count, i);
            col_count++;

            i = new QTableWidgetItem();
            i->setData(Qt::DisplayRole, (Profile::isUnitsEnglish() == true ? p.wind_gust * 2.23693629 : p.wind_gust));
            ui->tableDetailedInfo->setItem(row_count, col_count, i);
            col_count++;

            i = new QTableWidgetItem();
            i->setData(Qt::DisplayRole, (Profile::isUnitsEnglish() == true ? p.rain_1h * 0.0393700787 : p.rain_1h));
            ui->tableDetailedInfo->setItem(row_count, col_count, i);
            col_count++;

            i = new QTableWidgetItem();
            i->setData(Qt::DisplayRole, (Profile::isUnitsEnglish() == true ? p.rain_24h * 0.0393700787 : p.rain_24h));
            ui->tableDetailedInfo->setItem(row_count, col_count, i);
            col_count++;

            i = new QTableWidgetItem();
            i->setData(Qt::DisplayRole, (Profile::isUnitsEnglish() == true ? p.rain_midnight * 0.0393700787 : p.rain_midnight));
            ui->tableDetailedInfo->setItem(row_count, col_count, i);
            col_count++;

            i = new QTableWidgetItem();
            i->setData(Qt::DisplayRole, (Profile::isUnitsEnglish() == true ? p.snow_24h * 0.0393700787 : p.snow_24h));
            ui->tableDetailedInfo->setItem(row_count, col_count, i);
            col_count++;

            i = new QTableWidgetItem();
            i->setData(Qt::DisplayRole, p.raincount);
            ui->tableDetailedInfo->setItem(row_count, col_count, i);
            col_count++;

            i = new QTableWidgetItem();
            i->setData(Qt::DisplayRole, p.humidity);
            ui->tableDetailedInfo->setItem(row_count, col_count, i);
            col_count++;

            i = new QTableWidgetItem();
            i->setData(Qt::DisplayRole, p.luminosity);
            ui->tableDetailedInfo->setItem(row_count, col_count, i);
            col_count++;

            row_count++;
        }
    }

    //  now go through and make things pretty
    for (int i = 0; i < ui->tableDetailedInfo->columnCount() ; i++) // 22 columns
    {
        ui->tableDetailedInfo->horizontalHeader()->resizeSection(i, 80);
    }

    // the timestamp column needs extra space
    ui->tableDetailedInfo->horizontalHeader()->resizeSection(0, 140);
    // so do a few other columns
    ui->tableDetailedInfo->horizontalHeader()->resizeSection(22, 110);
    ui->tableDetailedInfo->horizontalHeader()->resizeSection(21, 110);
    ui->tableDetailedInfo->horizontalHeader()->resizeSection(20, 110);
    ui->tableDetailedInfo->horizontalHeader()->resizeSection(19, 110);
    ui->tableDetailedInfo->horizontalHeader()->resizeSection(18, 110);
    ui->tableDetailedInfo->horizontalHeader()->resizeSection(16, 110);
    ui->tableDetailedInfo->horizontalHeader()->resizeSection(15, 110);

    // 0 timestamp
    // 1 destination
    // 2 data ""
    // 3 latitude
    // 4 longiude
    // 5 altitude
    // 6 path
    // 7 payload ""
    // 8 message ""
    // 9 course
    // 10 speed 0
    // 11 temp nan
    // 12 pressure nan
    // 13 wind_speed nan
    // 14 wind_direction 0
    // 15 wind_gust nan
    // 16 rain_1hr nan
    // 17 rain_24hr nan
    // 18 rain_midnight nan
    // 19 snow_24hr nan
    // 20 raincount -1
    // 21 humidity -1
    // 22 luminosity -1


    bool hideThisRow;
    // altitude
    hideThisRow = true;
    for(int i=0; i < ui->tableDetailedInfo->rowCount(); i++)
    {
        if(ui->tableDetailedInfo->item(i, 5)->text() != "nan")
        {
            hideThisRow = false;
            break;
        }
    }
    ui->tableDetailedInfo->setColumnHidden(5, hideThisRow);
    // temperature
    hideThisRow = true;
    for(int i=0; i < ui->tableDetailedInfo->rowCount(); i++)
    {
        if(ui->tableDetailedInfo->item(i, 11)->text() != "nan")
        {
            hideThisRow = false;
            break;
        }
    }
    ui->tableDetailedInfo->setColumnHidden(11, hideThisRow);
    // pressure
    hideThisRow = true;
    for(int i=0; i < ui->tableDetailedInfo->rowCount(); i++)
    {
        if(ui->tableDetailedInfo->item(i, 12)->text() != "nan")
        {
            hideThisRow = false;
            break;
        }
    }
    ui->tableDetailedInfo->setColumnHidden(12, hideThisRow);
    // wind speed
    hideThisRow = true;
    for(int i=0; i < ui->tableDetailedInfo->rowCount(); i++)
    {
        if(ui->tableDetailedInfo->item(i, 13)->text() != "nan")
        {
            hideThisRow = false;
            break;
        }
    }
    ui->tableDetailedInfo->setColumnHidden(13, hideThisRow);
    // wind gust
    hideThisRow = true;
    for(int i=0; i < ui->tableDetailedInfo->rowCount(); i++)
    {
        if(ui->tableDetailedInfo->item(i, 15)->text() != "nan")
        {
            hideThisRow = false;
            break;
        }
    }
    ui->tableDetailedInfo->setColumnHidden(15, hideThisRow);
    // rain 1 hr
    hideThisRow = true;
    for(int i=0; i < ui->tableDetailedInfo->rowCount(); i++)
    {
        if(ui->tableDetailedInfo->item(i, 16)->text() != "nan")
        {
            hideThisRow = false;
            break;
        }
    }
    ui->tableDetailedInfo->setColumnHidden(16, hideThisRow);
    // rain 24 hr
    hideThisRow = true;
    for(int i=0; i < ui->tableDetailedInfo->rowCount(); i++)
    {
        if(ui->tableDetailedInfo->item(i, 17)->text() != "nan")
        {
            hideThisRow = false;
            break;
        }
    }
    ui->tableDetailedInfo->setColumnHidden(17, hideThisRow);
    // rain midnight
    hideThisRow = true;
    for(int i=0; i < ui->tableDetailedInfo->rowCount(); i++)
    {
        if(ui->tableDetailedInfo->item(i, 18)->text() != "nan")
        {
            hideThisRow = false;
            break;
        }
    }
    ui->tableDetailedInfo->setColumnHidden(18, hideThisRow);
    // snow 24 hr
    hideThisRow = true;
    for(int i=0; i < ui->tableDetailedInfo->rowCount(); i++)
    {
        if(ui->tableDetailedInfo->item(i, 19)->text() != "nan")
        {
            hideThisRow = false;
            break;
        }
    }
    ui->tableDetailedInfo->setColumnHidden(19, hideThisRow);
    // raincount
    hideThisRow = true;
    for(int i=0; i < ui->tableDetailedInfo->rowCount(); i++)
    {
        if(ui->tableDetailedInfo->item(i, 20)->text() != "-1")
        {
            hideThisRow = false;
            break;
        }
    }
    ui->tableDetailedInfo->setColumnHidden(20, hideThisRow);
    // humidity
    hideThisRow = true;
    for(int i=0; i < ui->tableDetailedInfo->rowCount(); i++)
    {
        if(ui->tableDetailedInfo->item(i, 21)->text() != "-1")
        {
            hideThisRow = false;
            break;
        }
    }
    ui->tableDetailedInfo->setColumnHidden(21, hideThisRow);
    // luminousity
    hideThisRow = true;
    for(int i=0; i < ui->tableDetailedInfo->rowCount(); i++)
    {
        if(ui->tableDetailedInfo->item(i, 22)->text() != "-1")
        {
            hideThisRow = false;
            break;
        }
    }
    ui->tableDetailedInfo->setColumnHidden(22, hideThisRow);
}
