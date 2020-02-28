#! /bin/bash

cd cpp
mkdir -p build
ln -s ../senva .
cd ../py
ln -s ../senva .
cd ../ps
cp -r ../senva .
