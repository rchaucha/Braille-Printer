# Braille Printer 
This is a C++ / Arduino project for my [Baccalauréat](https://en.wikipedia.org/wiki/Baccalaur%C3%A9at) in a team of 3. I was in charge of the coding part while the others were in charge of the mechanical part. 

**Goal**:  To create a printer capable of printing a text entered on a computer in [braille] (https://en.wikipedia.org/wiki/Braille) format on any plane surface (e.g cardboard box) by leaving a particular substance on it. It was based on the principle of a 3D printer.  
## Description This project is composed of two parts:
* **C++** : This is a GUI (using Qt) running on a computer that allows the user to enter the text he wants to print. As the user types, a rectangle appears dynamically on a webcam feedback that shows the space that will be taken by the text. You can see a screenshot of the GUI below. This program is also in charge of converting the text to braille by using bytes (0 : no point, 1 : point).
* **Arduino** : This is the program uploaded to the Arduino card connected to the computer. Both programs communicate which allow the C++ software to send the data to print to the Arduino that eventually commands the motors of the printer to move accordingly.
  
## Screenshot
  ![Screenshot](https://user-images.githubusercontent.com/18093026/105174007-cb6a8d80-5b19-11eb-81ef-e4851cf69862.PNG)
