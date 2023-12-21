#!/bin/bash

# Install the Vulkan SDK
source package_installed_dpkg.sh
if ! package_installed "vulkan-sdk"; then
  echo "No Vulkan SDK installed, installing..."

  wget -qO- https://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo tee /etc/apt/trusted.gpg.d/lunarg.asc
  sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-jammy.list http://packages.lunarg.com/vulkan/lunarg-vulkan-jammy.list
  sudo apt update
  sudo apt install vulkan-sdk
fi
