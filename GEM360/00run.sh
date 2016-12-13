#!/bin/bash
xset -d :0 dpms force off
sleep 1
xset -d :0 dpms force on
sleep 1
xset -d :0 -dpms
sleep 1
puredata -noaudio -open 0K360Gem.pd
