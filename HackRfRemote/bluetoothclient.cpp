#include "bluetoothclient.h"

BluetoothClient::BluetoothClient() :
    m_control(nullptr),
    m_service(nullptr),
    m_state(bluetoothleState::Idle),
    packetCount(0),
    totalReceivedDataSize(0),
    totalBaud(0),
    numberOfSamples(0),
    lastUpdateTime(0)
{
    /* 1 Step: Bluetooth LE Device Discovery */
    m_deviceDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    m_deviceDiscoveryAgent->setLowEnergyDiscoveryTimeout(60000);
    /* Device Discovery Initialization */
    connect(m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &BluetoothClient::addDevice);
    connect(m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred,
            this, &BluetoothClient::deviceScanError);
    connect(m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
            this, &BluetoothClient::scanFinished);
    connect(m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled,
            this, &BluetoothClient::scanFinished);
}

BluetoothClient::~BluetoothClient(){
    if(current_device)
        delete current_device;
}

void BluetoothClient::getDeviceList(QList<QString> &qlDevices){

    if(m_state == bluetoothleState::ScanFinished && current_device)
    {
        qlDevices.append(current_device->getName());
    }
    else
    {
        qlDevices.clear();
    }
}

void BluetoothClient::addDevice(const QBluetoothDeviceInfo &device)
{
    /* Is it a LE Bluetooth device? */
    if (device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration)
    {
        if(device.name().isEmpty()) return;

        QString info = device.name() + "\nUuid: " + device.deviceUuid().toString();
        emit statusChanged(info);

        if(device.name().startsWith("HackRf"))
        {            
            current_device = new DeviceInfo(device);
            m_deviceDiscoveryAgent->stop();
            emit m_deviceDiscoveryAgent->finished();
            startConnect();
        }
    }
}

void BluetoothClient::scanFinished()
{
    if (!current_device)
    {
        QString info = "Low Energy device not found.";
        emit statusChanged(info);
    }
    setState(ScanFinished);
}

void BluetoothClient::deviceScanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    QString info;
    if (error == QBluetoothDeviceDiscoveryAgent::PoweredOffError)
        info = "The Bluetooth adaptor is powered off, power it on before doing discovery.";
    else if (error == QBluetoothDeviceDiscoveryAgent::InputOutputError)
        info = "Writing or reading from the device resulted in an error.";
    else {
        static QMetaEnum qme = m_deviceDiscoveryAgent->metaObject()->enumerator(
            m_deviceDiscoveryAgent->metaObject()->indexOfEnumerator("Error"));
        info = "Error: " + QLatin1String(qme.valueToKey(error));
    }

    setState(Error);
    emit statusChanged(info);
}

void BluetoothClient::startScan(){

    setState(Scanning);
    current_device = nullptr;
    m_deviceDiscoveryAgent->start();
}

void BluetoothClient::startConnect(){

    m_qvMeasurements.clear();

    if (m_control) {
        m_control->disconnectFromDevice();
        delete m_control;
        m_control = nullptr;
    }

    // 2 Step: QLowEnergyController
    m_control = QLowEnergyController::createCentral(current_device->getDevice(), this);
    m_control->setRemoteAddressType(QLowEnergyController::RandomAddress);

    connect(m_control, &QLowEnergyController::errorOccurred, this, &BluetoothClient::errorOccurred);
    connect(m_control, &QLowEnergyController::connected, this, &BluetoothClient::deviceConnected);
    connect(m_control, &QLowEnergyController::disconnected, this, &BluetoothClient::deviceDisconnected);
    connect(m_control, &QLowEnergyController::serviceDiscovered, this, &BluetoothClient::serviceDiscovered);
    connect(m_control, &QLowEnergyController::discoveryFinished, this, &BluetoothClient::serviceScanDone);

    // Set connection parameters
    QLowEnergyConnectionParameters params;
    params.setIntervalRange(6, 12);
    m_control->requestConnectionUpdate(params);

    // Start connecting to the device
    m_control->connectToDevice();
    setState(Connecting);
}

void BluetoothClient::setService_name(const QString &newService_name)
{
    m_service_name = newService_name;
}

void BluetoothClient::serviceDiscovered(const QBluetoothUuid &gatt)
{
    if(gatt == QBluetoothUuid(AdvancedAudioDistribution))
    {
        m_bFoundUARTService =true;
        current_gatt = gatt;
    }
}

void BluetoothClient::serviceScanDone()
{
    delete m_service;
    m_service=0;

    if(m_bFoundUARTService)
    {
        m_service = m_control->createServiceObject(current_gatt, this);
    }

    if(!m_service)
    {        
        disconnectFromDevice();
        setState(DisConnected);
        return;
    }

    QString info =  "Service Found: " + current_gatt.toString() ;
    emit statusChanged(info);;

    connect(m_service, &QLowEnergyService::stateChanged, this, &BluetoothClient::serviceStateChanged);
    connect(m_service, &QLowEnergyService::characteristicChanged, this, &BluetoothClient::updateData);
    connect(m_service, &QLowEnergyService::descriptorWritten, this, &BluetoothClient::confirmedDescriptorWrite);

    m_service->discoverDetails();
    setState(ServiceFound);

}

