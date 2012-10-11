#!/bin/bash
#
# list all headers that include the Q_OBJECT macro (i.e. all those that require
# moc). this assists in generating the CMakeLists.txt file.
grep -l Q_OBJECT *.h | tr "\n" " "
