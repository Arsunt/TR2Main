# Tomb Raider 2 binaries

Here are few binary files that are taken from the original game and slightly modified to work with TR2Main.

- [Tomb2_original](Tomb2_original.zip) is the original executable build on December 1, 1997 by Core Design (though file timestamp is 2:36pm, December 10, 1997). It does not use *TRMain.dll*. This one is uploaded only so that you can **compare what has changed** in the patched versions.
- [Tomb2_legacy](Tomb2_legacy.zip) is based on the original executable, but it has a slightly enlarged *.text* section with *write* permission. The entry point is redirected to a function that links with *TRMain.dll*, and then jumps to a common entry point. This executable can be run with or without *TRMain.dll*. Recommended for **debugging and unit testing**.
- [Tomb2_clean](Tomb2_clean.zip) is based on the original executable, it has an original entry point and *.text* section, but with *write* permission. It links with *TRMain.dll* through *.import* table. *Winplay.dll* dependency removed. Also it has the compatibility manifest and XP manifest in the *.rsrc* section. This executable requires *TRMain.dll*. Recommended for **release builds**.

Also you can find here some additional media files

- [BAREFOOT.SFX](BAREFOOT.SFX.zip) is an SFX package with 4 sound effects of barefoot steps. These effects are taken from the PlayStation version of the game and slightly remastered by me (only the high-frequency range was restored).

If you are looking for a complete package with the *Tomb2.exe*, *TR2Main.dll* and other stuff, then you should go to [releases](https://github.com/Arsunt/TR2Main/releases).