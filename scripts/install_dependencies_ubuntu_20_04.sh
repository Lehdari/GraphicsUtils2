#!/bin/bash

# Install the Vulkan SDK
source package_installed_dpkg.sh
if ! package_installed "vulkan-sdk"; then
  echo "No Vulkan SDK installed, installing..."

  wget -qO - http://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo apt-key add -
  sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-focal.list http://packages.lunarg.com/vulkan/lunarg-vulkan-focal.list
  sudo apt update
  sudo apt install vulkan-sdk
fi
