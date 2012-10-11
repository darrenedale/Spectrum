#!/bin/bash
# create the CMakeLists.txt file
#
# this script processes a template (CMakeLists.template), and inserts the
# appropriate list of file names in various places to produce a working
# CMakeLists.txt file for the project
#
# this script must be run from the main project directory
SOURCES=`ls -1 *.cpp | tr "\n" " "`
MOCABLES=`./build_tools/mocables.sh`
UNMOCABLES=`./build_tools/unmocables.sh`
UIS=`ls -1 *.ui | tr "\n" " "`
RESOURCES=`ls -1 *.qrc | tr "\n" " "`
TEMPLATE="./build_tools/CMakeLists.template"
CMAKELISTS="`pwd`/CMakeLists.txt"

echo "Updating ${CMAKELISTS}"
cp ${TEMPLATE} ${CMAKELISTS}

echo "...setting MOC headers"
sed -i "s/%mocables%/${MOCABLES}/g" ${CMAKELISTS}

echo "...setting other headers"
sed -i "s/%unmocables%/${UNMOCABLES}/g" ${CMAKELISTS}

echo "...setting UIC files"
sed -i "s/%uifiles%/${UIS}/g" ${CMAKELISTS}

echo "...setting source code files"
sed -i "s/%sources%/${SOURCES}/g" ${CMAKELISTS}

echo "...setting QRC resource files"
sed -i "s/%resources%/${RESOURCES}/g" ${CMAKELISTS}

echo "Done."
