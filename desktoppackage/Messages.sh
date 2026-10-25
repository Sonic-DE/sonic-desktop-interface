#! /usr/bin/env bash
$XGETTEXT `find . -name \*.qml` -o $podir/plasma_shell_org.sonicde.desktop.pot
rm -f rc.cpp
