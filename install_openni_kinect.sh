#!/usr/bin/bash

echo -e "\n"
echo "***********************************"
echo "Installing OpenNI/SensorKinect/NITE"
echo "***********************************"

echo -e "\n"
echo "Installing dependencies"

# installation commands need root privilege
apt update
apt install -y build-essential
apt install -y usbutils # lsusb cmd
apt install -y udev libusb-1.0-0-dev freeglut3-dev
apt install -y git
apt install -y python3
apt install -y default-jdk

wdir=$(pwd)
echo $wdir 

# compile and install OpenNI library
echo -e "\n\n\n"
echo "-> Installing OpenNI"
git clone https://github.com/smbct/OpenNI.git --branch master --single-branch
cd "$wdir/OpenNI"
git checkout e6a36f9e73fa4841365acbc559d011e4d4992dc8
cd "./Platform/Linux/CreateRedist"
bash "./RedistMaker"
cd "../Redist/OpenNI-Bin-Dev-Linux-x64-v1.5.7.10/"
bash "./install.sh" # needs root privilege

# compile and install SensorKinect
echo -e "\n\n\n"
echo "-> Installing SensorKinect"
cd "$wdir"
git clone https://github.com/smbct/SensorKinect.git --branch unstable --single-branch
cd "./SensorKinect"
git checkout 720bb526fa4f29dce967b21de57f39987889ddd8
cd "./Platform/Linux/CreateRedist"
bash "./RedistMaker"
cd "../Redist/Sensor-Bin-Linux-x64-v5.1.2.1/"
bash "./install.sh" # needs root privilege

# install NITE library
echo -e "\n\n\n"
echo "-> Installing NITE"
cd "$wdir"
git clone https://github.com/arnaud-ramey/NITE-Bin-Dev-Linux-v1.5.2.23.git --branch master --single-branch
cd "./NITE-Bin-Dev-Linux-v1.5.2.23/"
git checkout 19ac086493c312601b9ecccb0260937c056124db
cd "./x64"
bash "./install.sh" # needs root privilege

echo "Done!"
