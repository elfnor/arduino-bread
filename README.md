Arduino Bread
=======================

This project replaces the control circuit of a Sanyo breadmaker "The Bread Factory Plus" (Model SBM-20)
(many other models are similar) with a circuit based on an Arduino.

The aim is to separate out the different stages in the breadmaker's cycle so I can make good sourdough bread. In general the sourdough takes longer to rise than dough made using store bought dried yeast granules. The rise time can also vary depending on how the sourdough starter is on any day. Also the longer the rise time the stronger the sour flavour of the bread.

This project is ongoing. Currently I have made a breadmaker with an Arduino brain where I can set up programs with my own times for any part of the breadmaker cycle. To make a loaf of sourdough, I put the ingredients in the pan and select a program that mixes and kneads the dough then keeps the pan at rise temperature indefinetly. When I've checked the bread is sufficiently risen, I stop that program and run a bake only program. Eventually, I'd like some way of measuring the height of the dough so the arduino can track when the bread is risen and then switch to the bake cycle. The current rise temperature is the same as for the original breadmaker cycle (~37 °C). I'll probably add an option to rise the bread at a lower temperature for longer to get a stronger sour flavour.

The model of breadmaker I'm using is a fairly old Sanyo "The Bread Factory Plus" (Model SBM-20). From other people's projects I suspect many breadmakers are quite similar internally and these notes could be applicable to many other models and brands.

The circuit and documentation can be found at http://elfnor.com/Arduino%20Bread%20Details.html

This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

elfnor


