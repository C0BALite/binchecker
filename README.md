BinDiff
=======

A description of this project.

This application is written using gtk/adw in C language. The application uses a shared library libbindiff.so. It compares files in two separate directories, finds differences in binary data and displays them. Symbolic links, nonexistent files and files requiring permission are skipped.

Usage: 

-Run the application 

-Select two folders using the folder buttons

-Press the scan button 

-Afterwards all of the corrupted files will be listed on a new page in a list format 

-To access specific information about a file select it in the list 

-Padding can be added to the sides of of the corrupted portion of data if need be. To select the amount of padding (in bytes) increase or decrease the number in the spinbox.

The application has been tested and runs on Alt Regular GNOME.

Installation Steps:
-------------------

1\. Install the required dependencies

``` sudo apt-get install meson gcc gtk4-devel libadwaita-devel ```

2\. Open project directory

``` cd bindiff ```

3\. Configure the build directory

``` meson setup build ```

4\. Install the app

``` meson install -C build ```

5\. Run the app from the applications menu
