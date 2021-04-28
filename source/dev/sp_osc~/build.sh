PROJECT="sp_osc~"
XCODEPROJ=${PROJECT}.xcodeproj

if [[ -d ${XCODEPROJ} ]]
then
    rm -rf ${XCODEPROJ}
fi
xcodegen
xcodebuild -project ${XCODEPROJ}

