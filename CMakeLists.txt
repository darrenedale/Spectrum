CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(spectrum)
FIND_PACKAGE(Qt4 REQUIRED)

#source code files
SET(spectrum_SOURCES computer.cpp cpu.cpp imagewidget.cpp mainwindow.cpp qspectrumdebugwindow.cpp qspectrumdisplay.cpp spectrum.cpp spectrumdisplaydevice.cpp spectrum_main.cpp spectrumthread.cpp z80.cpp z80interpreter.cpp z80interpreter_main.cpp z80iodevice.cpp )
SET(z80interpreter_SOURCES computer.cpp cpu.cpp imagewidget.cpp mainwindow.cpp qspectrumdebugwindow.cpp qspectrumdisplay.cpp spectrum.cpp spectrumdisplaydevice.cpp spectrum_main.cpp spectrumthread.cpp z80.cpp z80interpreter.cpp z80interpreter_main.cpp z80iodevice.cpp )

FOREACH(item ${spectrum_SOURCES})
	STRING(REGEX MATCH "z80interpreter.*" isMatch ${item})

	IF(isMatch)
		LIST(REMOVE_ITEM spectrum_SOURCES ${item})
	ENDIF(isMatch)
ENDFOREACH(item)

LIST(REMOVE_ITEM z80interpreter_SOURCES "spectrum_main.cpp")

MESSAGE(STATUS "Spectrum sources: ${spectrum_SOURCES}")
MESSAGE(STATUS "Z80Interpreter sources: ${z80interpreter_SOURCES}")
MESSAGE(FATAL_ERROR "Stopping now.")

# files that need to be processed by uic
SET(spectrum_UI_FILES )

# headers that need to be processed by moc
SET(spectrum_MOC_HEADERS imagewidget.h mainwindow.h qspectrumdebugwindow.h qspectrumdisplay.h spectrumthread.h )

# all headers
SET(spectrum_HEADERS ${spectrum_MOC_HEADERS} array.h computer.h cpu.h spectrumdisplaydevice.h spectrum.h types.h z80.h z80interpreter.h z80iodevice.h z80_opcodes_cb.h z80_opcodes_dd.h z80_opcodes_ddorfd_cb.h z80_opcodes_ddorfd.h z80_opcodes_ed.h z80_opcodes_fd.h z80_opcodes.h z80_opcodes_plain.h )

# resource files
SET(spectrum_RESOURCES )

# run moc on the moc headers and get the generated source files
QT4_WRAP_CPP(spectrum_MOC_SOURCES ${spectrum_MOC_HEADERS})

# run uic on the ui files and get the generated source files
QT4_WRAP_UI(spectrum_UI_SOURCES ${spectrum_UI_FILES})
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

# run qrc on the resource files and get the generated source files
QT4_ADD_RESOURCES(spectrum_RESOURCE_SOURCES ${spectrum_RESOURCES})

#build the application
INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})

ADD_EXECUTABLE(spectrum ${spectrum_SOURCES} ${spectrum_MOC_SOURCES} ${spectrum_UI_SOURCES} ${spectrum_RESOURCE_SOURCES})
ADD_EXECUTABLE(z80interpreter ${z80interpreter_SOURCES} ${spectrum_MOC_SOURCES})
TARGET_LINK_LIBRARIES(spectrum ${QT_LIBRARIES})
TARGET_LINK_LIBRARIES(z80interpreter ${QT_LIBRARIES})
