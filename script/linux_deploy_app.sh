#!/bin/sh

#function to copy dependencoies
copy_dependencies() {
    local filename=$1  # The filename passed to the function
    local dirOut=$2
    echo "Copy dependencies for $filename"
    # Use ldd to list dependencies and copy them
    ldd "$filename" | grep "=>" | awk '{print $3}' | while read -r LIB; do
        # Check if the library exists
        if [ -f "$LIB" ]; then
            cp "$LIB" "$dirOut"
        else
            echo "Warning: $LIB does not exist."
        fi
    done
}


get_os_name_version() {
    # Default output if OS information is unavailable
    local OS_INFO="Unknown_OS"

    # Check for /etc/os-release
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        # Replace spaces and special characters in the name and version
        local OS_NAME_CLEAN=$(echo "$NAME" | tr ' ' '_')
        local OS_VERSION_CLEAN=$(echo "$VERSION_ID" | tr -d '"')
        OS_INFO="${OS_NAME_CLEAN}_${OS_VERSION_CLEAN}"
    elif command -v lsb_release &>/dev/null; then
        # Fallback to lsb_release
        local OS_NAME_CLEAN=$(lsb_release -si | tr ' ' '_')
        local OS_VERSION_CLEAN=$(lsb_release -sr | tr -d '"')
        OS_INFO="${OS_NAME_CLEAN}_${OS_VERSION_CLEAN}"
    elif [ -f /etc/redhat-release ]; then
        # Fallback to /etc/redhat-release
        OS_INFO=$(cat /etc/redhat-release | tr ' ' '_')
    fi

    echo "$OS_INFO"
}


# Check if at least one argument is provided
if [ -z "$1" ]; then
    echo "Indicate the path where appImage will be created."
    exit 1
fi

# Read the first argument
PATH_INSTALL=$1
echo $PATH_INSTALL

#Set path
PATH_CODE="$(pwd)/.."
PATH_PRO="$PATH_CODE/main/Process_multi_thread_RT.pro"

OS_NAME_VERSION=$(get_os_name_version)



#Create app directories
cd $PATH_INSTALL

if [ -d "MyApp.AppDir" ]; then
  rm -r MyApp.AppDir
fi

if [ -d "fOptics" ]; then
  rm -r fOptics
fi

#Create appdir 
linuxdeploy-x86_64.AppImage --appdir MyApp.AppDir

#Copy icon
cp "$PATH_CODE/script/icon.png" MyApp.AppDir/usr/share/icons/hicolor/256x256/apps/Intraoperative_functional_brain_mapping.png

#Create desktop file in applications folder
cp "$PATH_CODE/script/Intraoperative_functional_brain_mapping.desktop" MyApp.AppDir/usr/share/applications/Intraoperative_functional_brain_mapping.desktop



#Compile Qt project
export QMAKE=~/Qt/6.10.1/gcc_64/bin/qmake
export LD_LIBRARY_PATH=~/Qt/6.10.1/gcc_64/lib

export QT_DIR=~/Qt/6.10.1/gcc_64
export NO_STRIP=1


cd MyApp.AppDir/usr/bin
$QMAKE $PATH_PRO CONFIG+=release
make -j$(nproc)

#remove object files
rm *.o
rm *.cpp
rm *.h
rm Makefile

#Copy ressource folders
cp -R "$PATH_CODE/main/share/files" ../share/files
cp -R "$PATH_CODE/main/share/python" ../share/python
cp -R "$PATH_CODE/main/share/script" ../share/script

#Temp remove big tensorflow network
#rm ../share/files/model_v1.h5

cd $PATH_INSTALL

#Copy dependencies
cp -L /usr/lib64/samba/libreplace-private-samba.so MyApp.AppDir/usr/lib
copy_dependencies "MyApp.AppDir/usr/lib/libreplace-private-samba.so" "MyApp.AppDir/usr/lib"

cp -L /usr/lib64/libOpenGL.so MyApp.AppDir/usr/lib
copy_dependencies "MyApp.AppDir/usr/lib/libOpenGL.so" "MyApp.AppDir/usr/lib"


echo "Deploy AppImage with linuxdeploy"
linuxdeploy-x86_64.AppImage --appdir MyApp.AppDir --plugin qt --output appimage







#Remove appDir
cd $PATH_INSTALL
rm -r MyApp.AppDir

#Create folder with appImage, installation script and doc
dir_name="fOptics_${OS_NAME_VERSION}"
mkdir -p "$dir_name"

mv Intraoperative_functional_brain_mapping-x86_64.AppImage $dir_name
cp "$PATH_CODE/script/linux_create_venv.sh" "$dir_name/."
cp "$PATH_CODE/script/requirements.txt" "$dir_name/."
cp "$PATH_CODE/doc_src/usage_doc.odt" "$dir_name/."
cp "$PATH_CODE/doc_src/Installation_doc.odt" "$dir_name/."
#cp "$PATH_CODE/script/install_Fedora_WSL.bat" "$dir_name/."

cp "$PATH_CODE/Python/Compute_results_and_metrics.ipynb" "$dir_name/."
cp "$PATH_CODE/Python/utils.py" "$dir_name/."
cp "$PATH_CODE/Python/create_Acquisition_info_file.py" "$dir_name/."

#cp "$PATH_CODE/script/foptics.bat" "$dir_name/."
#cp "$PATH_CODE/script/icone_qkP_icon.ico" "$dir_name/."

