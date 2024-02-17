#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: HackRf
# GNU Radio version: 3.10.9.2

from PyQt5 import Qt
from gnuradio import qtgui
from gnuradio import analog
from gnuradio import audio
from gnuradio import blocks
from gnuradio import filter
from gnuradio.filter import firdes
from gnuradio import gr
from gnuradio.fft import window
import sys
import signal
from PyQt5 import Qt
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio import eng_notation
from gnuradio import soapy



class hackrf_soapy(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "HackRf", catch_exceptions=True)
        Qt.QWidget.__init__(self)
        self.setWindowTitle("HackRf")
        qtgui.util.check_set_qss()
        try:
            self.setWindowIcon(Qt.QIcon.fromTheme('gnuradio-grc'))
        except BaseException as exc:
            print(f"Qt GUI: Could not set Icon: {str(exc)}", file=sys.stderr)
        self.top_scroll_layout = Qt.QVBoxLayout()
        self.setLayout(self.top_scroll_layout)
        self.top_scroll = Qt.QScrollArea()
        self.top_scroll.setFrameStyle(Qt.QFrame.NoFrame)
        self.top_scroll_layout.addWidget(self.top_scroll)
        self.top_scroll.setWidgetResizable(True)
        self.top_widget = Qt.QWidget()
        self.top_scroll.setWidget(self.top_widget)
        self.top_layout = Qt.QVBoxLayout(self.top_widget)
        self.top_grid_layout = Qt.QGridLayout()
        self.top_layout.addLayout(self.top_grid_layout)

        self.settings = Qt.QSettings("GNU Radio", "hackrf_soapy")

        try:
            geometry = self.settings.value("geometry")
            if geometry:
                self.restoreGeometry(geometry)
        except BaseException as exc:
            print(f"Qt GUI: Could not restore geometry: {str(exc)}", file=sys.stderr)

        ##################################################
        # Variables
        ##################################################
        self.samp_rate = samp_rate = 20e6
        self.center_freq = center_freq = 100e6
        self.audio_gain = audio_gain = 0.75

        ##################################################
        # Blocks
        ##################################################

        self.rational_resampler = filter.rational_resampler_ccc(
                interpolation=24,
                decimation=10,
                taps=[],
                fractional_bw=0)
        self.multiply_const = blocks.multiply_const_ff(audio_gain)
        self.low_pass_filter = filter.fir_filter_ccf(
            100,
            firdes.low_pass(
                1,
                samp_rate,
                150e3,
                100000,
                window.WIN_HAMMING,
                6.76))
        self.hackrf_source = None
        dev = 'driver=hackrf'
        stream_args = ''
        tune_args = ['']
        settings = ['']

        self.hackrf_source = soapy.source(dev, "fc32", 1, 'driver=hackrf',
                                  stream_args, tune_args, settings)
        self.hackrf_source.set_sample_rate(0, samp_rate)
        self.hackrf_source.set_bandwidth(0, 0)
        self.hackrf_source.set_frequency(0, center_freq)
        self.hackrf_source.set_gain(0, 'AMP', False)
        self.hackrf_source.set_gain(0, 'LNA', min(max(40, 0.0), 40.0))
        self.hackrf_source.set_gain(0, 'VGA', min(max(40, 0.0), 62.0))
        self.fm_demod = analog.fm_demod_cf(
        	channel_rate=480e3,
        	audio_decim=10,
        	deviation=75000,
        	audio_pass=15000,
        	audio_stop=16000,
        	gain=1.0,
        	tau=(75e-6),
        )
        self.audio_sink = audio.sink(44100, '', True)


        ##################################################
        # Connections
        ##################################################
        self.connect((self.fm_demod, 0), (self.multiply_const, 0))
        self.connect((self.hackrf_source, 0), (self.low_pass_filter, 0))
        self.connect((self.low_pass_filter, 0), (self.rational_resampler, 0))
        self.connect((self.multiply_const, 0), (self.audio_sink, 0))
        self.connect((self.rational_resampler, 0), (self.fm_demod, 0))


    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "hackrf_soapy")
        self.settings.setValue("geometry", self.saveGeometry())
        self.stop()
        self.wait()

        event.accept()

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.hackrf_source.set_sample_rate(0, self.samp_rate)
        self.low_pass_filter.set_taps(firdes.low_pass(1, self.samp_rate, 150e3, 100000, window.WIN_HAMMING, 6.76))

    def get_center_freq(self):
        return self.center_freq

    def set_center_freq(self, center_freq):
        self.center_freq = center_freq
        self.hackrf_source.set_frequency(0, self.center_freq)

    def get_audio_gain(self):
        return self.audio_gain

    def set_audio_gain(self, audio_gain):
        self.audio_gain = audio_gain
        self.multiply_const.set_k(self.audio_gain)




def main(top_block_cls=hackrf_soapy, options=None):

    qapp = Qt.QApplication(sys.argv)

    tb = top_block_cls()

    tb.start()

    tb.show()

    def sig_handler(sig=None, frame=None):
        tb.stop()
        tb.wait()

        Qt.QApplication.quit()

    signal.signal(signal.SIGINT, sig_handler)
    signal.signal(signal.SIGTERM, sig_handler)

    timer = Qt.QTimer()
    timer.start(500)
    timer.timeout.connect(lambda: None)

    qapp.exec_()

if __name__ == '__main__':
    main()
