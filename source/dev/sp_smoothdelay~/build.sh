PROJECT="sp_smoothdelay~"
XCODEPROJ=${PROJECT}.xcodeproj

if [[ -d ${XCODEPROJ} ]]
then
    rm -rf ${XCODEPROJ}
fi
xcodegen
xcodebuild -project ${XCODEPROJ}