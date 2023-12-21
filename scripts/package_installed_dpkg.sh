#!/bin/bash

function package_installed {
  REQUIRED_PKG="$1"
  PKG_OK=$(dpkg-query -W --showformat='${Status}\n' $REQUIRED_PKG|grep "install ok installed")
  echo Checking for $REQUIRED_PKG: $PKG_OK
  if [ "" = "$PKG_OK" ]; then
    return 1
  else
    return 0
  fi
}