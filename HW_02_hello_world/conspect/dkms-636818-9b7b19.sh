#!/bin/bash

PACKAGE_NAME="my_module"
PACKAGE_VERSION="1.0"

function install_module()
{
    local dkms_src="/usr/src/$PACKAGE_NAME-$PACKAGE_VERSION"
    mkdir -p $dkms_src

    cp Makefile $dkms_src
    cp my_module.c $dkms_src
    cp dkms.conf $dkms_src
    cp my_module.conf $dkms_src

    sed -i -e "s/P_NAME/$PACKAGE_NAME/g" \
        -i -e "s/P_VERSION"/$PACKAGE_VERSION/g \
        "$dkms_src/dkms.conf"

    dkms install "$PACKAGE_NAME/$PACKAGE_VERSION"
}

function remove_module()
{
    local dkms_src="/usr/src/$PACKAGE_NAME-$PACKAGE_VERSION"
    dkms remove "$PACKAGE_NAME/$PACKAGE_VERSION"
    rm -rf $dkms_src
}

case "$1" in 
    install)
        install_module
        ;;
    remove)
        remove_module
        ;;
    *)
        echo "Try again"
        exit -1
esac