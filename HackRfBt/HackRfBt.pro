QT = core
QT += bluetooth multimedia network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 cmdline

CONFIG += qwt

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
QMAKE_ASSET_CATALOGS_BUILD_PATH = $$PWD

SOURCES += \
        audiootputthread.cpp \
        gattserver.cpp \
        gnuradio.cpp \
        hackrfmanager.cpp \
        main.cpp \
        message.cpp \
        modulator.cpp

HEADERS += \
    IHackRFData.h \
    audiootputthread.h \
    gattserver.h \
    gnuradio.h \  
    hackrfmanager.h \
    modulator.h \
    message.h \   
    tcpclient.h \
    udpclient.h


macos {
    QMAKE_INFO_PLIST = ./macos/Info.plist
    QMAKE_ASSET_CATALOGS = $$PWD/macos/Assets.xcassets
    QMAKE_ASSET_CATALOGS_APP_ICON = "AppIcon"

    INCLUDEPATH += /usr/local/lib
    INCLUDEPATH += /opt/local/include 
    INCLUDEPATH += /usr/local/include
    INCLUDEPATH += $$PWD/gnuradio
    INCLUDEPATH += $$PWD/qwt

    INCLUDEPATH += /opt/homebrew/opt/rtl-sdr/include
    INCLUDEPATH += /opt/homebrew/opt/fftw/include
    INCLUDEPATH += /opt/homebrew/opt/portaudio/include
    INCLUDEPATH += /opt/homebrew/opt/hackrf/include/libhackrf

    LIBS += -L/usr/local/lib \
    -lboost_system
    -lboost_program_options
    -lboost_thread

    LIBS += -L/opt/local/lib/libgnuradio-analog.dylib
    LIBS += -L/opt/local/lib/libgnuradio-blocks.dylib
    LIBS += -L/opt/local/lib/libgnuradio-digital.dylib
    LIBS += -L/opt/local/lib/libgnuradio-filter.dylib
    LIBS += -L/opt/local/lib/libgnuradio-fft.dylib
    LIBS += -L/opt/local/lib/libgnuradio-runtime.dylib
    LIBS += -L/opt/local/lib/libgnuradio-audio.dylib
    LIBS += -L/opt/local/lib/libgnuradio-osmosdr.dylib
    LIBS += -L/opt/local/lib/libgnuradio-uhd.dylib

    LIBS += /opt/homebrew/opt/rtl-sdr/lib/librtlsdr.dylib /opt/homebrew/opt/fftw/lib/libfftw3.dylib /opt/homebrew/opt/portaudio/lib/libportaudio.dylib    
    LIBS += /opt/homebrew/opt/hackrf/lib/libhackrf.dylib

    LIBS += -L/opt/local/lib/libqwt.dylib
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
#    sudo apt-get install gnuradio

    INCLUDEPATH += /usr/lib
    INCLUDEPATH += /usr/include
    INCLUDEPATH += /usr/local/include
    INCLUDEPATH += /usr/local/lib
    INCLUDEPATH += /usr/include/libhackrf   
    INCLUDEPATH += /lib/x86_64-linux-gnu
    INCLUDEPATH += /usr/include/SoapySDR

    LIBS += -L/usr/local/lib \
    -lboost_system
    -lboost_program_options
    -lboost_thread

    INCLUDEPATH += /usr/include/osmosdr
    LIBS += -L/usr/lib -losmosdr

    LIBS += -lrt -lportaudio -lrtlsdr -lfftw3 -lhackrf -llog4cpp -lSoapySDR -lpthread
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
