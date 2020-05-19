
# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
### New features
- Into TR2Main.dll added information about the current version. Now it will be easier to find out which DLL is which version.
- The number of secrets in the script is taken into account when calculating the total number of secrets in the final statistics. Now it is calculated correctly, regardless of the total number of levels, bonus levels, and even mod levels.
- Additional checks have been implemented to prevent Lara from getting weapons that are not supported by current level. Thus, a number of problems associated with the "Nightmare in Vegas" level are fixed.
- Now *"New Game+"* is activated after the completion of the latest level in the script, and not any in which the final statistics appear. This allows you to activate this mode after the "Nightmare in Vegas" level, and not before.
- Now you can start the game in Gold mode, specifying the option *"-gold"* on the command line or shortcut properties (in the same way as the *"-setup"* option).
- For the Gold mode, Spider and Big Spider AI replaced by Wolf and Bear AI. Also Monk Spirits have no shadow.
- For the Gold mode, the credits slideshow starts from *CREDIT00.PCX*. For the original game mode, it starts from *CREDIT01.PCX*.
- For the Gold mode, the selection of any levels (including bonus levels) is activated after collecting all the secrets (available before the final statistics). The ability to select levels remains even after relaunching the game.
- For the Gold mode, pictures with "g" filename suffix (TITLEg.PCX, LEGALg.PCX, etc.) have higher priority than pictures without "g" filename suffix (TITLE.PCX, LEGAL.PCX, etc.)
- For the Gold mode, instead of the files *"TOMBPC.DAT"* and *"MAIN.SFX"*, the files *"TOMBPCg.DAT"* and *"MAINg.SFX"* are used. However, for both Gold and the original game, *"MAIN.SFX"* is still used for the *"TITLE.TR2"* and *"ASSAULT.TR2"* levels.
- Added separate subfolders for saved games of Gold and the original game.
- Added PlayStation reflection visual effect.
- Z-Buffer is no longer required for PlayStation styled gradient health/air bars.
- Added basic/advanced alpha blending for PlayStation styled semitransparent effects.
- Added glow effect for gunfire/flare (alpha blending required).
- Most sprites, gunfire/flare effects, sunglasses lens, the dragon spheres of doom are made semitransparent (alpha blending required). This is how they look in the PlayStation version.
- Rolling wheel blades, wall mounted blades, swords of statues and flamethrower masks are made reflective. This is how they look in the PlayStation version.
- The armed snowmobile windshield is made reflective. This is not presented in the PlayStation version, but probably it was supposed to be. Anyway it looks cool!
- The original game fast snowmobile windshield is made semitransparent (alpha blending required). This is how it looks in the PlayStation version.
- The Golden Mask fast snowmobile windshield is made reflective. In the Golden Mask it has the same windshield model as the armed snowmobile.
- The water surfaces and the glass on the sink in Lara's room are made semitransparent (alpha blending required). This is how they look in the PlayStation version.
- Added support for loading screens from the script.

### The original game bugfixes
- Fixed a bug that prevented the display of the save counter until the game relaunch, if the game was saved in an empty slot.
- Fixed a bug that allowed the player to interfere with the control of the demo level.
- Demo levels are excluded from the *"New Game"* menu, if the *"Select Level"* option is active.
- Fixed the statistics reset in case of *"New Game+"*, if the *"Select Level"* option is active.
- Fixed a bug that does not happen in the official levels (this situation is not in the script). Suppose a shotgun and shotgun ammo are set as a bonus for collecting all the secrets in the level. If Lara does not have a shotgun at the moment of picking the last secret, she receives the relying bonus, but the ammo items are not displayed on the screen. This is fixed now.
- Fixed a bug that does not happen in the official levels (this situation is not in the script). Suppose a magnum and magnum ammo are set as a bonus for collecting all the secrets in the level. If Lara does not have a magnum at the moment of picking the last secret, she receives the magnum only, but not the magnum ammo. This is fixed now.
- Fixed a bug that hid the ammo indicator in demo levels if *"New Game+"* is activated.
- Fixed a bug that produced black triangles originating from the top left of the screen on some laptop graphic adapters.
- Fixed a bug that produced traces of previous textures in untextured areas.

### TR2Main bugfixes
- Background capture is optimized even more, now it is 3-4 times faster than v0.8.2. No lags anymore (broken since v0.8.0).
- Fixed game crash in Software Rendered mode with PlayStation styled inventory text box enabled (broken since v0.8.0).
- Fixed a rare game crash occurring when launching the game (broken since v0.1.0).
- Fixed sunset effect in Bartoli's Hideout level (broken since v0.1.0).

