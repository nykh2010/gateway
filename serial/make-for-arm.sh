#!/bin/bash
cp -r Debug ARM_Debug
sed -i 's/gcc/${CC}/g' ./ARM_Debug/makefile ./ARM_Debug/*.mk ./ARM_Debug/*/*.mk
cd ./ARM_Debug