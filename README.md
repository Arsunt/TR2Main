# TR2Main

This is an open source implementation of the classic Tomb Raider II game (1997), made by reverse engineering of the original game for PC and PS1. Right now it's a dynamic library injecting the original game EXE to allow step-by-step decompilation, fixing, and improvement, but someday it will become single EXE file.

The goal of the project is to make 1:1 clone of the original PC game, to fix all its bugs, and to add more features from non-PC official ports (for example from PS1). So TR2Main is some sort of unofficial modern PC port of Tomb Raider II.

## Getting Started

You may download patched Tomb2.exe, TR2Main.dll from [releases](https://github.com/Arsunt/TR2Main/releases), put them in your game folder and just launch. Also you may build TR2Main.dll yourself and use it with patched Tomb2.exe from [here](/binaries).

## Changelog

Learn about [the latest improvements](CHANGELOG.md).

## Built With

* [Code::Blocks](http://www.codeblocks.org) - C/C++ IDE used
* [GCC](https://gcc.gnu.org) - C/C++ compiler
* [Doxygen](http://www.doxygen.org) - documentation

## Authors

* **Michael Chaban** \([Arsunt](https://github.com/Arsunt)\). Author of the project. E-mail: <arsunt@gmail.com>
* [**ChocolateFan**](https://github.com/asasas9500). Decompilation.
* [**TokyoSU**](https://github.com/TokyoSU). Decompilation and features development.
* **PaulD**. Author of [CD audio solution](modding/cd_pauld.cpp) used now by Steam.

## License

This project is licensed under the GNU General Public License - see the [COPYING.md](COPYING.md) file for details

## Acknowledgments

If you are inspired by my project and decided to borrow some of the ideas found in the code, do not forget provide a link to this project. It took me years to decompile and understand the details required for this project.

## Copyright
(c) 2017-2023 Michael Chaban. All rights reserved.
Original game is created by Core Design Ltd. in 1997.
Lara Croft and Tomb Raider are trademarks of Embracer Group AB.