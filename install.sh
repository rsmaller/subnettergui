#!/bin/bash
mkdir ~/.local/share/icons/subnettergui
cp ./install/subnettergui.svg ~/.local/share/icons/subnettergui/subnettergui.svg # Icon by Pexelpy on freeicons.io
echo "[Desktop Entry]
Type=Application
Name=Subnetter++
StartupWMClass=Subnetter++
Exec=/usr/local/bin/subnettergui
Icon=$HOME/.local/share/icons/subnettergui/subnettergui.svg" > ~/.local/share/applications/subnettergui.desktop
sudo cp ./build/subnettergui /usr/local/bin/subnettergui