## [0.8.2] - 2019-05-26
### TR2Main bugfixes
- Background capture is redesigned, now it's fast again (broken since v0.8.0).
- Background capture is processed only when required (broken since v0.8.0).
- Fixed PlayStation styled field of view (broken since v0.8.0).

## [0.8.1] - 2018-10-21
### The original game bugfixes
- Fixed texture corruption after FMV scenes.
- Fixed the game hangup if the game window is closed on the statistics screen.
### TR2Main bugfixes
- Level statistics background properly fades out now (broken since v0.8.0).

## [0.8.0] - 2018-09-25
### New features
- Added automatic blocking of incompatible video modes that caused the game to crash.
- The resolution limit is set to 2048x2048, since DirectX 5 cannot handle more.
- Changed the default aspect ratio from 1:1 to 4:3 for the windowed mode with the "any aspect" in the setup dialog.
- Added possibility to use up to 99 images in the credits slideshow. The original game limit is 9 images.
- Added support for HD PCX images (up to 2048x2048). The original game supported only 640x480 images.
- Added stretch limit control for background picture (optional feature).
- Added PNG/JPEG/BMP images support (hardware renderer required).
- Added *PIX* folder for automatic management of pictures with different aspect ratios.
- Added PNG screenshots with date-time filename format (optional feature). TGA/PCX screenshots retained the sequential filename format.
- Added setting to specify a user directory for saving screenshots files.
- Added edge padding instead of UV adjustment for background textures.
- Now *"end"* picture is used for the final statistics background.
- Added fade in/out between scenes.
- Added TR1 styled transparent inventory background for hardware renderer (optional feature). This background is also used for level statistics like in the PlayStation version even if not set as inventory background.
- Text box requester line height is slightly reduced to be more compact.
- Added PlayStation styled inventory text box (optional feature).
### The original game bugfixes
- Fixed the original game bug when the health bar is visible at the beginning of the final bath scene.
- Screenshots now work properly in the windowed mode in modern Windows systems.
- Screen tearing does not affect screenshots for the windowed mode anymore.
- Fixed an issue where the final bath scene is cut off. Now it fades out like in the PlayStation version.
- Text box frame is slightly adjusted to fill gaps in the corners.
### TR2Main bugfixes
- Fixed incorrect underwater fog settings (broken since v0.4.0).
- Fixed a minor issue related with clipping of round shadows (broken since v0.7.0).
- Fixed clipping of animated background textures (broken since v0.3.0).

## [0.7.0] - 2018-07-28
### New features
- Added round shadows instead of octagonal (optional feature).
### The original game bugfixes
- Fixed shadow clipping if the Z-Buffer is turned on.
- Fixed a rare bug that caused black sprites to appear in bright rooms.
### TR2Main bugfixes
- Fixed a bug that caused invisibility of pitch black sprites (broken since v0.4.0).

## [0.6.0] - 2018-02-24
### New features
- Custom HUD scaling options separated for the inventory and in-game (optional feature).
### The original game bugfixes
- The size of the active screen area should always be 1.0 for credits and final statistics.
### TR2Main bugfixes
- Health bar coordinates slightly corrected in the inventory for the PlayStation position style (broken since v0.5.0).

## [0.5.0] - 2018-02-17
### New features
- Improved optimization of the perspective correction for the software renderer (up to 2x faster than the original game code)
- Text is unstretched for any display modes.
- Added automatic scaling of text box borders depending on the current screen resolution.
- Added automatic scaling of the assault course timer depending on the current screen resolution.
- The best results of assault course are saved in the registry and are not reset after relaunching the game.
- The health/air bar accuracy increased for higher resolutions.
- Added PlayStation styled health bar position (optional feature).
- Added custom HUD scaling options (optional feature).
- Added simplified gradient health/air bars (optional feature; hardware renderer and Z-Buffer required).
### The original game bugfixes
- Fixed texel adjustment if the renderer is changed by *Shift+F12* hotkey.
- Fixed untextured polygon sorting for the software renderer.
- Other minor bugfixes of the renderer.
### TR2Main bugfixes
- Fixed cheat sequences (broken since v0.1.0).

## [0.4.0] - 2018-01-05
### New features
- Added draw distance and fog settings (optional feature).
### TR2Main bugfixes
- Fixed texture minification filtering (broken since v0.1.0).
- Fixed Z-Buffer for animated background of the inventory (broken since v0.3.0).
- Fixed *"Select level"* option (broken since v0.1.0).

