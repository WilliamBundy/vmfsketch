Copyright William Bundy, 2017. 
All rights reserved.
Do not distribute.

THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

=============================================================================
Changelog
==============================================================================

Version Alpha 1 Hotfix 1:
	- Fixed a bug where the editor could become unresponsive (fighting UI
		for input.)
	- Fixed a bug where text boxes would stop being active if you moused
		off them. Note: changing this means that text boxes still recieve
		keyboard input if they're selected, but keys pressed while the mouse
		is off the window will be passed to the editor. Clicking outside
		of the textbox will always deactivate it, however. You can press
		enter to achieve the same effect. 
	- Layers now export their names to the visgroups; rather than being named
		"Layer0" through "LayerN", it's "Floors - 0", "Walls - 1", Stairs - 2"
		etc.
	- Updated readme with some controls I forgot to mention.
Version Alpha 1:
	- Initial Release.

=============================================================================
Overview
==============================================================================

VMFSketch is an application designed to make early layouts of maps fast and 
pleasant. It exports to the Valve Map Format (vmf) for use in Hammer. The 
interaction design of VMFSketch is inspired by Vim ("Vi Improved"), and
therefore bears some similarities. 
	
	- Modal: The current mode determines the actions and tools 
		available to you. Many actions are only available in one 
		mode. This might be confusing at first, but it allows for
		much more consice shortcuts and prevents you from making 
		mistakes.
	- Keyboard-only actions: VMFSketch only has keyboard shortcuts
		for a lot of editing actions. This helps keep the GUI clean
		and editing fast, once you learn them.
	- Two-handed: VMFSketch isn't totally keyboard driven--it's
		designed so that you can easy hit all the shortcuts by 
		keeping your left hand around the WASD cluster while editing
		with the mouse.

Where possible, I have preserved the behavior of Hammer or other similar
applications, in order to help keep VMFSketch familiar to new users.

==============================================================================
Controls
==============================================================================
You can press F1 in-app to view a visual layout of the controls.
Controls:
	WASD: move around.
	1, 2: Decrease/increase grid size, respectively
		Decreasing the grid size makes the grid denser, 
		increasing it makes it sparser.
	Space: Enter panning mode
		F: Recenter everything (set camera coordinates to 0,0) and set scale
			to 0.5.
		G: Reset the grid size to 64hu
	Q: go to normal mode
		Click on a brush to select it, ctrl-click to select multiple.
		You can drag a rectangle to select multiple brushes
	E: go to edit mode
		You can click and drag to create rectangular brushes.
		Click on a brush to select it, ctrl-click to select multiple.
		Once selected, the green handles let you resize brushes.
		R, Shift-R: rotate selected brushes 90 degrees clockwise
			or counterclocwise, respectively.
		F, Shift-F: flip selected brushes horizontally or vertically,
			respectively.
		C, V: Copy, paste; just like normal, but without holding control.
		X: Cut, copying selected brushes.
		Shift-X, Delete: Delete selected brushes.
	T: go to vertex mode
		In vertex mode, the red handles let you modify the vertices
		of brushes individually. This is a safe operation; VMFSketch
		will not generate invalid solids. The blue handle lets you specify the 
		low side for sloped brushes. 
	G, Shift-G: Move up or down a layer, respectively. Switching layers
		this way moves selected brushes too. 

==============================================================================
Layers: What are they and how do they work?
==============================================================================

Layers in VMFSketch define the height of brushes. They can be moved around 
freely in space, and (planned feature) rotated. Layer height is determined
by two numbers. From the X,Y,Z of the layer downwards is specified by the
depth, and the distance upwards is specified by the rise. 

   ^ Rise
   |
------ Z
|
v Depth

Defining layers like this makes it easy to align layers while creating
geometry. You can specify a wall and floor at the same Z level, and increase
the rise of the walls to make them walls proper. 

Rise also comes into play when defining slopes. In vertex mode, the selected
brush has a blue handle. Dragging this handle across an edge selects it as
the "low side". That edge will be at Z, not Z+rise, creating a slope in the
final export. This makes it easy to create slopes that connect layers:
align the layers on Z, specify the rise for the next layer up, then create
slopes to link the two layers.

==============================================================================
Contact
==============================================================================

You can contact me on twitter: @William_Bundy, or as @Will#7355 on Discord.
I hang out in Uncle Dane's and UEAKCrash's discords generally, so you can
ping me there too.


==============================================================================
Contributing
==============================================================================
Though VMFSketch isn't a gigantic piece of software, a lot of work has gone 
into it and some of it was done in a hurry. If you find a bug or major UX 
fault, report that it happened, how to reproduce it, and I'll try and fix it.

With regards to requests: I have plenty of ideas for this piece of software.
There are glaring omissions that will certainly be released in later versions,
including but not limited to:
	- Undo support
	- Rotating layers
	- Rotating brushes
	- Better layer visualization
	- Individual brush settings
	- Better interfaces, esp. for files.
	- Extensions/addins
	- Generating entities (lights, spawns)
	- Generally being able to edit entities
	- Importing existing VMF files
	- Many, many more.
Currently the internals aren't perfectly flexible, and my current goal is to
improve those before adding more features to juggle; however, when requesting
features, please consider the goal of this software: making simple layouts as
fast as possible.

==============================================================================
Support VMFSketch
==============================================================================

I made VMFSketch because I wanted something better than Hammer for basic
brushwork. I choose to release it free because I'd like better maps for TF2.
If you'd like to support the development of VMFSketch and get early versions,
please pledge on my Patreon: https://patreon.com/williambundy



