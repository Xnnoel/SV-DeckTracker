# SV-DeckTracker
An automatic deck tracker for Shadowverse on PC emulators

##Intro

This is a program used to track cards in Shadowverse. It does this by taking a screenshot of the program and examining portions for cards.
It compares the image sections with pHashes from the card database, returning the best match. I got the idea after looking at 
[this implementation](https://github.com/wittenbe/Hearthstone-Image-Recognition) for Hearthstone. Props to him.

The current build is barebones, only getting the cards drawn from the deck. What this means is that cards that draw more than 1 at a time 
or cards that refresh your hand won't work with it. It's only about 80% accurate due to the card images being so different from the ones 
on their website. Basically, this is more fun to look at than being a helpful card tracker. The code isn't very well-documented/organized 
since I did this as a hobby while I wait for the PC release. 

##Compiling
- OpenCV 2.4.13
- QT 5.7

Compiled on Windows, 64 bit. It uses several windows function, so it won't compile on linux/mac.
