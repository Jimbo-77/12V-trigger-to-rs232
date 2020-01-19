# 12V-trigger-to-rs232
Code to manage the Cambridge 851A hifi amplifier from the HomeCinema 12V triggerOut.
:Author: Jimbo-77
:Email: jlegrand@free.fr
:Date: 29/12/2019
:Revision: version#
:License: Public Domain

= Project: 12V-trigger-to-rs232.git

The goal of this project is to create a program managing a Cambridge 851A amplifier when a 12V trigger out is received.
when the homcinema power up, the 12v trigger out become high. 
Then the arduino send a power on command and after few seconds the select input command.

The program includes different cases, like the Cambridge amplifier is not powered down when the input source is different than the homecinema input.

== Step 1: Installation
Please describe the steps to install this project.

For example:

1. Open this file
2. Edit as you like
3. Release to the World!

== Step 2: Assemble the circuit

Assemble the circuit following the diagram layout.png attached to the sketch

== Step 3: Load the code

Upload the code contained in this sketch on to your board

=== Folder structure

....
 sketch123                => Arduino sketch folder
  ├── sketch123.ino       => main Arduino file
  ├── schematics.png      => (optional) an image of the required schematics
  ├── layout.png          => (optional) an image of the layout
  └── ReadMe.adoc         => this file
....

=== License
This project is released under a {License} License.

=== Contributing
To contribute to this project please contact Jimbo-77 https://id.arduino.cc/Jimbo-77

=== BOM
Add the bill of the materials you need for this project.

|===
| ID | Part name      | Part number | Quantity
| R1 | 10k Resistor   | 1234-abcd   | 10
| L1 | Red LED        | 2345-asdf   | 5
| A1 | Arduino Zero   | ABX00066    | 1
|===


=== Help
This document is written in the _AsciiDoc_ format, a markup language to describe documents.
If you need help you can search the http://www.methods.co.nz/asciidoc[AsciiDoc homepage]
or consult the http://powerman.name/doc/asciidoc[AsciiDoc cheatsheet]
