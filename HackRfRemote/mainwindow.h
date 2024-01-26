#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QComboBox>
#include <QLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QLineEdit>
#include <QString>
#include <QDebug>
#include <QtWidgets>
#include <qregularexpression.h>
#include "message.h"
#include "bluetoothclient.h"
#include "hackrfmanager.h"
#include "udpserver.h"

#if defined (Q_OS_ANDROID)
const QVector<QString> permissions({"android.permission.BLUETOOTH",
                                    "android.permission.BLUETOOTH_ADMIN"});
#endif

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void DataHandler(QByteArray data);
    void sendCommand(uint8_t command, uint8_t value);
    void sendString(uint8_t command, QByteArray value);
    void requestData(uint8_t command);
    void changedState(BluetoothClient::bluetoothleState state);
    void getDataReceived(const QString &info);
    void getBaud(const QString &info);
    void getInfo(const QString &status);
    void on_ConnectClicked();   
    void on_Exit();
    void on_m_pBSpeak_clicked();
    void on_m_pBSetFreq_clicked();
    void setRadioValues();
    void setIp();    
    void getBuffer(QByteArray &buffer);

    QString enumToString(HackRfManager::Demod demod)
    {
        switch (demod)
        {
        case HackRfManager::DEMOD_AM:
            return "AM";
        case HackRfManager::DEMOD_WFM:
            return "WFM";
        case HackRfManager::DEMOD_NFM:
            return "NFM";
        case HackRfManager::DEMOD_USB:
            return "USB";
        case HackRfManager::DEMOD_LSB:
            return "LSB";
        case HackRfManager::DEMOD_CW:
            return "CW";
        case HackRfManager::DEMOD_BPSK31:
            return "BPSK31";
        default:
            return "Unknown";
        }
    }

    void on_m_pReset_clicked();

    void on_m_pIncFreq_clicked();

    void on_m_pDecFreq_clicked();

    void on_m_cFreqType_currentIndexChanged(int index);

signals:
    void connectToDevice(int i);

private:
    void initButtons();
    void createMessage(uint8_t msgId, uint8_t rw, QByteArray payload, QByteArray *result);
    void parseMessage(QByteArray *data, uint8_t &command, QByteArray &value,  uint8_t &rw);

    QList<QString> m_qlFoundDevices;
    BluetoothClient *m_bleConnection{};
    HackRfManager *hackRfManager{};
    UdpServer *udpServer{};
    bool m_connected{};
    Message message;
    HackRfManager::FreqMod currentFreqMod;
    HackRfManager::Demod currentDemod;

    Ui::MainWindow *ui;

};

#endif // MAINWINDOW_H
