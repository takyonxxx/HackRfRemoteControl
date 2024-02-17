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
        audiootput.cpp \
        gattserver.cpp \
        hackrfmanager.cpp \
        main.cpp \
        message.cpp \
        modulator.cpp \
        sdrdevice.cpp

HEADERS += \
    IHackRFData.h \
    audiootput.h \
    gattserver.h \
    hackrfmanager.h \
    modulator.h \
    message.h \
    sdrdevice.h \
    tcpclient.h \
    topblock.h \
    udpclient.h


macos {
    QMAKE_INFO_PLIST = ./macos/Info.plist
    QMAKE_ASSET_CATALOGS = $$PWD/macos/Assets.xcassets
    QMAKE_ASSET_CATALOGS_APP_ICON = "AppIcon"

    INCLUDEPATH += /opt/homebrew/Cellar/gnuradio/3.10.9.2_1/include
    INCLUDEPATH += /opt/homebrew/Cellar/soapysdr/0.8.1_1/include
    INCLUDEPATH += /opt/homebrew/Cellar/boost/1.84.0/include
    INCLUDEPATH += /opt/homebrew/Cellar/uhd/4.6.0.0_1/include
    INCLUDEPATH += /opt/homebrew/opt/rtl-sdr/include
    INCLUDEPATH += /opt/homebrew/Cellar/fftw/3.3.10_1/include
    INCLUDEPATH += /opt/homebrew/Cellar/portaudio/19.7.0/include
    INCLUDEPATH += /opt/homebrew/Cellar/hackrf/2023.01.1/include/libhackrf
    INCLUDEPATH += /opt/homebrew/Cellar/opt/qwt/include
    INCLUDEPATH += /opt/homebrew/Cellar/spdlog/1.12.0/include
    INCLUDEPATH += /opt/homebrew/Cellar/fmt/10.2.1/include
    INCLUDEPATH += /opt/homebrew/Cellar/gmp/6.3.0/include

    LIBS += -L/opt/homebrew/Cellar/gnuradio/3.10.9.2_1/lib \
        -lgnuradio-analog \
        -lgnuradio-blocks \
        -lgnuradio-digital \
        -lgnuradio-filter \
        -lgnuradio-fft \
        -lgnuradio-runtime \
        -lgnuradio-audio \
        -lgnuradio-osmosdr \
        -lgnuradio-uhd

    LIBS += -L/opt/homebrew/Cellar/soapysdr/0.8.1_1/lib -lSoapySDR

    LIBS += -L/opt/homebrew/Cellar/boost//1.84.0/lib -lboost_system -lboost_filesystem-mt -lboost_program_options

    LIBS += -L/opt/homebrew/opt/rtl-sdr/lib -lrtlsdr \
        -L/opt/homebrew/Cellar/fftw/3.3.10_1/lib -lfftw3 \
        -L/opt/homebrew/Cellar/portaudio/19.7.0/lib -lportaudio \
        -L/opt/homebrew/Cellar/uhd/4.6.0.0_1/lib -luhd \
        -L/opt/homebrew/Cellar/hackrf/2023.01.1/lib -lhackrf

    INCLUDEPATH += /opt/local/include
    LIBS += -L/opt/local/lib -lqwt
}

unix:!macx{
    message("linux enabled")
#    sudo apt install libusb-1.0-0-dev
#    sudo apt-get install -y fftw3-dev
#    sudo apt install librtlsdr-dev
#    sudo apt install libportaudio2
#    sudo apt install portaudio19-dev
#    sudo apt install sox
#    sudo apt-get install libgl-dev
#    sudo apt-get install qtmultimedia5-dev
#    sudo apt-get install hackrf
#    sudo apt-get install -y libhackrf-dev
#    sudo apt-get install libosmosdr-dev
#    sudo apt-get install gr-osmosdr
#    sudo apt-get install gnuradio

    INCLUDEPATH += /usr/lib
    INCLUDEPATH += /usr/include
    INCLUDEPATH += /usr/local/include
    INCLUDEPATH += /usr/local/lib
    INCLUDEPATH += /usr/include/libhackrf   
    INCLUDEPATH += /lib/x86_64-linux-gnu
    INCLUDEPATH += /usr/include/SoapySDR
    INCLUDEPATH += /usr/include/osmosdr

    LIBS += -L/usr/local/lib \
    -lboost_system
    -lboost_program_options
    -lboost_thread

    LIBS += -lrt -lportaudio -lrtlsdr -lfftw3 -lhackrf -llog4cpp -lSoapySDR -lpthread -losmosdr
    LIBS += -lgnuradio-analog -lgnuradio-blocks -lgnuradio-digital -lgnuradio-filter -lgnuradio-fft -lgnuradio-runtime -lgnuradio-audio -lgnuradio-uhd -lgnuradio-osmosdr

    # INCLUDEPATH += /usr/lib/x86_64-linux-gnu
    # INCLUDEPATH += /usr/lib/arm-linux-gnueabihf
    # LIBS += -L/usr/lib/arm-linux-gnueabihf/ -lhackrf
    # PRE_TARGETDEPS += /usr/lib/arm-linux-gnueabihf/libhackrf.a
}

ios {
    QMAKE_INFO_PLIST = ./ios/Info.plist
    QMAKE_ASSET_CATALOGS = $$PWD/ios/Assets.xcassets
    QMAKE_ASSET_CATALOGS_APP_ICON = "AppIcon"
}

win32 {
    message("Win32 enabled")
    # RC_ICONS += $$\PWD\icons\hackrf.png

    INCLUDEPATH += $$PWD\include\libhackrf
    INCLUDEPATH += $$PWD\include\rtl-sdr
    # INCLUDEPATH += $$PWD\libs\fftw3-3
    INCLUDEPATH += $$PWD\libs\portaudio\include
    # INCLUDEPATH += $$PWD\include\osmosdr
    # INCLUDEPATH += $$PWD\include\soapysdr

    LIBS += -L$$PWD\libs\rtl-sdr -lrtlsdr
    LIBS += -L$$PWD\libs\hackrf -lhackrf
    # LIBS += -L$$PWD\libs\fftw3-3 -llibfftw3-3
    LIBS += -L$$PWD\libs\portaudio\lib\x64\Release -lportaudio_x64
    # LIBS += -L$$PWD\libs\osmosdr -losmosdr
    # LIBS += -L$$PWD\libs\soapysdr -lSoapyOsmoSDR

}


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    icons/hackrf.png
