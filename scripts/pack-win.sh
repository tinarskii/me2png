#!/bin/bash
set -e

VERSION=$(grep -oP 'Me2PNG v\K[0-9]+\.[0-9]+\.[0-9]+' src/Main.cpp | head -1)
PACKAGE="me2png-$VERSION-windows"

echo "Packaging Windows $VERSION..."

mkdir -p dist/$PACKAGE
cp build-win/me2png.exe dist/$PACKAGE/
cp build-win/libgcc_s_seh-1.dll dist/$PACKAGE/
cp build-win/libportaudio.dll dist/$PACKAGE/
cp build-win/libstdc++-6.dll dist/$PACKAGE/
cp build-win/libwinpthread-1.dll dist/$PACKAGE/
cp build-win/raylib.dll dist/$PACKAGE/
cp -r assets/ dist/$PACKAGE/assets/

cd dist
zip -r $PACKAGE.zip $PACKAGE/
rm -rf $PACKAGE

echo "Done: dist/$PACKAGE.zip"