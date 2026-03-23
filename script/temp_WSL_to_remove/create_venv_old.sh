#!/bin/sh

#############IT SHOULD BE PYTHON 3.10 !!!!!!!!!!!

# Check if at least one argument is provided
if [ -z "$1" ]; then
    echo "Indicate the path where the venv will be created."
    exit 1
fi

# Read the first argument
path_venv="$1/Optics_venv"
echo $path_venv
path_venv=$(readlink -f "$path_venv")
echo $path_venv

#remove venv if it exists
if [ -d $path_venv ]; then
  rm -r $path_venv
fi


sudo dnf update -y

#install python3.10.9
sudo dnf install -y python3.10



#install dependencies for using appimage
sudo dnf install -y wget freeglut freeglut-devel fuse fuse-devel fontconfig fribidi && sudo yum install -y openblas-compat
sudo dnf install -y blas-devel flexiblas 

#download and install pip
wget https://bootstrap.pypa.io/get-pip.py
python3.10 get-pip.py

rm get-pip.py


#install virtualenv
python3.10 -m pip install virtualenv 

#create venv
python3.10 -m venv $path_venv


#activate venv
source "$path_venv/bin/activate"

#upgrade pip
pip install --upgrade pip

#install requirements
pip install -r requirements.txt


cd $HOME
echo "alias source_fOptics='source $path_venv/bin/activate'" >> ~/.bashrc

#Create alias pour intraop

