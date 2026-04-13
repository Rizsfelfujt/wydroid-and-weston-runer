# Compositor Runner

Simple GTK3 application to start Weston compositor. Tested Linux Mint 22  Cinnamon 6.2.9. X11 
App automatically launches waydroid and Weston compositor, adjusting the screen size.

## Why I made this

I wanted a simple GUI to start Weston compositor with custom resolution.

## Features

- Set width and height
- Start Weston

## Screenshot

![App](App.png)

## Build

g++ main.cpp -o program $(pkg-config gtkmm-3.0 --cflags --libs) -lX11

## Install

Download from Releases and install:

sudo dpkg -i compositor-runer.deb
