#!/bin/bash

TOPDIR=$(pwd)

#
# set resource.c for lite-wakeup
#
TARGET_RES=resource.c-fsmn-fbank24-30ms-75K
if [ ! -e "$TOPDIR/examples/lite-wakeup/resource.c" ]; then
    cd $TOPDIR/examples/lite-wakeup

    if [ -d res/$TARGET_RES ]; then
        $TOPDIR/tools/res2src/res2src --directory res/$TARGET_RES > resource.c
    else
        ln -s res/$TARGET_RES resource.c
    fi
fi

#
# set resource.c for lite-uda
#
TARGET_RES=LUDA_FIXED_WB_2mic_155mm_0605_3EntBeam.c
if [ ! -e "$TOPDIR/examples/lite-uda/resourceWB.c" ]; then
    cd $TOPDIR/examples/lite-uda/
    ln -s WB/$TARGET_RES resourceWB.c
fi
