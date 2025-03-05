QT       += core gui widgets
TARGET    = TerminalApp
TEMPLATE  = app

# Ajoutez cette ligne si QTerminalWidget est une bibliothèque externe
INCLUDES += ..
LIBS += -L.. -lqtterminalwidget

SOURCES  += main.cpp
