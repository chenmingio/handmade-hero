echo "Building Handmade Hero CPU"

APP_NAME="CPUHero"
APP_BUNDLE_NAME="CPUHero.app"


mkdir -p ../../build/mac_os/
pushd ../../build/mac_os/ || exit

MAC_PLATFORM_LAYER_PATH="../../code/mac_platform"
RESOURCE_PATH="${MAC_PLATFORM_LAYER_PATH}/resources"
BUNDLE_RESOURCE_PATH="${APP_BUNDLE_NAME}/Contents/Resources"

INCLUDED_FRAMEWORKS="-framework AppKit -framework IOKit -framework AudioToolbox"

#clang -g "$INCLUDED_FRAMEWORKS" -o $APP_NAME ${MAC_PLATFORM_LAYER_PATH}/osx_main.mm || exit
clang -g  "-DHANDMADE_INTERNAL=1" -framework AppKit -framework IOKit -framework AudioToolbox -o $APP_NAME ${MAC_PLATFORM_LAYER_PATH}/osx_main.mm || exit

rm -rf $APP_BUNDLE_NAME
mkdir -p ${BUNDLE_RESOURCE_PATH}

# copy executable in to .app folder
cp $APP_NAME ${APP_BUNDLE_NAME}/${APP_NAME}
# copy info.plist from code resources to .app folder
cp ${RESOURCE_PATH}/Info.plist ${APP_BUNDLE_NAME}/Info.plist
cp ${RESOURCE_PATH}/test_background.bmp ${BUNDLE_RESOURCE_PATH}

popd || exit

echo "Finished Building ${APP_NAME}"
