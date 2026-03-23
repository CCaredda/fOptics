#!/bin/sh

#Check if .venv path exists
if [ ! -d $HOME/.venv ]; then
  mkdir $HOME/.venv
fi

path_venv="$HOME/.venv/Optics_venv"
echo $path_venv

#remove venv if it exists
if [ -d $path_venv ]; then
  rm -r $path_venv
fi


sudo dnf update -y

#install python3
sudo dnf install -y python3



#install dependencies for using appimage
sudo dnf install -y wget freeglut freeglut-devel fuse fuse-devel fontconfig fribidi && sudo yum install -y openblas-compat
sudo dnf install -y blas-devel flexiblas 


#create venv
python3 -m venv $path_venv

#activate venv
source "$path_venv/bin/activate"

#upgrade pip
pip install --upgrade pip

#install requirements
pip install -r requirements.txt

