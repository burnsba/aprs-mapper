/* <APRS-Mapper . This program is used for recording and mapping APRS(Automatic Packet Reporting System) packets>
    Copyright (C) 2011  Ben Burns, Nikolas Boatright, Patrick Gilbert, Jason Schansman

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "connectionmenu.h"
#include "profilemenu.h"
#include "ui_profilemenu.h"
#include "createnew.h"
#include "aprsmain.h"
#include "parserthread.h"
#include "parser.h"
#include "point.h"
#include <QRegExp>
#include <QRegExpValidator>
#include <QDoubleValidator>
#include <QIntValidator>


ProfileMenu::ProfileMenu(int currTab, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProfileMenu)
{
    ui->setupUi(this);

    this->setWindowTitle("Profile: " + Profile::getCurrentProfile());

    ui->tabWidget->setCurrentIndex(currTab);

    ui->warningLabel->setVisible(false);

    QRegExp specialCharsRegex("^[^!@#$%^&\\*\\(\\)_\\+=\\[\\]\\\\\{\\}\\|;',\\./<>?:\"\\r\\n\\f]{1,8}$");
    QRegExpValidator* specialChars = new QRegExpValidator(specialCharsRegex, this);
    QRegExp longitudeRegex("^-?((1[0-7][0-9])|(0?[0-9][0-9]))?\\.\\d{0,4}$");
    QRegExpValidator* longValidate = new QRegExpValidator(longitudeRegex, this);
    QRegExp latitudeRegex("^-?(([0-8][0-9])|([0-9]))?\\.\\d{0,4}$");
    QRegExpValidator* latValidate = new QRegExpValidator(latitudeRegex, this);
    QIntValidator* portValidate = new QIntValidator(1, 65536, this);
    QIntValidator* reportValidate = new QIntValidator(1, 60, this);
    QIntValidator* timeValidators = new QIntValidator(1, 168, this);

    ui->callsignEdit->setValidator(specialChars);
    ui->longDegEditDecimal->setValidator(longValidate);
    ui->latDegEditDecimal->setValidator(latValidate);
    ui->portEdit->setValidator(portValidate);
    ui->posIntervalTimeEdit->setValidator(reportValidate);
    ui->purgeEdit->setValidator(timeValidators);
    ui->clearEdit->setValidator(timeValidators);

    ui->callsignEdit->setText(Profile::getCallSign());
    ui->latDegEditDecimal->setText(QString::number(Profile::getLatitude(), 'g', 6));
    ui->longDegEditDecimal->setText(QString::number(Profile::getLongitude(), 'g', 6));
    ui->passwordEdit->setText(Profile::getConnectionPass());
    ui->purgeEdit->setText(QString::number(Profile::getPurgeTime()));
    ui->clearEdit->setText(QString::number(Profile::getClearTime()));
    ui->OverlayCharEdit->setText(Profile::getOverlayChar());
    ui->SSID_edit->setText(Profile::getSSID());
    ui->posCommentEdit->setText(Profile::getPosComment());
    ui->posIntervalTimeEdit->setText(QString::number(Profile::getPosIntervalTime()));


    ui->unitsMetricButton->setChecked(!(Profile::isUnitsEnglish()));

    ui->dbUserEdit->setText(Profile::getMySQLUser());
    ui->dbPassEdit->setText(Profile::getMySQLPass());
    ui->dbNameEdit->setText(Profile::getMySQLDBName());
    ui->ipAddrEdit->setText(Profile::getMySQLIPAddr());
    ui->portEdit->setText(QString::number(Profile::getMySQLPort()));
    ui->dbFilepathEdit->setText(Profile::getSQLiteFilepath());

    ui->mySqlButton->setChecked(Profile::isMySQL());
    databaseToggle(Profile::isMySQL());

    ui->lineEditMapCache->setText(Profile::getTileCacheDir());
    ui->lineEditMapUrl->setText(Profile::getTileServerUrl());
    if (Profile::getOnlineMode())
    {
        ui->mapModeOnline->setChecked(true);
        ui->mapModeOffline->setChecked(false);
    }
    else
    {
        ui->mapModeOnline->setChecked(false);
        ui->mapModeOffline->setChecked(true);
    }

    refreshConnections();

    //ui->connectList->setCurrentItem(0);

    connect(ui->editButton, SIGNAL(clicked()), this, SLOT(editClicked()));
    connect(ui->removeButton, SIGNAL(clicked()), this, SLOT(removeClicked()));
    connect(ui->addButton, SIGNAL(clicked()), this, SLOT(addClicked()));
    connect(ui->mySqlButton, SIGNAL(toggled(bool)), this, SLOT(databaseToggle(bool)));
    connect(ui->unitsEnglishButton, SIGNAL(toggled(bool)), this, SLOT(unitsToggle(bool)));

    ui->latNameLabel_3->adjustSize();
    ui->latNameLabel_4->adjustSize();
    ui->latNameLabel_5->adjustSize();
    ui->passwordLabel->adjustSize();
    ui->dbFilepathLabel->adjustSize();
    ui->dbNameLabel->adjustSize();
    ui->dbPassLabel->adjustSize();
    ui->dbUserLabel->adjustSize();
    ui->clearLabel->adjustSize();
    ui->purgeLabel->adjustSize();
    ui->clearSecLabel->adjustSize();
    ui->purgeSecLabel->adjustSize();
    ui->ipAddrLabel->adjustSize();
    ui->portLabel->adjustSize();
    ui->posCommentLabel->adjustSize();
    ui->posIntervalTimeLabel->adjustSize();
    ui->posIntervalTimeMinLabel->adjustSize();
    ui->UnitsLabel->adjustSize();
    ui->unitsEnglishButton->adjustSize();
    ui->unitsMetricButton->adjustSize();
    ui->symIDLabel->adjustSize();

    // setup the icon box to trigger when it's clicked on
    MapGraphicsScene *icon = new MapGraphicsScene(ui->iconGraphicsView);
    ui->iconGraphicsView->setScene(icon);
    connect(icon, SIGNAL(mgsPressEvent(QGraphicsSceneMouseEvent*)), this, SLOT(on_icon_click(QGraphicsSceneMouseEvent*)));
    // hide the list of all icons
    ui->iconGraphicsViewIcons->hide();
    MapGraphicsScene *allIcons = new MapGraphicsScene(ui->iconGraphicsViewIcons);
    ui->iconGraphicsViewIcons->setScene(allIcons);
    connect(allIcons, SIGNAL(mgsPressEvent(QGraphicsSceneMouseEvent*)), this, SLOT(on_select_icon(QGraphicsSceneMouseEvent*)));

    // load the symbol id. Also: can't show the image until the graphics view has
    // been initialized.
    redrawIcon(Profile::getIconId());
    updateOverlayBox(Profile::getIconId());

    temp_icon_id = Profile::getIconId();

    connect(ui->purgeEdit, SIGNAL(textChanged(QString)), this, SLOT(purgeTimeChanged(QString)));
    connect(ui->clearEdit, SIGNAL(textChanged(QString)), this, SLOT(purgeTimeChanged(QString)));
    connect(ui->symIDEdit, SIGNAL(textEdited(QString)), this, SLOT(iconCharsChanged(QString)));
    connect(ui->OverlayCharEdit, SIGNAL(textEdited(QString)), this, SLOT(overlayChanged(QString)));
    iconImageChanged();
}


ProfileMenu::~ProfileMenu()
{
    delete ui;
}

void ProfileMenu::on_buttonBox_accepted()
{
    //ErrorHandler::Alert(ErrorType::EverythingIsOk);
    bool dbChange = false;
    if(ui->mySqlButton->isChecked() != Profile::isMySQL() || \
       ui->dbFilepathEdit->text() != Profile::getSQLiteFilepath() || \
       ui->dbNameEdit->text() != Profile::getMySQLDBName() || \
       ui->dbPassEdit->text() != Profile::getMySQLPass() || \
       ui->dbUserEdit->text() != Profile::getMySQLUser() || \
       ui->portEdit->text().toInt() != Profile::getMySQLPort() || \
       ui->ipAddrEdit->text() != Profile::getMySQLIPAddr())
    {
        dbChange = true;
    }

    Profile::setCallSign(ui->callsignEdit->text());
    Profile::setSSID(ui->SSID_edit->text());
    Profile::setPosComment(ui->posCommentEdit->text());
    Profile::setUnitsEnglish(ui->unitsEnglishButton->isChecked());
    Profile::setPosIntervalTime(ui->posIntervalTimeEdit->text().toInt());
    Profile::setLongitude(ui->longDegEditDecimal->text().toDouble());
    Profile::setLatitude(ui->latDegEditDecimal->text().toDouble());
    Profile::setConnectionPass(ui->passwordEdit->text());
    Profile::setMySQL(ui->mySqlButton->isChecked());
    Profile::setMySQLUser(ui->dbUserEdit->text());
    Profile::setMySQLPass(ui->dbPassEdit->text());
    Profile::setMySQLDBName(ui->dbNameEdit->text());
    Profile::setMySQLIPAddr(ui->ipAddrEdit->text());
    Profile::setMySQLPort(ui->portEdit->text().toInt());
    Profile::setSQLiteFilepath(ui->dbFilepathEdit->text());
    Profile::setOnlineMode(ui->mapModeOnline->isChecked() == true ? 1 : 0);
    Profile::setTileServerUrl(ui->lineEditMapUrl->text());
    Profile::setTileCacheDir(ui->lineEditMapCache->text());
    Profile::setClearTime(ui->clearEdit->text().toInt());
    Profile::setPurgeTime(ui->purgeEdit->text().toInt());
    Profile::setIconId(temp_icon_id);
    if (ui->OverlayCharEdit->text().length() > 0)
        Profile::setOverlayChar(ui->OverlayCharEdit->text().at(0));
    Profile::saveProfile();
    if(dbChange)
    {
        emit DatabaseChanged((ui->mySqlButton->isChecked()) ? "MySQL" : "SQLite");
    }

    emit refreshHome();
}

void ProfileMenu::editClicked()
{
    if(!ui->connectList->selectedItems().empty())
    {
        QString alias = ui->connectList->currentItem()->text();
        ConnectionMenu* c;
        c = new ConnectionMenu(alias);
        c->exec();
        refreshConnections();
        emit ConnectionEdited(alias);
    }
    else
    {
        ErrorHandler::Alert("Select a Connection to Edit");
    }
}

void ProfileMenu::removeClicked()
{
    if(!ui->connectList->selectedItems().empty())
    {
        QString alias = ui->connectList->currentItem()->text();
        Profile::removeConnection(alias);
        refreshConnections();
        emit ConnectionRemoved(alias);
    }
    else
    {
        ErrorHandler::Alert("Select a Connection to Remove");
    }
}
void ProfileMenu::addClicked()
{
    createnew* n = new createnew(true);
    n->exec();
    refreshConnections();
    emit ConnectionAdded();
}

void ProfileMenu::refreshConnections()
{
    ui->connectList->clear();
    QVector<QString> list = Profile::getConnectionList();
    int size = list.size();
    for(int count = 0; count < size; count++)
    {
//       QListWidgetItem* connectBox = new QListWidgetItem(list.at(count));
//       connectBox->setFlags(Qt::ItemIsUserCheckable);
//       //check connection state

//       if (Profile::getConnectStart(list.at(count)))
//           connectBox->setCheckState(Qt::Checked);
//       else
//           connectBox->setCheckState(Qt::Unchecked);


       ui->connectList->addItem(/*connectBox*/ list.at(count));

    }
}

