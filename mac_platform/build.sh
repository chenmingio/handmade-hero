echo "Building Handmade Hero CPU"

APP_NAME="CPUHero"
APP_BUNDLE_NAME="CPUHero.app"


mkdir -p ../../build/mac_os/
pushd ../../build/mac_os/ || exit

MAC_PLATFORM_LAYER_PATH="../../code/mac_platform"
RESOURCE_PATH="${MAC_PLATFORM_LAYER_PATH}/resources"
BUNDLE_RESOURCE_PATH="${APP_BUNDLE_NAME}/Contents/Resources"

COMPILER_WARNING_FLAGS="-Werror
                        -Weverything"

# NOTE: (TED) -Wpadded can be useful when we want to know which structs
#             are not properly padded. We have disabled it for now.

DISABLED_WARNINGS="-Wno-old-style-cast
                   -Wno-cast-qual
                   -Wno-cast-align
                   -Wno-gnu-anonymous-struct
                   -Wno-nested-anon-types
                   -Wno-padded
                   -Wno-unused-variable
                   -Wno-unused-but-set-variable
                   -Wno-unused-parameter
                   -Wno-unused-function
                   -Wno-pedantic
                   -Wno-missing-prototypes
                   -Wno-nullable-to-nonnull-conversion
                   -Wno-c++11-long-long
                   -Wno-poison-system-directories
                   -Wno-deprecated-declarations
                   "

INCLUDED_FRAMEWORKS="-framework AppKit
                     -framework IOKit
                     -framework AudioToolbox"

COMMON_COMPILER_FLAGS="${COMPILER_WARNING_FLAGS}
                       ${DISABLED_WARNINGS}
                       -DHANDMADE_INTERNAL=1
                       ${INCLUDED_FRAMEWORKS}"

clang -fsanitize=address -g -o GameCode.dylib ${COMMON_COMPILER_FLAGS} -dynamiclib ../../code/game_library/handmade.cpp || exit
clang -fsanitize=address -g ${COMMON_COMPILER_FLAGS} -o $APP_NAME ${MAC_PLATFORM_LAYER_PATH}/osx_main.mm || exit

rm -rf $APP_BUNDLE_NAME
mkdir -p ${BUNDLE_RESOURCE_PATH}

# copy executable in to .app folder
cp $APP_NAME ${APP_BUNDLE_NAME}/${APP_NAME}
# copy info.plist from code resources to .app folder
cp ${RESOURCE_PATH}/Info.plist ${APP_BUNDLE_NAME}/Info.plist
cp ${RESOURCE_PATH}/test_background.bmp ${BUNDLE_RESOURCE_PATH}
cp GameCode.dylib ${BUNDLE_RESOURCE_PATH}/GameCode.dylib
cp -r GameCode.dylib.dSYM ${BUNDLE_RESOURCE_PATH}/GameCode.dylib.dSYM

popd || exit

echo "Finished Building ${APP_NAME}"
