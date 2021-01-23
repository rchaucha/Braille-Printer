# Braille Printer 
This is a C++ / Arduino project for my [Baccalauréat](https://en.wikipedia.org/wiki/Baccalaur%C3%A9at) in a team of 3. I was in charge of the coding part while the others were in charge of the mechanical part. 

**Goal**:  To create a printer capable of printing a text entered on a computer in [braille] (https://en.wikipedia.org/wiki/Braille) format on any plane surface (e.g cardboard box) by leaving a particular substance on it. It was based on the principle of a 3D printer.  
## Description 
This project is composed of two parts:
* **C++** : This is a GUI (using Qt) running on a computer that allows the user to enter the text he wants to print. As the user types, a rectangle appears dynamically on a webcam feedback that shows the space that will be taken by the text. You can see a screenshot of the GUI below. This program is also in charge of converting the text to braille by using bytes (0 : no point, 1 : point).
* **Arduino** : This is the program uploaded to the Arduino card connected to the computer. Both programs communicate which allow the C++ software to send the data to print to the Arduino that eventually commands the motors of the printer to move accordingly.
  
## Screenshot
  ![Screenshot](https://user-images.githubusercontent.com/18093026/105174007-cb6a8d80-5b19-11eb-81ef-e4851cf69862.PNG)
  
## Physical product
The next picture shows what the printer looks like. The Arduino is in the red box, there are 3 motors (for 3 axis) and we used a syringe to leave the substance. 

![braille](https://user-images.githubusercontent.com/18093026/105184796-aa109e00-5b27-11eb-8e9d-e2e330db9cb0.jpg)
  
## Demo
Here you can see a demo of the product using a pen to symbolize the substance left by the printer for the sake of the test.
At the end you can see the word that we instructed the program to print, witch matches the word actually printed.

![braille_demo](https://user-images.githubusercontent.com/18093026/105183632-53ef2b00-5b26-11eb-8c68-c28b116bdab7.gif)
