#ifndef BLUETOOTHCLIENT_H
#define BLUETOOTHCLIENT_H

#include <QtBluetooth/qbluetoothdeviceinfo.h>
#include <QtBluetooth/qbluetoothuuid.h>
#include <QtBluetooth/qbluetoothdevicediscoveryagent.h>
#include <QtBluetooth/qlowenergycontroller.h>
#include <QtBluetooth/qlowenergyservice.h>
#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QMetaEnum>
#include <QDateTime>
#include <QLatin1String>
#include <qregularexpression.h>
#include <deviceinfo.h>

#define AdvancedAudioDistribution   "0000110d-0000-1000-8000-00805f9b34fb"

class BluetoothClient : public QObject
{
    Q_OBJECT

public:
    //TODO Error handling
    enum bluetoothleState {
        Idle = 0,
        Scanning,
        ScanFinished,
        Connecting,
        Connected,
        DisConnected,
        ServiceFound,
        AcquireData,
        Error
    };
    Q_ENUM(bluetoothleState)

    BluetoothClient();
    ~BluetoothClient();

    void writeData(QByteArray data);
    void setState(BluetoothClient::bluetoothleState newState);
    BluetoothClient::bluetoothleState getState() const;
    void getDeviceList(QList<QString> &qlDevices);
    void disconnectFromDevice();

    void setService_name(const QString &newService_name);
    void calculateAndEmitAverageBaud(qint64 dataSize, qint64 currentTime);

private slots:
    /* Slots for QBluetothDeviceDiscoveryAgent */
    void addDevice(const QBluetoothDeviceInfo&);
    void scanFinished();
    void deviceScanError(QBluetoothDeviceDiscoveryAgent::Error);

    /* Slots for QLowEnergyController */
    void serviceDiscovered(const QBluetoothUuid &);
    void serviceScanDone();
    void controllerError(QLowEnergyController::Error);
    void deviceConnected();
    void deviceDisconnected();
    void errorOccurred(QLowEnergyController::Error newError);

    /* Slotes for QLowEnergyService */
    void serviceStateChanged(QLowEnergyService::ServiceState s);
    void updateData(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void confirmedDescriptorWrite(const QLowEnergyDescriptor &d, const QByteArray &value);
    void searchCharacteristic();

public slots:
    /* Slots for user */
    void startScan();
    void startConnect();

signals:
    /* Signals for user */
    void statusChanged(const QString &status);
    void newData(QByteArray data);
    void sendInfo(QString);
    void sendBaud(QString);
    void changedState(BluetoothClient::bluetoothleState newState);

private:

    QLowEnergyController *m_control;
    QString m_service_name{"Bal"};
    QBluetoothUuid current_gatt{};

    QBluetoothDeviceDiscoveryAgent *m_deviceDiscoveryAgent;
    DeviceInfo *current_device{};
    QVector<quint16> m_qvMeasurements;

    QLowEnergyService *m_service;
    QLowEnergyDescriptor m_notificationDescTx;
    QLowEnergyDescriptor m_notificationDescRx;
    QLowEnergyService *m_UARTService;
    bool m_bFoundUARTService;
    BluetoothClient::bluetoothleState m_state;

    QLowEnergyCharacteristic m_readCharacteristic;
    QLowEnergyCharacteristic m_writeCharacteristic;
    QLowEnergyService::WriteMode m_writeMode;

    int packetCount = 0;
    qint64 totalReceivedDataSize = 0;
    qint64 totalBaud = 0;
    qint64 numberOfSamples = 0;
    qint64 lastUpdateTime = 0;

};

#endif // BLUETOOTHCLIENT_H