void BluetoothClient::disconnectFromDevice()
{
    m_control->disconnectFromDevice();
}

void BluetoothClient::deviceDisconnected()
{
    delete m_service;
    m_service = 0;
    setState(DisConnected);
}

void BluetoothClient::errorOccurred(QLowEnergyController::Error newError)
{
    auto statusText = QString("Controller Error: %1").arg(newError);
    qDebug() << statusText;
    emit statusChanged(statusText);
}

void BluetoothClient::deviceConnected()
{
    m_control->discoverServices();
    setState(Connected);
}

void BluetoothClient::controllerError(QLowEnergyController::Error error)
{
    QString info = QStringLiteral("Controller Error: ") + m_control->errorString();
    emit statusChanged(info);
}

void BluetoothClient::searchCharacteristic()
{
    if (!m_service)
        return;

    for (const auto &c : m_service->characteristics()) {
        if (!c.isValid())
            continue;

        if (c.properties() & (QLowEnergyCharacteristic::WriteNoResponse | QLowEnergyCharacteristic::Write)) {
            m_writeCharacteristic = c;
            m_writeMode = (c.properties() & QLowEnergyCharacteristic::WriteNoResponse) ?
                              QLowEnergyService::WriteWithoutResponse : QLowEnergyService::WriteWithResponse;

            QString info = "Tx Characteristic found\n" + c.uuid().toString();
            emit statusChanged(info);
            qDebug() << info;
        }

        if (c.properties() & (QLowEnergyCharacteristic::Notify | QLowEnergyCharacteristic::Read)) {
            m_readCharacteristic = c;
            QString info = "Rx Characteristic found\n" + c.uuid().toString();
            qDebug() << info;

            m_notificationDescRx = c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
            if (m_notificationDescRx.isValid()) {
                m_service->writeDescriptor(m_notificationDescRx, QByteArray::fromHex("0100"));

                QString descriptorInfo = "Write Rx Descriptor defined.\n" + c.uuid().toString();
                emit statusChanged(descriptorInfo);
                qDebug() << descriptorInfo;
            }
        }

        if (m_writeCharacteristic.isValid() && m_readCharacteristic.isValid()) {
            // Both characteristics found, exit the loop
            break;
        }
    }
}


/* Slotes for QLowEnergyService */
void BluetoothClient::serviceStateChanged(QLowEnergyService::ServiceState s)
{
    if (s == QLowEnergyService::RemoteServiceDiscovered)
    {
        searchCharacteristic();
    }
}

void BluetoothClient::calculateAndEmitAverageBaud(qint64 dataSize, qint64 currentTime)
{
    // Calculate time difference since the last update
    qint64 deltaTime = currentTime - lastUpdateTime;

    if (deltaTime > 0)
    {
        // Calculate baud for the current update
        qint64 bitsTransferred = dataSize * 8;
        qint64 baud = (bitsTransferred * 1000) / deltaTime;  // Assuming time is in milliseconds

        // Update total baud and number of samples
        totalBaud += baud;
        numberOfSamples++;

        // Calculate average baud rate in kbaud
        double averageBaudK = static_cast<double>(totalBaud) / numberOfSamples / 1000;

        // Emit the average baud rate in kbaud
        emit sendBaud(QString("Baud: %1 kbaud").arg(averageBaudK, 0, 'f', 1));
    }

    // Update lastUpdateTime for the next calculation
    lastUpdateTime = currentTime;
}

void BluetoothClient::updateData(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    emit newData(value);
//    packetCount++;
//    qint64 dataSize = value.size();
//    totalReceivedDataSize += dataSize;
//    double totalReceivedDataMB = static_cast<double>(totalReceivedDataSize) / (1024 * 1024);
//    QString totalReceivedDataString = QString::number(totalReceivedDataMB, 'f', 3) + " MB" + " - " + QString::number(dataSize) + " Byte";
//    emit sendInfo(totalReceivedDataString);
//    calculateAndEmitAverageBaud(dataSize, QDateTime::currentMSecsSinceEpoch());
}

void BluetoothClient::confirmedDescriptorWrite(const QLowEnergyDescriptor &d, const QByteArray &value)
{
    setState(AcquireData);
}

void BluetoothClient::writeData(QByteArray data)
{
    if(m_service && m_writeCharacteristic.isValid())
    {
        m_service->writeCharacteristic(m_writeCharacteristic, data, m_writeMode);
    }
}

void BluetoothClient::setState(BluetoothClient::bluetoothleState newState)
{
    if (m_state == newState)
        return;

    m_state = newState;
    emit changedState(newState);
}

BluetoothClient::bluetoothleState BluetoothClient::getState() const {
    return m_state;
}
