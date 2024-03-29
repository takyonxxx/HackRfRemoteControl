#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: HackRf
# GNU Radio version: 3.10.1.1

from packaging.version import Version as StrictVersion

if __name__ == '__main__':
    import ctypes
    import sys
    if sys.platform.startswith('linux'):
        try:
            x11 = ctypes.cdll.LoadLibrary('libX11.so')
            x11.XInitThreads()
        except:
            print("Warning: failed to XInitThreads()")

from PyQt5 import Qt
from gnuradio import qtgui
from gnuradio.filter import firdes
import sip
from gnuradio import analog
from gnuradio import audio
from gnuradio import blocks
from gnuradio import filter
from gnuradio import gr
from gnuradio.fft import window
import sys
import signal
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio import eng_notation
from gnuradio.qtgui import Range, RangeWidget
from PyQt5 import QtCore
import osmosdr
import time



from gnuradio import qtgui

class hackrf_osmosdr(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "HackRf", catch_exceptions=True)
        Qt.QWidget.__init__(self)
        self.setWindowTitle("HackRf")
        qtgui.util.check_set_qss()
        try:
            self.setWindowIcon(Qt.QIcon.fromTheme('gnuradio-grc'))
        except:
            pass
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

        self.settings = Qt.QSettings("GNU Radio", "hackrf_osmosdr")

        try:
            if StrictVersion(Qt.qVersion()) < StrictVersion("5.0.0"):
                self.restoreGeometry(self.settings.value("geometry").toByteArray())
            else:
                self.restoreGeometry(self.settings.value("geometry"))
        except:
            pass

        ##################################################
        # Variables
        ##################################################
        self.sample_rate = sample_rate = 20e6
        self.channel_width = channel_width = 300e3
        self.center_freq = center_freq = 100e6
        self.audio_gain = audio_gain = 0.75

        ##################################################
        # Blocks
        ##################################################
        self._center_freq_range = Range(80e6, 108e6, 0.1, 100e6, 200)
        self._center_freq_win = RangeWidget(self._center_freq_range, self.set_center_freq, "Frequency", "counter_slider", float, QtCore.Qt.Horizontal)
        self.top_layout.addWidget(self._center_freq_win)
        self._audio_gain_range = Range(0, 1, 0.05, 0.75, 200)
        self._audio_gain_win = RangeWidget(self._audio_gain_range, self.set_audio_gain, "Volume", "counter_slider", float, QtCore.Qt.Horizontal)
        self.top_layout.addWidget(self._audio_gain_win)
        self.rational_resampler_xxx_0 = filter.rational_resampler_ccc(
                interpolation=12,
                decimation=6,
                taps=[],
                fractional_bw=0)
        self.osmosdr_source_0 = osmosdr.source(
            args="numchan=" + str(1) + " " + ""
        )
        self.osmosdr_source_0.set_time_unknown_pps(osmosdr.time_spec_t())
        self.osmosdr_source_0.set_sample_rate(sample_rate)
        self.osmosdr_source_0.set_center_freq(center_freq, 0)
        self.osmosdr_source_0.set_freq_corr(0, 0)
        self.osmosdr_source_0.set_dc_offset_mode(0, 0)
        self.osmosdr_source_0.set_iq_balance_mode(0, 0)
        self.osmosdr_source_0.set_gain_mode(False, 0)
        self.osmosdr_source_0.set_gain(0, 0)
        self.osmosdr_source_0.set_if_gain(40, 0)
        self.osmosdr_source_0.set_bb_gain(60, 0)
        self.osmosdr_source_0.set_antenna('', 0)
        self.osmosdr_source_0.set_bandwidth(0, 0)
        self.low_pass_filter = filter.fir_filter_ccf(
            int(sample_rate/channel_width),
            firdes.low_pass(
                1,
                sample_rate,
                150e3,
                10e3,
                window.WIN_HAMMING,
                6.76))
        self.freq_sink = qtgui.freq_sink_c(
            1024, #size
            window.WIN_BLACKMAN_hARRIS, #wintype
            center_freq, #fc
            sample_rate, #bw
            "", #name
            1,
            None # parent
        )
        self.freq_sink.set_update_time(0.10)
        self.freq_sink.set_y_axis(-140, 10)
        self.freq_sink.set_y_label('Relative Gain', 'dB')
        self.freq_sink.set_trigger_mode(qtgui.TRIG_MODE_FREE, 0.0, 0, "")
        self.freq_sink.enable_autoscale(False)
        self.freq_sink.enable_grid(False)
        self.freq_sink.set_fft_average(0.05)
        self.freq_sink.enable_axis_labels(True)
        self.freq_sink.enable_control_panel(True)
        self.freq_sink.set_fft_window_normalized(False)



        labels = ['', '', '', '', '',
            '', '', '', '', '']
        widths = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        colors = ["blue", "red", "green", "black", "cyan",
            "magenta", "yellow", "dark red", "dark green", "dark blue"]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0]

        for i in range(1):
            if len(labels[i]) == 0:
                self.freq_sink.set_line_label(i, "Data {0}".format(i))
            else:
                self.freq_sink.set_line_label(i, labels[i])
            self.freq_sink.set_line_width(i, widths[i])
            self.freq_sink.set_line_color(i, colors[i])
            self.freq_sink.set_line_alpha(i, alphas[i])

        self._freq_sink_win = sip.wrapinstance(self.freq_sink.qwidget(), Qt.QWidget)
        self.top_layout.addWidget(self._freq_sink_win)
        self.freq_sink.set_block_alias("Spectrum")
        self.blocks_multiply_const_vxx_0 = blocks.multiply_const_ff(audio_gain)
        self.audio_sink = audio.sink(48000, '', True)
        self.analog_wfm_rcv_0 = analog.wfm_rcv(
        	quad_rate=600e3,
        	audio_decimation=12,
        )


        ##################################################
        # Connections
        ##################################################
        self.connect((self.analog_wfm_rcv_0, 0), (self.blocks_multiply_const_vxx_0, 0))
        self.connect((self.blocks_multiply_const_vxx_0, 0), (self.audio_sink, 0))
        self.connect((self.low_pass_filter, 0), (self.rational_resampler_xxx_0, 0))
        self.connect((self.osmosdr_source_0, 0), (self.freq_sink, 0))
        self.connect((self.osmosdr_source_0, 0), (self.low_pass_filter, 0))
        self.connect((self.rational_resampler_xxx_0, 0), (self.analog_wfm_rcv_0, 0))


    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "hackrf_osmosdr")
        self.settings.setValue("geometry", self.saveGeometry())
        self.stop()
        self.wait()

        event.accept()

    def get_sample_rate(self):
        return self.sample_rate

    def set_sample_rate(self, sample_rate):
        self.sample_rate = sample_rate
        self.freq_sink.set_frequency_range(self.center_freq, self.sample_rate)
        self.low_pass_filter.set_taps(firdes.low_pass(1, self.sample_rate, 150e3, 10e3, window.WIN_HAMMING, 6.76))
        self.osmosdr_source_0.set_sample_rate(self.sample_rate)

    def get_channel_width(self):
        return self.channel_width

    def set_channel_width(self, channel_width):
        self.channel_width = channel_width

    def get_center_freq(self):
        return self.center_freq

    def set_center_freq(self, center_freq):
        self.center_freq = center_freq
        self.freq_sink.set_frequency_range(self.center_freq, self.sample_rate)
        self.osmosdr_source_0.set_center_freq(self.center_freq, 0)

    def get_audio_gain(self):
        return self.audio_gain

    def set_audio_gain(self, audio_gain):
        self.audio_gain = audio_gain
        self.blocks_multiply_const_vxx_0.set_k(self.audio_gain)




def main(top_block_cls=hackrf_osmosdr, options=None):

    if StrictVersion("4.5.0") <= StrictVersion(Qt.qVersion()) < StrictVersion("5.0.0"):
        style = gr.prefs().get_string('qtgui', 'style', 'raster')
        Qt.QApplication.setGraphicsSystem(style)
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
