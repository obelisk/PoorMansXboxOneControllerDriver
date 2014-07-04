Poor Man's Xbox One Driver
--------------------------

So you've got a nice Xbox One controller but your mac doesn't support it eh? Not anymore! Using the Poor Mans Xbox One Driver, you can remap the buttons to keyboard buttons allowing yourself to play basic games and just have a grand old time.

Currently tested with Castle Crashers and seems to work pretty well.


Why do you call it the "Poor Man's Driver"?
-------------------------------------------
Because it's not technically a driver. A true driver would make the operating system recognise it as an HID device and would have a kernel extension. This is just a underland program that you start before you want to start using the controller.

There are a couple benefits to this however:
	It will not crash your system like a KEXT with a bug in it will

	You don't need to be root/administrator to use it


Ok...So how do I actually use it?
---------------------------------
I've tried to make it as simple as possible.

You'll need to have gcc installed (which you can get for free from developers.apple.com, install the commandline tools).

If you want to, you can modify the active_conf array to change the button mapping. A full list of the button codes can be found here: http://snipplr.com/view/42797/

Then run this command to compile and run the "driver":



gcc -objc -framework CoreFoundation -framework IOKit -framework ApplicationServices -framework Carbon -O2 -o driver first.c; ./driver


Now go into your game and you should now be able to play your game using the keyboard controllers from your Xbox One Controller.



Here is a picture of how I've configured Castle Crashers to work with the controller: http://imgur.com/gcPZmuJ


Future Goals
------------
I would like to make the ability to create game profiles and then load them more easily instead of having one array that needs to be changed for each game that has a different mapping. Hopefully I can do that in the next couple weeks to months.