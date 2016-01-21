#!/bin/bash
set -e
echo 'pBuild 1.1'
echo 'Installing Pebble SDK and its Dependencies...'

cd ~
mkdir -p ~/pebble-dev
touch ~/pebble-dev/ENABLE_ANALYTICS

# Get the Pebble SDK and toolchain
PEBBLE_SDK_VER=${PEBBLE_SDK#PebbleSDK-}
if [ ! -d $HOME/pebble-dev/${PEBBLE_SDK} ]; then
  # Legacy SDK path before 3.8
  # wget https://sdk.getpebble.com/download/${PEBBLE_SDK_VER} -O PebbleSDK-${PEBBLE_SDK_VER}.tar.gz
  wget https://s3.amazonaws.com/assets.getpebble.com/sdk3/release/sdk-core-${PEBBLE_SDK_VER}.tar.bz2 -O PebbleSDK-${PEBBLE_SDK_VER}.tar.bz2
  wget http://assets.getpebble.com.s3-website-us-east-1.amazonaws.com/sdk/arm-cs-tools-ubuntu-universal.tar.gz

  # Extract the SDK
  tar xvjf PebbleSDK-${PEBBLE_SDK_VER}.tar.bz2 -C ~/pebble-dev/
  # Extract the toolchain
  tar zxf arm-cs-tools-ubuntu-universal.tar.gz -C ~/pebble-dev/${PEBBLE_SDK}

  # Install the Python library dependencies locally
  cd ~/pebble-dev/${PEBBLE_SDK}
  virtualenv --no-site-packages .env
  source .env/bin/activate
  pip install -r requirements.txt
  deactivate
fi
