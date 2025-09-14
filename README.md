<h1 align="center" id="title">BinDiff</h1>

A description of this project.
<p id="description">This application is written using gtk/adw in C language using GNOME Builder. The application uses a shared library libbindiff.so. It compares files in two separate directories, finds differences in binary data and displays them. Symbolic links, nonexistent files and files requiring permission are skipped.</p> Usage:

-Run the application

-Select two folders using the Dir1 and Dir2 buttons

-Press the scan button (the one with the refresh icon)

-Afterwards all of the corrupted files will be listed in the main window

-To access specific information about a file select it in the list

-Padding can be added to the sides of of the corrupted portion of data if need be. To select the amount of padding (in bytes) increase or decrease the number in the spinbox to the right of the scan button

The application has been tested and runs on Alt Regular GNOME. App tested using file_scrambler.py.

<h2> Installation Steps:</h2>

<p>1. Install the required dependencies</p>

```
sudo apt-get install meson gcc gtk4-devel libadwaita-devel
```

<p>2. Open project directory</p>

```
cd bindiff
```

<p>3. Configure the build directory</p>

```
meson setup build
```

<p>4. Compile the project</p>

```
meson compile -C build
```

<p>5. Run the app</p>

```
./build/src/bindiff
```
