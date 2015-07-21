#!/bin/bash
set -e
echo 'pBuild 1.0'
echo 'Installing Pebble SDK and its Dependencies...'

cd ~ 

# Get the Pebble SDK and toolchain
PEBBLE_SDK_VER=${PEBBLE_SDK#PebbleSDK-}
if [ ! -d $HOME/pebble-dev/${PEBBLE_SDK} ]; then
  wget https://sdk.getpebble.com/download/${PEBBLE_SDK_VER} -O PebbleSDK-${PEBBLE_SDK_VER}.tar.gz
  wget http://assets.getpebble.com.s3-website-us-east-1.amazonaws.com/sdk/arm-cs-tools-ubuntu-universal.tar.gz
  # Build the Pebble directory
  mkdir -p ~/pebble-dev
  # Extract the SDK
  tar zxf PebbleSDK-${PEBBLE_SDK_VER}.tar.gz -C ~/pebble-dev/
  # Extract the toolchain
  tar zxf arm-cs-tools-ubuntu-universal.tar.gz -C ~/pebble-dev/${PEBBLE_SDK}

  # Install the Python library dependencies locally
  cd ~/pebble-dev/${PEBBLE_SDK}
  virtualenv --no-site-packages .env
  source .env/bin/activate
  pip install -r requirements.txt
  deactivate
fi

