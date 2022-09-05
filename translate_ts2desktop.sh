#!/bin/bash

DESKTOP_TEMP_FILE=dde-device-formatter.desktop.tmp
DESKTOP_SOURCE_FILE=dde-device-formatter.desktop
DESKTOP_DEST_FILE=dde-device-formatter.desktop
DESKTOP_TS_DIR=translations/desktop/

deepin-desktop-ts-convert ts2desktop $DESKTOP_SOURCE_FILE $DESKTOP_TS_DIR $DESKTOP_TEMP_FILE
mv $DESKTOP_TEMP_FILE $DESKTOP_DEST_FILE