void ProfileMenu::on_pushButtonCache_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select a directory to cache tiles", "", QFileDialog::ShowDirsOnly);
    ui->lineEditMapCache->setText(dir);
}

void ProfileMenu::on_dbFilepathButton_clicked()
{
    QString dir = QFileDialog::getOpenFileName(this, "Select the databasefile","");
    ui->dbFilepathEdit->setText(dir);
}

void ProfileMenu::databaseToggle(bool mySQL)
{
    if(mySQL)
    {
        ui->dbFilepathEdit->setEnabled(false);
        ui->dbFilepathButton->setEnabled(false);
        ui->dbNameEdit->setEnabled(true);
        ui->dbPassEdit->setEnabled(true);
        ui->dbUserEdit->setEnabled(true);
        ui->ipAddrEdit->setEnabled(true);
        ui->portEdit->setEnabled(true);
    }
    else
    {
        ui->dbFilepathEdit->setEnabled(true);
        ui->dbFilepathButton->setEnabled(true);
        ui->dbNameEdit->setEnabled(false);
        ui->dbPassEdit->setEnabled(false);
        ui->dbUserEdit->setEnabled(false);
        ui->ipAddrEdit->setEnabled(false);
        ui->portEdit->setEnabled(false);
    }
}

void ProfileMenu::unitsToggle(bool english)
{
    if(english)
    {
    }
    else
    {
    }
}

