# webMAN MOD - Fork of original webMAN plugin by Deank for Playstation 3

__README IS STILL IN PROGRESS, IF YOU SEE ANY ERROR PLEASE TELL ME OR PUSH A COMMIT__

webMAN MOD is an homebrew plugin with many features added on the original webMAN by Deank.
We would like to integrate all existing features available on the PS3 Scene like a AIO plugin, and webMAN MOD was born.

## Current functionality
### General
- Support on custom firmware with cobra feature enabled (ver 4.46-4.76 CEX & DEX)
- Support on classic custom firmware with PRXLoader (ver 3.41-4.76 CEX & DEX) *https://github.com/NzV/PRX_Loader*
- Support on classic custom firmware with Mamba PRXLoader (ver 3.41-4.76 CEX & DEX) *https://github.com/NzV/MAMBA_PRX_Loader*
- All PS3 Models with downgrade compatibility is supported, see *http://www.psdevwiki.com/ps3/SKU_Models*

### webMAN vanilla features
- FTP server with remote control functions (shutdown/restart)
- WWW server with remote control functions (scroll down for the complete list of shortcuts)
- Support for loading and browsing of [local] PS3 games in ISO and folder format, DVD videos in ISO format, Blu-ray movies in ISO format, PS1/PS2/PSP games in ISO format with cover display
- NETISO support for network loading and browsing of PS3 games in ISO and folder format, DVD videos in ISO format, Blu-ray movies in ISO format, PS1 and PSP games
- NTFS support for PS3 and PS1 games in ISO format, Blu-ray movies in ISO format and DVD Video in ISO format
- Dynamic Fan Control and in-game temperature monitoring
- PAD shortcuts (scroll down for the complete list of shortcuts)
- Keep USB device awake
- Mount last game or AUTOBOOT.ISO to system startup

### webMAN MOD additional features
- Easy installer/updater
- New folder icons (by Brunolee)
- It can mount PS2 Classics games on PS2 Classic Launcher (.BIN.ENC)
- It can auto-mount any custom folder or ISO. Official only can mount AUTOBOOT.ISO
- Title ID can be displayed on XMB menu
- Covers are shown using the Title ID on the file name of the ISO. Official needs to mount the game to show the covers.
- It can rip a game from disc to hdd0 or copy from hdd0 to usb000 or from usb00x to hdd0.
- FTP includes new SITE commands to allow copy/paste files locally, unmount game, toggle external gamedata, turn on/off dev_blind, change file attributes
- Integrated external gameDATA
- Web Debugger (remote peek/poke/find bytes)
- Spoof IDPS/PSID and display IDPS/PSID on /cpursx.ps3
- Translated to 23 languages
- Several shortcuts to toggle Cobra, swap Rebug files, mount net0/ or net1/, show IDPS/PSID, etc.
- Enable screen capture on CFW that don't has the feature.
- Minor improvements on File Manager (links to navigate faster, mount ISO, mount net0/ or net1/)
- MIN+ memory profile (same as MIN but uses 512K and 2.5X more buffer for PS3 games)
- MAX+ memory profile (same as MAX 1280K for PS3 games, others buffer is reduced, eg: 2X less buffer for ftp and 4X for DVD etc...)
- Copy operations use shadow copy on hdd0 for faster copy
- Scan for games on the stealth folder "/video"
- Support last_game.txt / autoboot on nonCobra edition
- "Offline" mode (blocks some PSN/tracking servers). Game updates still workGame updates still works in this mode. (v1.33.03)
- XMBM+ integration when grouping of XMB content is disabled (v1.33.03)
- Extended Content Profile Grouping (v1.33.07)
- PS3 Manager API Support
- Integrated Mysis video_rec plugin
- support for *.ntfs[BDFILE]* (fake ISO created by IRISMAN)
- support to mount NTFS games using raw_iso.sprx (rawseciso by Estwald)
- Optional Video subfolder to "Bluray™ and DVD" folder (Display RetroXMB videos, videos on USB devices and Data Disc icon)
- "slider" webGUI (mobile friendly).
- Display of Play time & startup time to SELECT+START and /cpursx.ps3 (Use SELECT+START+R2 to display Game ID, Title, play time and more in-Game info)
- Include a virtual pad feature, you can control your console via http://pad.aldostools.org on web browser or with webPAD software (windows only)
- dev_bdvd/PS3_UPDATE now is redirected when the plugin is loaded (and when a game is mounted) [This is intended to prevent an accidental update if a game disc is inserted in the drive]
- VSH Menu for webMAN now can be installed optionally (Holding R1 when launching the Updater)
- Added /play.ps3 to launch XMB Functions (e.g: /play.ps3?col=network&seg=seg_premo) <- this will start Remote Play server from XMB.
- Once a game is mounted via html, if you click on the displayed icon the game will be launched on the PS3. This is nice to start the game once it's mounted from your mobile This option uses the new command /play.ps3
- support for auto-play /BDISO and /DVDISO

## How to build
Requirements for Windows:
- git, clone this repository with the following command: *git clone https://github.com/Matsumot0/webMAN-MOD.git*
- Official PS3 SDK v3.40 or 4.00 complete leaked version, google is your friend to find it
- Open Source PSL1GHT SDK to compile prepNTFS and PKG Updater only.
- GCC (for Windows [MinGW](http://sourceforge.net/projects/mingw) with mingw32-base will be fine) or [Cygwin (x86/x64)](https://cygwin.com/install.html)

Requirements for GNU/Linux:
- Soon...

## Credits
- All the documentation on *http://www.psdevwiki.com*, and to all the devs who contributed
- Cobra team, for their work and sharing cobra source code in public
- Deank as the creator of webMAN
- aldostools for all his works on this project!
- Zar my mate since the first day
- NzV for Ps3mapi, Mamba prxloader
- OsirisX for PS3XPAD
- Mysis, who wrote some useful libs and reverse engineering VSH Exports functions: http://www.ps3devwiki.com/ps3/VSH#Exports
- 3141card for VSH Menu POC & Littlebalup for his enhancement/new features
- PSX-SCENE, PSX-PLACE, PLAYSTATIONHAX, PS3HAX & other scene websites/users, who translated, helped in the testing process


##License
###WebMAN MOD

Hum, not yet