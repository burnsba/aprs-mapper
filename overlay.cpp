#include "overlay.h"
#include "coordinate.h"

#include <QGraphicsView>
#include <QPixmap>

QString RadarStationShortName(RadarStation r)
{
    switch(r)
    {
    case (BMX): return "BMX";
    case (MXX): return "MXX";
    case (EOX): return "EOX";
    case (MOB): return "MOB";
    case (HTX): return "HTX";
    case (ABC): return "ABC";
    case (ACG): return "ACG";
    case (APD): return "APD";
    case (AHG): return "AHG";
    case (AKC): return "AKC";
    case (AIH): return "AIH";
    case (AEC): return "AEC";
    case (FSX): return "FSX";
    case (IWA): return "IWA";
    case (EMX): return "EMX";
    case (YUX): return "YUX";
    case (LZK): return "LZK";
    case (SRX): return "SRX";
    case (BBX): return "BBX";
    case (EYX): return "EYX";
    case (BHX): return "BHX";
    case (HNX): return "HNX";
    case (VTX): return "VTX";
    case (DAX): return "DAX";
    case (NKX): return "NKX";
    case (MUX): return "MUX";
    case (SOX): return "SOX";
    case (VBX): return "VBX";
    case (FTG): return "FTG";
    case (GJX): return "GJX";
    case (PUX): return "PUX";
    case (DOX): return "DOX";
    case (EVX): return "EVX";
    case (JAX): return "JAX";
    case (BYX): return "BYX";
    case (MLB): return "MLB";
    case (AMX): return "AMX";
    case (TLH): return "TLH";
    case (TBW): return "TBW";
    case (FFC): return "FFC";
    case (VAX): return "VAX";
    case (JGX): return "JGX";
    case (GUA): return "GUA";
    case (HKI): return "HKI";
    case (HKM): return "HKM";
    case (HMO): return "HMO";
    case (HWA): return "HWA";
    case (CBX): return "CBX";
    case (SFX): return "SFX";
    case (ILX): return "ILX";
    case (LOT): return "LOT";
    case (VWX): return "VWX";
    case (IND): return "IND";
    case (IWX): return "IWX";
    case (DMX): return "DMX";
    case (DVN): return "DVN";
    case (DDC): return "DDC";
    case (GLD): return "GLD";
    case (TWX): return "TWX";
    case (ICT): return "ICT";
    case (HPX): return "HPX";
    case (JKL): return "JKL";
    case (LVX): return "LVX";
    case (PAH): return "PAH";
    case (POE): return "POE";
    case (LCH): return "LCH";
    case (LIX): return "LIX";
    case (SHV): return "SHV";
    case (CBW): return "CBW";
    case (GYX): return "GYX";
    case (LWX): return "LWX";
    case (BOX): return "BOX";
    case (DTX): return "DTX";
    case (APX): return "APX";
    case (GRR): return "GRR";
    case (MQT): return "MQT";
    case (DLH): return "DLH";
    case (MPX): return "MPX";
    case (GWX): return "GWX";
    case (DGX): return "DGX";
    case (EAX): return "EAX";
    case (SGF): return "SGF";
    case (LSX): return "LSX";
    case (BLX): return "BLX";
    case (GGW): return "GGW";
    case (TFX): return "TFX";
    case (MSX): return "MSX";
    case (UEX): return "UEX";
    case (LNX): return "LNX";
    case (OAX): return "OAX";
    case (LRX): return "LRX";
    case (ESX): return "ESX";
    case (RGX): return "RGX";
    case (DIX): return "DIX";
    case (ABX): return "ABX";
    case (FDX): return "FDX";
    case (HDX): return "HDX";
    case (ENX): return "ENX";
    case (BGM): return "BGM";
    case (BUF): return "BUF";
    case (TYX): return "TYX";
    case (OKX): return "OKX";
    case (RAX): return "RAX";
    case (MHX): return "MHX";
    case (LTX): return "LTX";
    case (BIS): return "BIS";
    case (MVX): return "MVX";
    case (MBX): return "MBX";
    case (ILN): return "ILN";
    case (CLE): return "CLE";
    case (FDR): return "FDR";
    case (TLX): return "TLX";
    case (INX): return "INX";
    case (VNX): return "VNX";
    case (MAX): return "MAX";
    case (PDT): return "PDT";
    case (RTX): return "RTX";
    case (PBZ): return "PBZ";
    case (CCX): return "CCX";
    case (JUA): return "JUA";
    case (CLX): return "CLX";
    case (CAE): return "CAE";
    case (GSP): return "GSP";
    case (ABR): return "ABR";
    case (UDX): return "UDX";
    case (FSD): return "FSD";
    case (MRX): return "MRX";
    case (NQA): return "NQA";
    case (OHX): return "OHX";
    case (AMA): return "AMA";
    case (EWX): return "EWX";
    case (BRO): return "BRO";
    case (GRK): return "GRK";
    case (CRP): return "CRP";
    case (FWS): return "FWS";
    case (DYX): return "DYX";
    case (EPZ): return "EPZ";
    case (HGX): return "HGX";
    case (DFX): return "DFX";
    case (LBB): return "LBB";
    case (MAF): return "MAF";
    case (SJT): return "SJT";
    case (ICX): return "ICX";
    case (MTX): return "MTX";
    case (CXX): return "CXX";
    case (FCX): return "FCX";
    case (AKQ): return "AKQ";
    case (ATX): return "ATX";
    case (OTX): return "OTX";
    case (RLX): return "RLX";
    case (GRB): return "GRB";
    case (ARX): return "ARX";
    case (MKX): return "MKX";
    case (CYS): return "CYS";
    case (RIW): return "RIW";
    default:
        return "";
    }
}