void ProfileMenu::on_icon_click(QGraphicsSceneMouseEvent *e)
{
    // set the background of the icons list to the icon image file
    QPixmap *iconsPixmap = new QPixmap("resource/aprs_icons.png");
    ui->iconGraphicsViewIcons->scene()->addPixmap(*iconsPixmap);
    ui->iconGraphicsViewIcons->scene()->setSceneRect(0, 0, iconsPixmap->width(), iconsPixmap->height());
    ui->iconGraphicsViewIcons->setGeometry(0, 0, iconsPixmap->width(), iconsPixmap->height());
    ui->iconGraphicsViewIcons->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    ui->iconGraphicsViewIcons->setTransformationAnchor(QGraphicsView::NoAnchor);
    ui->iconGraphicsViewIcons->setResizeAnchor(QGraphicsView::NoAnchor);
    ui->iconGraphicsViewIcons->show();
}

void ProfileMenu::on_select_icon(QGraphicsSceneMouseEvent *e)
{
    qreal x_pos = e->scenePos().x();
    qreal y_pos = e->scenePos().y();

    // all of the icons are in one .png file, in rows of 16 icons
    // icons are 20x20 px with 1px borders between
    int side_length = 20;
    int border = 1;
    int col = (x_pos / (side_length + border));
    int row = (y_pos / (side_length + border));
    int id = col + (row * 16);

    temp_icon_id = id; // set the temp variable to possibly be saved to the profile

    //qDebug() << "selected " + QString::number(id);

    updateOverlayBox(id);

    redrawIcon(id);

    ui->iconGraphicsViewIcons->hide();
}


