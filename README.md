# TR2Main by Arsunt

This is a dynamic library for the classic Tomb Raider II game (Windows version). The purpose of the library is to reimplement all the routines performed by the game. This is not a wrapper, it's a full-fledged solution that solves a bunch of compatibility issues, and also allows to easily change gameflow.

The goal of this project is to maximally recreate the code of the original game, gradually transferring its various aspects to dynamic libraries that are linked to the modified original EXE. In this case, main code remains in the game, but graphics, sounds and controls will be in the separate DLL files.

## Getting Started

You may download patched Tomb2.exe, TR2Main.dll from here, put them in your game folder and just launch. Also you may build TR2Main.dll yourself and use it with patched Tomb2.exe. You will need DirectX SDK 5/7 to build the DLL.

## Built With

* [Code::Blocks](http://www.codeblocks.org) - C/C++ IDE used
* [GCC](https://gcc.gnu.org) - C/C++ compiler
* [Doxygen](http://www.doxygen.org) - documentation

## Authors

* **Michael Chaban** \([Arsunt](https://github.com/Arsunt)\). E-mail: (arsunt@gmail.com)

## License

This project is licensed under the GNU General Public License - see the [COPYING.md](COPYING.md) file for details

## Acknowledgments

If you are inspired by my project and decided to borrow some of the ideas found in the code, do not forget to credit my name. It took me years to decompile and understand the details necessary for this project. Thank you for understanding!

## Copyrignt
(c) 2017 Michael Chaban. All rights reserved.
Original game is written by Core Design Ltd. in 1997.
Lara Croft and Tomb Raider are trademarks of Square Enix Ltd.