QString RadarStationLongName(RadarStation r)
{
    switch(r)
    {
    case (BMX): return "AL, Shelby County AP/Birmingham";
    case (MXX): return "AL, Carrville/Maxwell AFB";
    case (EOX): return "AL, Ft. Rucker";
    case (MOB): return "AL, Mobile";
    case (HTX): return "AL, N.E./Hytop";
    case (ABC): return "AK, Bethel";
    case (ACG): return "AK, Sitka";
    case (APD): return "AK, Fairbanks";
    case (AHG): return "AK, Anchorage/Kenei";
    case (AKC): return "AK, King Salmon";
    case (AIH): return "AK, Middleton Islands";
    case (AEC): return "AK, Nome";
    case (FSX): return "AZ, Flagstaff";
    case (IWA): return "AZ, Williams AFB/Phoenix";
    case (EMX): return "AZ, Tucson";
    case (YUX): return "AZ, Yuma";
    case (LZK): return "AR, Little Rock";
    case (SRX): return "AR, Western Arkansas/Ft. Smith";
    case (BBX): return "CA, Beale AFB";
    case (EYX): return "CA, Edwards AFB";
    case (BHX): return "CA, Eureka";
    case (HNX): return "CA, Hanford AP/San Joaquin Valley";
    case (VTX): return "CA, Sulphur Mtn/Los Angeles";
    case (DAX): return "CA, McClellan AFB/Sacremento";
    case (NKX): return "CA, San Diego";
    case (MUX): return "CA, Mt Umunhum/San Francisco";
    case (SOX): return "CA, Santa Ana Mountains/March AFB";
    case (VBX): return "CA, Vandenberg AFB";
    case (FTG): return "CO, Front Range AP/Denver";
    case (GJX): return "CO, Grand Junction";
    case (PUX): return "CO, Pueblo";
    case (DOX): return "DE, Dover AFB";
    case (EVX): return "FL, Red Bay/Eglin AFB";
    case (JAX): return "FL, Jacksonville";
    case (BYX): return "FL, Key West";
    case (MLB): return "FL, Melbourne";
    case (AMX): return "FL, Miami";
    case (TLH): return "FL, Tallahassee";
    case (TBW): return "FL, Ruskin/Tampa Bay";
    case (FFC): return "GA, Peach Tree City/Atlanta";
    case (VAX): return "GA, Moody AFB";
    case (JGX): return "GA, Robins AFB";
    case (GUA): return "Gu, Andersen AFBam";
    case (HKI): return "HI, South Kauai";
    case (HKM): return "HI, Kohala";
    case (HMO): return "HI, Molokai";
    case (HWA): return "HI, South Hawaii";
    case (CBX): return "ID, Boise";
    case (SFX): return "ID, Pocatello/Idaho falls";
    case (ILX): return "IL, Central [Springfield]";
    case (LOT): return "IL, Chicago";
    case (VWX): return "IN, Evansville";
    case (IND): return "IN, Indianapolis";
    case (IWX): return "IN, Webster";
    case (DMX): return "IA, Acorn Valley/Des Moines";
    case (DVN): return "IA, Davenport/Quad Cities";
    case (DDC): return "KS, Dodge City";
    case (GLD): return "KS, Goodland";
    case (TWX): return "KS, Wabaunsee County/Topeka";
    case (ICT): return "KS, Wichita";
    case (HPX): return "KY, Ft. Campbell";
    case (JKL): return "KY, Jackson";
    case (LVX): return "KY, Ft Knox Mil Res/Louisville";
    case (PAH): return "KY, Paducah";
    case (POE): return "LA, Ft Polk";
    case (LCH): return "LA, Lake Charles";
    case (LIX): return "LA, Slidell AP/New Orleans";
    case (SHV): return "LA, Shreveport";
    case (CBW): return "ME, Caribou [Loring AFB]";
    case (GYX): return "ME, Gray/Portland";
    case (LWX): return "VA, Sterling/Washington DC";
    case (BOX): return "MA, Taunton/Boston";
    case (DTX): return "MI, Pontiac/Detroit";
    case (APX): return "MI, Alpena/Gaylord";
    case (GRR): return "MI, Grand Rapids/Muskegon";
    case (MQT): return "MI, Marquette";
    case (DLH): return "MN, Duluth";
    case (MPX): return "MN, Chanhassen Township/Minn-St.P";
    case (GWX): return "MS, Columbus AFB";
    case (DGX): return "MS, Jackson/Brandon";
    case (EAX): return "MO, Pleasant Hill/KC";
    case (SGF): return "MO, Springfield";
    case (LSX): return "MO, St Charles City/St Louis";
    case (BLX): return "MT, Billings";
    case (GGW): return "MT, Glasgow";
    case (TFX): return "MT, Great Falls";
    case (MSX): return "MT, Pt Six Mtn/Missoula";
    case (UEX): return "NE, Grand Island";
    case (LNX): return "NE, North Platte";
    case (OAX): return "NE, Omaha";
    case (LRX): return "NV, Elko";
    case (ESX): return "NV, Las Vegas";
    case (RGX): return "NV, Virginia Peak/Reno";
    case (DIX): return "NJ, Fort Dix/Philadelphia, PA";
    case (ABX): return "NM, La Mesita Negra/Albuquerque";
    case (FDX): return "NM, Field Village/Cannon AFB";
    case (HDX): return "NM, Holloman AFB";
    case (ENX): return "NY, East Berne/Albany";
    case (BGM): return "NY, Binghamton";
    case (BUF): return "NY, Buffalo";
    case (TYX): return "NY, Ft Drum AFB/Montague";
    case (OKX): return "NY, Brookhaven/New York City";
    case (RAX): return "NC, Triple West AP/Raleigh-Durham";
    case (MHX): return "NC, Newport/Morehead City";
    case (LTX): return "NC, Shallotte/Wilmington";
    case (BIS): return "ND, Bismarck";
    case (MVX): return "ND, Fargo";
    case (MBX): return "ND, Minot AFB";
    case (ILN): return "OH, Wilmington/Cincinnati";
    case (CLE): return "OH, Cleveland";
    case (FDR): return "OK, Frederick";
    case (TLX): return "OK, Twin Lakes/Oklahoma City";
    case (INX): return "OK, Shreck Farm/Tulsa";
    case (VNX): return "OK, Vance AFB";
    case (MAX): return "OR, Medford";
    case (PDT): return "OR, Pendleton";
    case (RTX): return "OR, Portland";
    case (PBZ): return "PA, Coraopolis/Pittsburgh";
    case (CCX): return "PA, Moshannon St Forest/State College";
    case (JUA): return "PR, San Juan";
    case (CLX): return "SC, Charleston";
    case (CAE): return "SC, Columbia";
    case (GSP): return "SC, Greenville/Spartanburg [Greer]";
    case (ABR): return "SD, Aberdeen";
    case (UDX): return "SD, Rapid City";
    case (FSD): return "SD, Sioux Falls";
    case (MRX): return "TN, Knoxville";
    case (NQA): return "TN, Millington NAS/Memphis";
    case (OHX): return "TN, Old Hickory Mt/Nashville";
    case (AMA): return "TX, Amarillo";
    case (EWX): return "TX, New Braunfels AP/Austin-San Ant";
    case (BRO): return "TX, Brownsville";
    case (GRK): return "TX, Central Texas [Ft Hood]";
    case (CRP): return "TX, Corpus Christi";
    case (FWS): return "TX, Spinks AP/Dallas-Ft Worth";
    case (DYX): return "TX, Moran/Dyess AFB";
    case (EPZ): return "TX, El Paso";
    case (HGX): return "TX, League City/Houston";
    case (DFX): return "TX, Bracketville/Laughlin AFB";
    case (LBB): return "TX, Lubbock";
    case (MAF): return "TX, Midland/Odessa";
    case (SJT): return "TX, San Angelo";
    case (ICX): return "UT, Cedar City";
    case (MTX): return "UT, Promontory Pt/Salt Lake City";
    case (CXX): return "VT, Burlington";
    case (FCX): return "VA, Roanoke";
    case (AKQ): return "VA, Wakefield/Norfolk-Richmond";
    case (ATX): return "WA, Everett/Seattle-Tacoma";
    case (OTX): return "WA, Spokane";
    case (RLX): return "WV, Charleston";
    case (GRB): return "WI, Green Bay";
    case (ARX): return "WI, LaCrosse";
    case (MKX): return "WI, Sullivan Township/Milwaukee";
    case (CYS): return "WY, Cheyenne";
    case (RIW): return "WY, Riverton";

    default:
        return "";
    }
}

