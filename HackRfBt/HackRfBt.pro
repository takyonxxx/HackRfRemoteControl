QT = core
QT += bluetooth multimedia network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 cmdline
QMAKE_CXXFLAGS += -v

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
QMAKE_ASSET_CATALOGS_BUILD_PATH = $$PWD

SOURCES += \
        custombuffer.cpp \
        gattserver.cpp \
        main.cpp \
        message.cpp \
        sdrdevice.cpp

HEADERS += \
    custombuffer.h \
    gattserver.h \
    message.h \
    sdrdevice.h \
    tcpclient.h \
    udpclient.h


macos {
    QMAKE_INFO_PLIST = ./macos/Info.plist
    QMAKE_ASSET_CATALOGS = $$PWD/macos/Assets.xcassets
    QMAKE_ASSET_CATALOGS_APP_ICON = "AppIcon"

    INCLUDEPATH += /opt/homebrew/Cellar/spdlog/1.12.0/include
    INCLUDEPATH += /opt/homebrew/Cellar/fmt/10.2.1/include
    INCLUDEPATH += /opt/homebrew/Cellar/gmp/6.3.0/include
    INCLUDEPATH += /opt/homebrew/Cellar/gnuradio/3.10.9.2_1/include
    INCLUDEPATH += /opt/homebrew/Cellar/boost/1.84.0_1/include
    INCLUDEPATH += /opt/homebrew/Cellar/soapysdr/0.8.1_1/include

    LIBS += -L/opt/homebrew/Cellar/gnuradio/3.10.9.2_1/lib \
        -lgnuradio-analog \
        -lgnuradio-blocks \
        -lgnuradio-digital \
        -lgnuradio-filter \
        -lgnuradio-fft \
        -lgnuradio-runtime \
        -lgnuradio-audio \
        -lgnuradio-soapy \
        -lgnuradio-pmt \
        -lgnuradio-uhd

    LIBS += -L/opt/homebrew/Cellar/boost/1.84.0_1/lib -lboost_system -lboost_filesystem-mt -lboost_program_options
    LIBS += -L/opt/homebrew/Cellar/soapysdr/0.8.1_1/lib -lSoapySDR
}

unix:!macx{
    message("linux enabled")
#    sudo apt install qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools
#    sudo apt-get install qtmultimedia5-dev
#    sudo apt install qtconnectivity5-dev

#    sudo apt install libusb-1.0-0-dev
#    sudo apt-get install -y fftw3-dev
#    sudo apt install librtlsdr-dev
#    sudo apt install libportaudio2
#    sudo apt install portaudio19-dev
#    sudo apt-get install libgl-dev
#    sudo apt-get install hackrf
#    sudo apt-get install libhackrf-dev
#    sudo apt-get install libosmosdr-dev
#    sudo apt-get install libgl1-mesa-dev
#    sudo apt-get install gr-osmosdr
#    sudo apt-get install gnuradio
#    sudo apt-get install libsoapysdr-dev libsoapysdr0.8
#    nmap -sP 192.168.1.0/24

    INCLUDEPATH += /usr/include
    INCLUDEPATH += /usr/local/include
    INCLUDEPATH += /usr/include/osmosdr
    INCLUDEPATH += /usr/lib
    INCLUDEPATH += /usr/local/lib
    INCLUDEPATH += /usr/lib/x86_64-linux-gnu
    # INCLUDEPATH += /usr/lib/arm-linux-gnueabihf
    LIBS += -L/usr/lib/aarch64-linux-gnu/SoapySDR/modules0.8

    LIBS += -lboost_system -lboost_program_options -lboost_thread
    LIBS += -lrt -lpthread -losmosdr -lfmt -llog4cpp -lSoapySDR
    LIBS += -lgnuradio-analog \
    -lgnuradio-blocks \
    -lgnuradio-digital \
    -lgnuradio-filter \
    -lgnuradio-fft \
    -lgnuradio-runtime \
    -lgnuradio-audio \
    -lgnuradio-soapy \
    -lgnuradio-pmt \
    -lgnuradio-uhd

    # INCLUDEPATH += /usr/lib/x86_64-linux-gnu
    # INCLUDEPATH += /usr/lib/arm-linux-gnueabihf
    # LIBS += -L/usr/lib/arm-linux-gnueabihf/ -lhackrf
    # PRE_TARGETDEPS += /usr/lib/arm-linux-gnueabihf/libhackrf.a
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
