#ifndef OVERLAY_H
#define OVERLAY_H

#include <QObject>
#include <QPixmap>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsItem>

// ordered byb state, then how it appears on the weather.gov page
// http://www.srh.noaa.gov/jetstream/doppler/ridge_download.htm

typedef enum _radar_station {

    BMX = 0,
    MXX = 1,
    EOX = 2,
    MOB = 3,
    HTX = 4,
    ABC = 5,
    ACG = 6,
    APD = 7,
    AHG = 8,
    AKC = 9,
    AIH = 10,
    AEC = 11,
    FSX = 12,
    IWA = 13,
    EMX = 14,
    YUX = 15,
    LZK = 16,
    SRX = 17,
    BBX = 18,
    EYX = 19,
    BHX = 20,
    HNX = 21,
    VTX = 22,
    DAX = 23,
    NKX = 24,
    MUX = 25,
    SOX = 26,
    VBX = 27,
    FTG = 28,
    GJX = 29,
    PUX = 30,
    DOX = 31,
    EVX = 32,
    JAX = 33,
    BYX = 34,
    MLB = 35,
    AMX = 36,
    TLH = 37,
    TBW = 38,
    FFC = 39,
    VAX = 40,
    JGX = 41,
    GUA = 42,
    HKI = 43,
    HKM = 44,
    HMO = 45,
    HWA = 46,
    CBX = 47,
    SFX = 48,
    ILX = 49,
    LOT = 50,
    VWX = 51,
    IND = 52,
    IWX = 53,
    DMX = 54,
    DVN = 55,
    DDC = 56,
    GLD = 57,
    TWX = 58,
    ICT = 59,
    HPX = 60,
    JKL = 61,
    LVX = 62,
    PAH = 63,
    POE = 64,
    LCH = 65,
    LIX = 66,
    SHV = 67,
    CBW = 68,
    GYX = 69,
    LWX = 70,
    BOX = 71,
    DTX = 72,
    APX = 73,
    GRR = 74,
    MQT = 75,
    DLH = 76,
    MPX = 77,
    GWX = 78,
    DGX = 79,
    EAX = 80,
    SGF = 81,
    LSX = 82,
    BLX = 83,
    GGW = 84,
    TFX = 85,
    MSX = 86,
    UEX = 87,
    LNX = 88,
    OAX = 89,
    LRX = 90,
    ESX = 91,
    RGX = 92,
    DIX = 93,
    ABX = 94,
    FDX = 95,
    HDX = 96,
    ENX = 97,
    BGM = 98,
    BUF = 99,
    TYX = 100,
    OKX = 101,
    RAX = 102,
    MHX = 103,
    LTX = 104,
    BIS = 105,
    MVX = 106,
    MBX = 107,
    ILN = 108,
    CLE = 109,
    FDR = 110,
    TLX = 111,
    INX = 112,
    VNX = 113,
    MAX = 114,
    PDT = 115,
    RTX = 116,
    PBZ = 117,
    CCX = 118,
    JUA = 119,
    CLX = 120,
    CAE = 121,
    GSP = 122,
    ABR = 123,
    UDX = 124,
    FSD = 125,
    MRX = 126,
    NQA = 127,
    OHX = 128,
    AMA = 129,
    EWX = 130,
    BRO = 131,
    GRK = 132,
    CRP = 133,
    FWS = 134,
    DYX = 135,
    EPZ = 136,
    HGX = 137,
    DFX = 138,
    LBB = 139,
    MAF = 140,
    SJT = 141,
    ICX = 142,
    MTX = 143,
    CXX = 144,
    FCX = 145,
    AKQ = 146,
    ATX = 147,
    OTX = 148,
    RLX = 149,
    GRB = 150,
    ARX = 151,
    MKX = 152,
    CYS = 153,
    RIW = 154,


    NULL_STATION = 155

} RadarStation;



QString RadarStationShortName(RadarStation r);
QString RadarStationLongName(RadarStation r);
QString RadarStationToState(RadarStation r);

class Overlay : public QObject
{
    Q_OBJECT
public:
    Overlay();
    Overlay(QImage overlay, qreal lon_origin, qreal lat_origin, qreal m_lon_width,
            qreal m_lat_height, int zoom, RadarStation r, int type);

    void Hide();
    void Show();
    void Draw();
    void RemoveFromMap();
    void setZoom(int z) { m_zoom = z; }
    void setScene(QGraphicsScene* g) { m_scene = g; }
    void setSceneLocation(qreal px, qreal py) { m_item->setPos(px, py); }
    QPointF getLocation() { return QPointF(m_lon_origin, m_lat_origin); }
    QPointF getSceneLocation() { return m_item->scenePos(); }
    RadarStation getId() { return m_id; }
    int getType() { return m_overlay_type; }

signals:

public slots:

private:
    int m_image_width;      // number of pixels wide the image is, (per zoom)
    int m_image_height;     // number of pixels tall the image is, (per zoom)
    int m_zoom;             // zoom level
    qreal m_lat_origin;
    qreal m_lon_origin;
    qreal m_lat_height;
    qreal m_lon_width;
    QPixmap *m_background;  // the overlay image
    QGraphicsItem *m_item;  // the graphics item in the scene
    QGraphicsScene* m_scene;
    RadarStation m_id;      // callsign/number/identifier thinger
    int m_overlay_type; // 0=undefined, 1=radar, 2=warning

};

#endif // OVERLAY_H
