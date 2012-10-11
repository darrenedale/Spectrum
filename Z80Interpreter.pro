#-------------------------------------------------
#
# Project created by QtCreator 2012-03-20T15:10:25
#
#-------------------------------------------------

#QT       += core gui

TARGET = z80interpreter
TEMPLATE = app

LIBS += -lreadline

SOURCES += z80interpreter_main.cpp\
    z80.cpp \
    z80interpreter.cpp \
    z80iodevice.cpp \
    cpu.cpp

HEADERS  += \
    z80.h \
    z80_opcodes_plain.h \
    z80_opcodes.h \
    types.h \
    z80_opcodes_ed.h \
    z80_opcodes_cb.h \
    z80interpreter.h \
    z80iodevice.h \
    array.h \
    cpu.h \
    z80_opcodes_dd.h \
    z80_opcodes_fd.h \
    z80_opcodes_ddorfd.h \
    z80_opcodes_ddorfd_cb.h

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
