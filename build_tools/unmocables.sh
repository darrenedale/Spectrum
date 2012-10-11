#!/bin/bash
#
# list all headers that don't include the Q_OBJECT macro (i.e. all those that
# don't require moc). this assists in generating the CMakeLists.txt file.
grep -L Q_OBJECT *.h | tr "\n" " "
