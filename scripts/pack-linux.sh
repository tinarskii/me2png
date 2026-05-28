#!/bin/bash
set -e

VERSION=$(grep -oP 'Me2PNG v\K[0-9]+\.[0-9]+\.[0-9]+' src/Main.cpp | head -1)
PACKAGE="me2png-$VERSION-linux"

echo "Packaging Linux $VERSION..."

mkdir -p dist/$PACKAGE
cp build/me2png dist/$PACKAGE/
cp -r assets/ dist/$PACKAGE/assets/

cd dist
tar -czf $PACKAGE.tar.gz $PACKAGE/
rm -rf $PACKAGE

echo "Done: dist/$PACKAGE.tar.gz"