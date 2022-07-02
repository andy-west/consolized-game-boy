# consolized-game-boy
Code, 3D models, and other files to make a consolized Game Boy

## Description
This is a modified version of Andy West's project from *element14 Presents* episode 531: "Game Guy - The Unportable Game Boy".

Notable changes:
 - Single Pico!
 - Using RGB222 Video to free up GPIO (with the limited color schemes, I doubt anyone would notice!)
 - The Color scheme can be changed by holding select and pressing Left or Right
 - All pins have been remapped; this was done mostly to make things easier to route PCB traces
 - 3x scale maintaining aspect ratio
 - Added option to enable scanlines (select+up to cycle scanline color, select+down to disable)
 - With Version 2 PCB, you won't need a functioning Gameboy motherboard.  You just need a good CPU and RAM (you could even use CPU/RAM from a Super Gameboy SNES/Famicom cartridge)

[Video sample](https://youtu.be/khdu8cWNxHo)

[OSD](https://youtu.be/it27PfHu0o8)


Check out Andy's video!  


[![Game Guy - The Unportable Game Boy](https://img.youtube.com/vi/ypGMU5lLjeU/0.jpg)](https://www.youtube.com/watch?v=ypGMU5lLjeU)
 
element14 Community page:
https://community.element14.com/challenges-projects/element14-presents/project-videos/w/documents/27407
