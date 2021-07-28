#!/usr/bin/env bash

PROJECT="sp_compressor~"
XCODEPROJ=${PROJECT}.xcodeproj

if [[ -d ${XCODEPROJ} ]]
then
    rm -rf ${XCODEPROJ}
fi
xcodegen
xcodebuild -project ${XCODEPROJ}