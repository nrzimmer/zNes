#-------------------------------------------------
#
# Project created by QtCreator 2018-05-31T17:13:54
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = zNes
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    mos6502.cpp


HEADERS += \
        mainwindow.h \
    mos6502.h


FORMS += \
        mainwindow.ui

nasm.name = nasm ${QMAKE_FILE_IN}
nasm.input = ASM_FILES
nasm.variable_out = OBJECTS
nasm.commands = nasm -f elf64 ${QMAKE_FILE_IN} -o ${QMAKE_FILE_OUT}
nasm.output = ${QMAKE_VAR_OBJECTS_DIR}${QMAKE_FILE_IN_BASE}$${first(QMAKE_EXT_OBJ)}
nasm.CONFIG += target_predeps

QMAKE_EXTRA_COMPILERS  += nasm
ASM_FILES += nes6502.asm
