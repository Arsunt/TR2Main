# TR2Main by Arsunt

This is a dynamic library for the classic Tomb Raider II game (Windows version). The purpose of the library is to reimplement all the routines performed by the game. This is not a wrapper, it's a full-fledged solution that solves a bunch of compatibility issues, and also allows to easily change gameflow.

The goal of this project is to maximally recreate the code of the original game, gradually transferring its various aspects to dynamic libraries that are linked to the modified original EXE. In this case, main code remains in the game, but graphics, sounds and controls will be in the separate DLL files.

## Getting Started

You may download patched Tomb2.exe, TR2Main.dll from [releases](https://github.com/Arsunt/TR2Main/releases), put them in your game folder and just launch. Also you may build TR2Main.dll yourself and use it with patched Tomb2.exe from [here](/binaries). You will need DirectX SDK 5/7 to build the DLL.

## Changelog

Learn about [the latest improvements](CHANGELOG.md).

## Built With

* [Code::Blocks](http://www.codeblocks.org) - C/C++ IDE used
* [GCC](https://gcc.gnu.org) - C/C++ compiler
* [Doxygen](http://www.doxygen.org) - documentation

## Authors

* **Michael Chaban** \([Arsunt](https://github.com/Arsunt)\). E-mail: (arsunt@gmail.com)
* **PaulD**. Author of [CD audio solution](modding/cd_pauld.cpp) used now by Steam.

## License

This project is licensed under the GNU General Public License - see the [COPYING.md](COPYING.md) file for details

## Acknowledgments

If you are inspired by my project and decided to borrow some of the ideas found in the code, do not forget provide a link to this project. It took me years to decompile and understand the details required for this project.

## Copyright
(c) 2017-2019 Michael Chaban. All rights reserved.
Original game is created by Core Design Ltd. in 1997.
Lara Croft and Tomb Raider are trademarks of Square Enix Ltd.