QString RadarStationToState(RadarStation r)
{
    switch(r)
    {
    case (BMX): return "AL";
    case (MXX): return "AL";
    case (EOX): return "AL";
    case (MOB): return "AL";
    case (HTX): return "AL";
    case (ABC): return "AK";
    case (ACG): return "AK";
    case (APD): return "AK";
    case (AHG): return "AK";
    case (AKC): return "AK";
    case (AIH): return "AK";
    case (AEC): return "AK";
    case (FSX): return "AZ";
    case (IWA): return "AZ";
    case (EMX): return "AZ";
    case (YUX): return "AZ";
    case (LZK): return "AR";
    case (SRX): return "AR";
    case (BBX): return "CA";
    case (EYX): return "CA";
    case (BHX): return "CA";
    case (HNX): return "CA";
    case (VTX): return "CA";
    case (DAX): return "CA";
    case (NKX): return "CA";
    case (MUX): return "CA";
    case (SOX): return "CA";
    case (VBX): return "CA";
    case (FTG): return "CO";
    case (GJX): return "CO";
    case (PUX): return "CO";
    case (DOX): return "DE";
    case (EVX): return "FL";
    case (JAX): return "FL";
    case (BYX): return "FL";
    case (MLB): return "FL";
    case (AMX): return "FL";
    case (TLH): return "FL";
    case (TBW): return "FL";
    case (FFC): return "GA";
    case (VAX): return "GA";
    case (JGX): return "GA";
    case (GUA): return "Gu";
    case (HKI): return "HI";
    case (HKM): return "HI";
    case (HMO): return "HI";
    case (HWA): return "HI";
    case (CBX): return "ID";
    case (SFX): return "ID";
    case (ILX): return "IL";
    case (LOT): return "IL";
    case (VWX): return "IN";
    case (IND): return "IN";
    case (IWX): return "IN";
    case (DMX): return "IA";
    case (DVN): return "IA";
    case (DDC): return "KS";
    case (GLD): return "KS";
    case (TWX): return "KS";
    case (ICT): return "KS";
    case (HPX): return "KY";
    case (JKL): return "KY";
    case (LVX): return "KY";
    case (PAH): return "KY";
    case (POE): return "LA";
    case (LCH): return "LA";
    case (LIX): return "LA";
    case (SHV): return "LA";
    case (CBW): return "ME";
    case (GYX): return "ME";
    case (LWX): return "VA";
    case (BOX): return "MA";
    case (DTX): return "MI";
    case (APX): return "MI";
    case (GRR): return "MI";
    case (MQT): return "MI";
    case (DLH): return "MN";
    case (MPX): return "MN";
    case (GWX): return "MS";
    case (DGX): return "MS";
    case (EAX): return "MO";
    case (SGF): return "MO";
    case (LSX): return "MO";
    case (BLX): return "MT";
    case (GGW): return "MT";
    case (TFX): return "MT";
    case (MSX): return "MT";
    case (UEX): return "NE";
    case (LNX): return "NE";
    case (OAX): return "NE";
    case (LRX): return "NV";
    case (ESX): return "NV";
    case (RGX): return "NV";
    case (DIX): return "NJ";
    case (ABX): return "NM";
    case (FDX): return "NM";
    case (HDX): return "NM";
    case (ENX): return "NY";
    case (BGM): return "NY";
    case (BUF): return "NY";
    case (TYX): return "NY";
    case (OKX): return "NY";
    case (RAX): return "NC";
    case (MHX): return "NC";
    case (LTX): return "NC";
    case (BIS): return "ND";
    case (MVX): return "ND";
    case (MBX): return "ND";
    case (ILN): return "OH";
    case (CLE): return "OH";
    case (FDR): return "OK";
    case (TLX): return "OK";
    case (INX): return "OK";
    case (VNX): return "OK";
    case (MAX): return "OR";
    case (PDT): return "OR";
    case (RTX): return "OR";
    case (PBZ): return "PA";
    case (CCX): return "PA";
    case (JUA): return "PR";
    case (CLX): return "SC";
    case (CAE): return "SC";
    case (GSP): return "SC";
    case (ABR): return "SD";
    case (UDX): return "SD";
    case (FSD): return "SD";
    case (MRX): return "TN";
    case (NQA): return "TN";
    case (OHX): return "TN";
    case (AMA): return "TX";
    case (EWX): return "TX";
    case (BRO): return "TX";
    case (GRK): return "TX";
    case (CRP): return "TX";
    case (FWS): return "TX";
    case (DYX): return "TX";
    case (EPZ): return "TX";
    case (HGX): return "TX";
    case (DFX): return "TX";
    case (LBB): return "TX";
    case (MAF): return "TX";
    case (SJT): return "TX";
    case (ICX): return "UT";
    case (MTX): return "UT";
    case (CXX): return "VT";
    case (FCX): return "VA";
    case (AKQ): return "VA";
    case (ATX): return "WA";
    case (OTX): return "WA";
    case (RLX): return "WV";
    case (GRB): return "WI";
    case (ARX): return "WI";
    case (MKX): return "WI";
    case (CYS): return "WY";
    case (RIW): return "WY";
    default:
        return "";
    }
}


