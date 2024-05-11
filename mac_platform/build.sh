echo "Building Handmade Hero CPU"

APP_NAME="CPUHero"
APP_BUNDLE_NAME="CPUHero.app"


mkdir -p ../../build/mac_os/
pushd ../../build/mac_os/ || exit

MAC_PLATFORM_LAYER_PATH="../../code/mac_platform"

INCLUDED_FRAMEWORKS="-framework AppKit -framework IOKit -framework AudioToolbox"

#clang -g "$INCLUDED_FRAMEWORKS" -o $APP_NAME ${MAC_PLATFORM_LAYER_PATH}/osx_main.mm || exit
clang -g -framework AppKit -framework IOKit -framework AudioToolbox -o $APP_NAME ${MAC_PLATFORM_LAYER_PATH}/osx_main.mm || exit

rm -rf $APP_BUNDLE_NAME
mkdir -p ${APP_BUNDLE_NAME}/Contents/Resources

# copy executable in to .app folder
cp $APP_NAME ${APP_BUNDLE_NAME}/${APP_NAME}
# copy info.plist from code resources to .app folder
cp ${MAC_PLATFORM_LAYER_PATH}/resources/Info.plist ${APP_BUNDLE_NAME}/Contents/Info.plist

popd || exit

echo "Finished Building ${APP_NAME}"