void ProfileMenu::redrawIcon(int id)
{
    int h_start = 0;
    int v_start = 0;
    int side_length = 20;
    int border = 1;
    int start_col = id % 16; // 16 columns per row
    int start_row = id / 16;
    h_start = ((side_length * start_col) + (border * (start_col + 1)));
    v_start = ((side_length * start_row) + (border * (start_row + 1)));

    QPixmap *iconImage = new QPixmap(20, 20); // empty space for working
    QImage icons = QImage("resource/aprs_icons.png", 0); // going to use this file
    iconImage->fill(Qt::transparent);    // make sure the background is empty
    QPainter painter(iconImage); // set a painter
    painter.setBackgroundMode(Qt::TransparentMode); // make double sure the background is empty
    // so now grab the correct icon from the big file
    painter.drawImage(0, 0, icons, h_start, v_start, side_length, side_length);
    painter.end();

    foreach(QGraphicsItem *item, ui->iconGraphicsView->scene()->items())
    {
        ui->iconGraphicsView->scene()->removeItem(item);
    }

    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(*iconImage);
    ui->iconGraphicsView->scene()->addItem(item);
    ui->iconGraphicsView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    ui->iconGraphicsView->setTransformationAnchor(QGraphicsView::NoAnchor);
    ui->iconGraphicsView->setResizeAnchor(QGraphicsView::NoAnchor);
    iconImageChanged();
}

void ProfileMenu::updateOverlayBox(int id)
{
    if (overlayAllowed(id))
    {
        ui->OverlayCharEdit->setEnabled(true);
    }
    else
    {
        ui->OverlayCharEdit->setEnabled(false);
    }
}

void ProfileMenu::purgeTimeChanged(QString newTime)
{
    if(ui->purgeEdit->text().toInt() < ui->clearEdit->text().toInt())
    {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        ui->warningLabel->setVisible(true);
    }
    else
    {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        ui->warningLabel->setVisible(false);
    }
}

void ProfileMenu::iconCharsChanged(QString newChars)
{
    if(newChars.length() == 2)
    {
        QString tableLookup = QString(newChars[0]);
        QString charLookup = QString(newChars[1]);
        temp_icon_id = parser::GetSymbolID(tableLookup, charLookup);
        disconnect(ui->symIDEdit, SIGNAL(textEdited(QString)), this, SLOT(iconCharsChanged(QString)));
        disconnect(ui->OverlayCharEdit, SIGNAL(textEdited(QString)), this, SLOT(overlayChanged(QString)));
        redrawIcon(temp_icon_id);
        updateOverlayBox(temp_icon_id);


        if(newChars[0] != '\\' && newChars[0] != '/')
        {
            ui->OverlayCharEdit->setText(tableLookup);
        }

        connect(ui->symIDEdit, SIGNAL(textEdited(QString)), this, SLOT(iconCharsChanged(QString)));
        connect(ui->OverlayCharEdit, SIGNAL(textEdited(QString)), this, SLOT(overlayChanged(QString)));
    }
}

void ProfileMenu::iconImageChanged()
{
    disconnect(ui->symIDEdit, SIGNAL(textEdited(QString)), this, SLOT(iconCharsChanged(QString)));
    disconnect(ui->OverlayCharEdit, SIGNAL(textEdited(QString)), this, SLOT(overlayChanged(QString)));
    ui->symIDEdit->setText(parser::GetLookupCharsFromID(temp_icon_id));
    connect(ui->symIDEdit, SIGNAL(textEdited(QString)), this, SLOT(iconCharsChanged(QString)));
    connect(ui->OverlayCharEdit, SIGNAL(textEdited(QString)), this, SLOT(overlayChanged(QString)));
}

void ProfileMenu::overlayChanged(QString newOverlay)
{
    if(newOverlay != "")
    {
        QString symIdText = ui->symIDEdit->text();
        symIdText.remove(0,1);
        symIdText.prepend(newOverlay);
        qDebug() << symIdText;
        disconnect(ui->symIDEdit, SIGNAL(textEdited(QString)), this, SLOT(iconCharsChanged(QString)));
        disconnect(ui->OverlayCharEdit, SIGNAL(textEdited(QString)), this, SLOT(overlayChanged(QString)));
        ui->symIDEdit->setText(symIdText);
        connect(ui->symIDEdit, SIGNAL(textEdited(QString)), this, SLOT(iconCharsChanged(QString)));
        connect(ui->OverlayCharEdit, SIGNAL(textEdited(QString)), this, SLOT(overlayChanged(QString)));
    }
}