Overlay::Overlay()
{

}

Overlay::Overlay(QImage overlay, qreal lon_origin, qreal lat_origin, qreal lon_width,
                 qreal lat_height, int zoom, RadarStation r, int type)
{

    m_image_height = overlay.height();
    m_image_width  = overlay.width();
    m_lon_origin = lon_origin;
    m_lat_origin = lat_origin;
    m_lon_width = lon_width;
    m_lat_height = lat_height;
    m_zoom = zoom;
    m_id = r;
    m_overlay_type = type;
    m_item = NULL;
    m_scene = NULL;

    // draw the overlay as a pixmap.
    m_background = new QPixmap(m_image_width, m_image_height);  // empty space for working
    m_background->fill(Qt::transparent);                        // make sure the background is empty
    QPainter painter(m_background);                             // set a painter
    painter.setBackgroundMode(Qt::TransparentMode);             // make double sure the background is empty
    painter.drawImage(0, 0, overlay, 0, 0, m_image_width, m_image_height);
    painter.end();

}


void Overlay::Hide()
{
    if (m_item != NULL)
        m_item->hide();
}

void Overlay::Show()
{
    if (m_item != NULL)
        m_item->show();
}

void Overlay::RemoveFromMap()
{
    if (m_item != NULL)
    {
        if (m_item->scene() != NULL)
        m_scene->removeItem(m_item);
        m_scene = NULL;
        delete m_item;
    }
}


