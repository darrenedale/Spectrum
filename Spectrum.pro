#-------------------------------------------------
#
# Project created by QtCreator 2012-03-20T15:10:25
#
#-------------------------------------------------

QT       += core gui

TARGET = Spectrum
TEMPLATE = app

# LIBS += /usr/lib/libreadline.so

SOURCES += spectrum_main.cpp\
        mainwindow.cpp \
    imagewidget.cpp \
    z80.cpp \
    z80iodevice.cpp \
    computer.cpp \
    cpu.cpp \
    spectrum.cpp \
    spectrumdisplaydevice.cpp \
    qspectrumdisplay.cpp \
    spectrumthread.cpp \
    qspectrumdebugwindow.cpp

HEADERS  += mainwindow.h \
    imagewidget.h \
    z80.h \
    z80_opcodes_plain.h \
    z80_opcodes.h \
    types.h \
    z80_opcodes_ed.h \
    z80_opcodes_cb.h \
    z80iodevice.h \
    computer.h \
    array.h \
    cpu.h \
    z80_opcodes_dd.h \
    z80_opcodes_fd.h \
    z80_opcodes_ddorfd.h \
    z80_opcodes_ddorfd_cb.h \
    spectrum.h \
    spectrumdisplaydevice.h \
    qspectrumdisplay.h \
    spectrumthread.h \
    qspectrumdebugwindow.h

OTHER_FILES += \
    plain_opcode_cycles.txt \
    z80_plain_opcode_cycles.inc \
    z80_ed_opcode_cycles.inc \
    ../../Reference/Computing/z80/z80time.txt \
    ../../Reference/Computing/z80/z80_instruction_set_summary.txt \
    ../../Reference/Computing/z80/z80_reference.txt \
    z80_ed_opcode_sizes.inc \
    hob/Z80.jad \
    hob/Z80.java \
    evenparitytable16.inc \
    evenparitytable8.inc \
    z80_plain_opcode_sizes.inc
