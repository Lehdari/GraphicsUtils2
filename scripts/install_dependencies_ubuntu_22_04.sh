#!/bin/bash

SCRIPT_DIR="$(cd -P "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source $SCRIPT_DIR/package_installed_dpkg.sh

# Install the Vulkan SDK
if ! package_installed "vulkan-sdk"; then
  echo "No Vulkan SDK installed, installing..."

  wget -qO- https://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo tee /etc/apt/trusted.gpg.d/lunarg.asc
  sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-jammy.list http://packages.lunarg.com/vulkan/lunarg-vulkan-jammy.list
  sudo apt update
  sudo apt install vulkan-sdk
fi

# Install Eigen 3
if ! package_installed "libeigen3-dev"; then
  echo "Eigen 3 not installed, installing..."
  sudo apt install libeigen3-dev
fi