void Overlay::Draw()
{

    if ((m_scene == NULL) || (m_background == NULL))
        return;

    try
    {
        if (m_item != NULL)
        {
            m_scene->removeItem(m_item);
            delete m_item;
        }
    }
    catch (...)    {    }

    // figure out the size of the overlay
    // take the upper left corner in tile position ...
    QPointF upper_left = tileForCoordinate(m_lat_origin, m_lon_origin, m_zoom);
    // (y scale is negative)
    // then take the lower right corner in tile position ...
    QPointF lower_right = tileForCoordinate(m_lat_origin - m_lat_height, m_lon_origin + m_lon_width, m_zoom);

    // now figure out the pixel size, converting from tiles to pixels
    qreal width_it_should_be_at_this_zoom = (upper_left.x() - lower_right.x()) * TILE_SIZE;
    width_it_should_be_at_this_zoom = width_it_should_be_at_this_zoom < 0 ? width_it_should_be_at_this_zoom * -1 : width_it_should_be_at_this_zoom;
    qreal height_it_should_be_at_this_zoom = (upper_left.y() - lower_right.y()) * TILE_SIZE;
    height_it_should_be_at_this_zoom = height_it_should_be_at_this_zoom < 0 ? height_it_should_be_at_this_zoom * -1 : height_it_should_be_at_this_zoom;

    // now scale the image
    QPixmap scaled_pixmap = m_background->scaled(width_it_should_be_at_this_zoom,
              height_it_should_be_at_this_zoom);

    // all done. add it to the scene
    m_item = new QGraphicsPixmapItem(scaled_pixmap);

    // warnings should go on top of radar images
    if (m_overlay_type == 1)    // radar
        m_item->setZValue(5);
    else if (m_overlay_type == 2) // warnings
        m_item->setZValue(10);
    // the scene position will be adjusted by Map::Add()
    m_item->setPos(1,1);
    m_scene->addItem(m_item);
}
