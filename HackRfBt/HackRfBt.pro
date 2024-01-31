QT = core
QT += bluetooth multimedia network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 cmdline

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
QMAKE_ASSET_CATALOGS_BUILD_PATH = $$PWD

win32:RC_ICONS += $$\PWD\icons\hackrf.png

SOURCES += \
        audiootputthread.cpp \
        gattserver.cpp \
        gui/spectrum.cc \
        gui/spectrumview.cc \
        hackrfmanager.cpp \
        main.cpp \
        message.cpp \
        receiver/audiopostproc.cc \
        receiver/configuration.cc \
        receiver/demodulator.cc \
        receiver/receiver.cc \
        receiver/rtldatasource.cc \
        receiver/source.cc \
        sdr/aprs.cc \
        sdr/ax25.cc \
        sdr/baudot.cc \
        sdr/bch31_21.cc \
        sdr/buffer.cc \
        sdr/exception.cc \
        sdr/fsk.cc \
        sdr/logger.cc \
        sdr/node.cc \
        sdr/options.cc \
        sdr/pocsag.cc \
        sdr/portaudio.cc \
        sdr/psk31.cc \
        sdr/queue.cc \
        sdr/rtlsource.cc \
        sdr/sha1.cc \
        sdr/traits.cc \
        sdr/utils.cc \
        sdr/wavfile.cc \
        sdrmanager.cpp

HEADERS += \
    audiootputthread.h \
    gattserver.h \
    gui/gui.hh \
    gui/spectrum.hh \
    gui/spectrumview.hh \
    hackrfmanager.h \
    receiver/audiopostproc.hh \
    receiver/configuration.hh \
    receiver/demodulator.hh \
    receiver/receiver.hh \
    receiver/rtldatasource.hh \
    receiver/source.hh \
    message.h \
    sdr/aprs.hh \
    sdr/autocast.hh \
    sdr/ax25.hh \
    sdr/baseband.hh \
    sdr/baudot.hh \
    sdr/bch31_21.hh \
    sdr/buffer.hh \
    sdr/buffernode.hh \
    sdr/combine.hh \
    sdr/config.hh \
    sdr/demod.hh \
    sdr/exception.hh \
    sdr/fftplan.hh \
    sdr/fftplan_fftw3.hh \
    sdr/fftplan_native.hh \
    sdr/filternode.hh \
    sdr/firfilter.hh \
    sdr/freqshift.hh \
    sdr/fsk.hh \
    sdr/interpolate.hh \
    sdr/logger.hh \
    sdr/math.hh \
    sdr/node.hh \
    sdr/operators.hh \
    sdr/options.hh \
    sdr/pocsag.hh \
    sdr/portaudio.hh \
    sdr/psk31.hh \
    sdr/queue.hh \
    sdr/rtlsource.hh \
    sdr/sdr.hh \
    sdr/sha1.hh \
    sdr/siggen.hh \
    sdr/streamsource.hh \
    sdr/subsample.hh \
    sdr/traits.hh \
    sdr/utils.hh \
    sdr/wavfile.hh \
    sdrmanager.h \
    tcpclient.h \
    udpclient.h


macos {
    QMAKE_INFO_PLIST = ./macos/Info.plist
    QMAKE_ASSET_CATALOGS = $$PWD/macos/Assets.xcassets
    QMAKE_ASSET_CATALOGS_APP_ICON = "AppIcon"

    INCLUDEPATH += /opt/homebrew/opt/rtl-sdr/include
    INCLUDEPATH += /opt/homebrew/opt/fftw/include
    INCLUDEPATH += /opt/homebrew/opt/portaudio/include
    LIBS += /opt/homebrew/opt/rtl-sdr/lib/librtlsdr.dylib /opt/homebrew/opt/fftw/lib/libfftw3.dylib /opt/homebrew/opt/portaudio/lib/libportaudio.dylib

    INCLUDEPATH += /opt/homebrew/opt/hackrf/include/libhackrf
    LIBS += /opt/homebrew/opt/hackrf/lib/libhackrf.dylib

#    INCLUDEPATH += /usr/local/lib
#    INCLUDEPATH += /usr/local/include
#    INCLUDEPATH += /usr/local/Cellar/boost/1.80.0/include
#    INCLUDEPATH += /usr/local/Cellar/gnuradio/3.10.4.0/include

#    LIBS += -L/usr/local/lib \
#    -lboost_system
#    -lboost_program_options
#    -lboost_thread

#    LIBS += -L/usr/local/Cellar/gnuradio/3.10.4.0/lib/libgnuradio-analog.dylib
#    LIBS += -L/usr/local/Cellar/gnuradio/3.10.4.0/lib/libgnuradio-blocks.dylib
#    LIBS += -L/usr/local/Cellar/gnuradio/3.10.4.0/lib/libgnuradio-digital.dylib
#    LIBS += -L/usr/local/Cellar/gnuradio/3.10.4.0/lib/libgnuradio-filter.dylib
#    LIBS += -L/usr/local/Cellar/gnuradio/3.10.4.0/lib/libgnuradio-fft.dylib
#    LIBS += -L/usr/local/Cellar/gnuradio/3.10.4.0/lib/libgnuradio-runtime.dylib
#    LIBS += -L/usr/local/Cellar/gnuradio/3.10.4.0/lib/libgnuradio-audio.dylib

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
#    nmap -sn 192.168.1.0/24

#    start_hackrf.sh
#    #!/bin/bash
#    sudo chown root.root /home/pi/HackRfBt/HackRfBt
#    sudo chmod 4755 /home/pi/HackRfBt/HackRfBt
#    cd /home/pi/HackRfBt
#    sudo ./HackRfBt

#    chmod +x start_hackrf.sh

#    sudo nano /etc/systemd/system/hackrf.service

#    [Unit]
#    Description=HackRF service
#    After=multi-user.target

#    [Service]
#    ExecStartPre=/bin/sleep 10
#    ExecStart=/bin/bash /home/pi/start_hackrf.sh
#    WorkingDirectory=/home/pi/HackRfBt
#    StandardOutput=inherit
#    StandardError=inherit
#    Restart=always
#    User=pi

#    [Install]
#    WantedBy=multi-user.target

#    sudo chmod 644 /lib/systemd/system/hackrf.service
#    sudo systemctl daemon-reload
#    sudo systemctl enable hackrf.service
#    sudo systemctl start hackrf.service
#    sudo systemctl status hackrf.service

    INCLUDEPATH += /usr/lib
    INCLUDEPATH += /usr/include
    INCLUDEPATH += /usr/local/lib
    INCLUDEPATH += /usr/include/libhackrf
    INCLUDEPATH += /usr/lib/x86_64-linux-gnu
    INCLUDEPATH += /usr/lib/arm-linux-gnueabihf
    LIBS += -lrt -lportaudio -lrtlsdr -lfftw3 -lhackrf

    # LIBS += -L/usr/lib/arm-linux-gnueabihf/ -lhackrf
    # INCLUDEPATH += /usr/include/libhackrf
    # PRE_TARGETDEPS += /usr/lib/arm-linux-gnueabihf/libhackrf.a
}

ios {
    QMAKE_INFO_PLIST = ./ios/Info.plist
    QMAKE_ASSET_CATALOGS = $$PWD/ios/Assets.xcassets
    QMAKE_ASSET_CATALOGS_APP_ICON = "AppIcon"
}


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