## [0.3.0] - 2018-01-03
### New features
- Added PlayStation styled field of view (optional feature).
- Added PlayStation styled gradient health/air bars (optional feature; hardware renderer and Z-Buffer required).
- Added PlayStation styled (animated) background of the inventory (optional feature; hardware renderer required).
- Tiled static background of the inventory is unstretched for any display modes.
### The original game bugfixes
- Fixed a rare game crash occurring when exiting the game while playing CD Audio.
### TR2Main bugfixes
- Fixed back-face culling if the Z-Buffer is turned on (broken since v0.1.0).
- Fixed a bug that prevented collecting more than one item of each type in the inventory (broken since v0.1.0).
- Fixed a minor bug with looped tracks in the PaulD's CD Audio solution code (broken since v0.2.0).

## [0.2.0] - 2017-12-31
### New features
- The TGA screenshot captures the game window instead of the whole screen in the case of windowed mode.
- Added PaulD's CD Audio solution. Now the game supports *cdaudio.mp3* / *cdaudio.dat* music package. Just put it into *audio* folder in the game installation directory.
### The original game bugfixes
- Fixed default/custom keyboard layout conflicts when starting the game without visiting the controls menu.
### TR2Main bugfixes
- Z-Buffer depth priority reverted to original 16/24/32/8 due to the *"CreateZBuffer"* errors on some systems (broken since v0.1.0).
- Screenshot key is set to *BackSpace* (previously it was *PrintScreen*) to fix problems on some systems (broken since v0.1.0).
- Fixed CD Audio synchronization bug (broken since v0.1.0).

## [0.1.0] - 2017-12-25
### New features
- Added NoCD patch. It is required that the *DATA* and *FMV* folders are located in the game installation folder, and not on the CD.
- Added automatic adjustment of aspect ratio and field of view, depending on the current screen resolution.
- Added automatic scaling of picked up items depending on the current screen resolution.
- Added automatic scaling of secrets (dragons) in the level statistics menu depending on the current screen resolution.
- Added automatic scaling of the health/air bars depending on the current screen resolution.
- Video modes in the setup dialog are now sorted by depth, width and height.
- Screenshot files are now created in the *screenshots* subfolder, not the game installation folder itself.
- TGA screenshots are now available for 24/32-bit video modes. Not just for 16-bit video modes, as it was in the original game.
- Screenshot key is set to *PrintScreen*. In the original game, it's set to the *S* key, which makes it unacceptable to map *S* key with another action.
- ~~Z-Buffer depth priority set to 32/24/16/8 bits to improve rendering quality if a 32-bit Z-Buffer is available. In the original game, it is 16/24/32/8~~ (reverted in v0.2.0).
### The original game bugfixes
- Fixed incorrect parameters of the game window, leading to the appearance of unwanted borders in full screen mode.
- The default music volume level is set to 10/10.  In the original game, the default value is 165/10, which leads to the absence of music until the volume level is adjusted in the game menu.
- Fixed the *low ceiling jump* bug that was present in the early releases of the Tomb Raider II.  In later versions, Core Design fixed it.
- Fixed a bug that caused some graphic objects to disappear or flicker if the Z-Buffer is turned off.
- Fixed a bug leading to a non-responsive keyboard when switching to another Windows application or launching a game using Wine under Linux.
- Removed repeating actions caused by a single hotkey keystroke (F1, F2, F12, Screenshot key). One keystroke - one action!
- Fixed the problem when the game overwrites the screenshot files made in previous launches of the game.
- Fixed unsafe memory management in the screenshot function, which caused the game to crash at higher screen resolutions.
- Fixed incorrect TGA screenshot creation. In the original game, the bottom line of pixels is filled with junk.

[Unreleased]: https://github.com/Arsunt/TR2Main/compare/v0.8.2...HEAD
[0.8.2]: https://github.com/Arsunt/TR2Main/compare/v0.8.1...v0.8.2
[0.8.1]: https://github.com/Arsunt/TR2Main/compare/v0.8.0...v0.8.1
[0.8.0]: https://github.com/Arsunt/TR2Main/compare/v0.7.0...v0.8.0
[0.7.0]: https://github.com/Arsunt/TR2Main/compare/v0.6.0...v0.7.0
[0.6.0]: https://github.com/Arsunt/TR2Main/compare/v0.5.0...v0.6.0
[0.5.0]: https://github.com/Arsunt/TR2Main/compare/v0.4.0...v0.5.0
[0.4.0]: https://github.com/Arsunt/TR2Main/compare/v0.3.0...v0.4.0
[0.3.0]: https://github.com/Arsunt/TR2Main/compare/v0.2.0...v0.3.0
[0.2.0]: https://github.com/Arsunt/TR2Main/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/Arsunt/TR2Main/releases/tag/v0.1.0