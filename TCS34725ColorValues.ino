/* TCS34725 Color Sensor Measurement by using Adafruit TCS34725 library

   Used the https://en.wikipedia.org/wiki/HSL_and_HSV for deriving the formulas

   R, G, B, C, Illuminance (Lux), Color Temperature (K), Hue (from HSL), H2 Hue (from Cartesian Chromaticity),
   Alpha, Beta, C2 Chroma (from Cartesian Chromaticity), Intensity (I), Value (V), Lightness (L),
   Luma Y601 (SDTV BT.601 Standard), Y240 (Adobe and SMPTE 240M), Y709 (HDTV BT.709 Standard), Y2020 (UHDTV BT.2020 Standard)
   Y Luminance (Lv, Ω ), Saturation (S_V), Saturation from Lighteness (S_L) , Saturation from Intensity (S_I)

   Partha Pratim Ray
   September 11, 2023

*/


#include <Wire.h>
#include "Adafruit_TCS34725.h"

/* Initialise with specific int time and gain values */
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_614MS, TCS34725_GAIN_1X);


void setup(void) {
  Serial.begin(9600);

  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }

  // Now we're ready to get readings!
}



void loop() {
  
  uint16_t r, g, b, c, colorTemp, lux;

  tcs.getRawData(&r, &g, &b, &c);

  // colorTemp = tcs.calculateColorTemperature(r, g, b);
  colorTemp = tcs.calculateColorTemperature(r, g, b);


  // Lux is unit of illuminance. Lux unit is equal to lumen per square mete (lm/m^2)
  
  lux = tcs.calculateLux(r, g, b);


  // Normalization between 0 - 1

  float R = r / 65535.0;     
  float G = g / 65535.0;
  float B = b / 65535.0;
  float Clear = c / 65535.0;

  
  // Compute hue and chroma
  
  float M = max(max(R, G), B);
  float m = min(min(R, G), B);
  float C = M - m;

    
  float H_prime;
  float H;
   
    if (C == 0) {                // C = 0
    H_prime = 0; // Undefined
  } else if (M == R) {          // M + R
    H_prime = fmod(((G - B) / C), 6);
  } else if (M == G) {          // M == G
    H_prime = ((B - R) / C) + 2;
  } else {                       // M == B
    H_prime = ((R - G) / C) + 4;
  }
  H = 60 * H_prime;
  

  // Cartesian Chromaticity coordinates
  float alpha = 0.5 * (2 * R - G - B);
  float beta = (sqrt(3) / 2) * (G - B);
  float H2 = atan2(beta, alpha);
  float C2 = sqrt(alpha * alpha + beta * beta);


 
  // Lightness
  float I = (R + G + B) / 3.0;
  float V = M;
  float L = 0.5 * (M + m);



  //Luma Y' 
  /*Luma represents the brightness in an image and is a 
   * weighted sum of the RGB components to account for human 
   * perception. The weighting values depend on the specific 
   * color space standard used. Some common weightings include:
  */
  float Y601 = 0.2989 * R + 0.5870 * G + 0.1140 * B;   // SDTV
  float Y240 = 0.212 * R + 0.701 * G + 0.087 * B;      // Adobe
  float Y709 = 0.2126 * R + 0.7152 * G + 0.0722 * B;   // HDTV
  float Y2020 = 0.2627 * R + 0.6780 * G + 0.0593 * B;   // UHDTV




  // Convert gamma-corrected sRGB to linear RGB for Luminance 

  /* If you're starting with gamma-corrected sRGB
    values (often the case when you get values from an 8-bit image or typical display), you need to convert those gamma-corrected values into linear values before calculating the luminance. This conversion is crucial because our formula for luminance assumes linear RGB values.
  
   The sRGB color space, which is commonly used in a variety of devices like monitors, 
   digital cameras, and scanners, applies a gamma correction to images. 
   To find the linear RGB values from gamma-corrected sRGB values, 
   you'll have to apply an inverse gamma correction 
   (often referred to as the "de-gamma" process).
   For the sRGB color space, the transformation between linear and gamma-corrected values 
   is a piecewise function, not just a simple power function.

   The formula for the luminance Y in terms of linear RGB is:
   
   Y=0.2126×Rlinear+0.7152×Glinear+0.0722×Blinear

   Unit is candela per square meter (cd/m^2)
   
   */
   
  float Rlinear = sRGBToLinear(R);   
  float Glinear = sRGBToLinear(G);
  float Blinear = sRGBToLinear(B);


  float Y = 0.2126 * Rlinear + 0.7152 * Glinear + 0.0722 * Blinear; //Luminance 




  // Saturation
  float S_V = (V == 0) ? 0 : C / V;
  float S_L = (L == 1 || L == 0) ? 0 : C / (1 - abs(2 * L - 1));
  float S_I = (I == 0) ? 0 : 1 - m / I;


/*Radiance (Le,Ω):

Radiance measures the amount of light that travels in a particular direction 
from a surface per unit solid angle per unit projected area. 
It's usually measured in Watts per square meter per steradian (W/m^2·sr). 
Radiance considers both the intensity and the direction of light. 
The formula to compute radiance from RGB values is not as straightforward as the 
above formulas and depends on the specifics of the imaging system and scene conditions. 
Generally, if you have luminance (Y) and the solid angle and area considerations, 
you can compute radiance. However, for most consumer-grade imaging systems and sensors, 
direct conversion from RGB to Radiance might not be readily available without additional 
scene and sensor-specific data.


Radiance is not calculated in this code.
*/



  Serial.print("Color Temp: "); Serial.print(colorTemp, DEC); Serial.println(" K");
  Serial.print("Illuminance: "); Serial.print(lux, DEC); Serial.println(" Lux");

  Serial.print("R: "); Serial.print(r, DEC);   Serial.print(" (Normalized 0-1): "); Serial.println(R); 
  Serial.print("G: "); Serial.print(g, DEC);   Serial.print(" (Normalized 0-1): "); Serial.println(G); 
  Serial.print("B: "); Serial.print(b, DEC);   Serial.print(" (Normalized 0-1): "); Serial.println(B); 
  Serial.print("C: "); Serial.print(c, DEC);   Serial.print(" (Normalized 0-1): "); Serial.println(Clear); 
  

  // Print out the results
  Serial.print("Hue (from HSL): "); Serial.print(H); Serial.println(" °");
  Serial.print("H2: Hue (from Cartesian Chromaticity): "); Serial.println(H2);
  
  Serial.print("Alpha: "); Serial.println(alpha);
  Serial.print("Beta: "); Serial.println(beta);  

  
  Serial.print("C2: Chroma (from Cartesian Chromaticity): "); Serial.println(C2);
  
  Serial.print("I: Intensity (0-1): "); Serial.println(I);
  
  Serial.print("V: Value (0-1): "); Serial.println(V);
  
  Serial.print("L: Lightness (0-1): "); Serial.println(L);
  
  Serial.print("Y601:  Luma (SDTV standard - BT.601): "); Serial.println(Y601);
  Serial.print("Y240: Luma (Adobe RGB standard - SMPTE 240M): "); Serial.println(Y240);
  Serial.print("Y709: Luma (HDTV standard - BT.709): "); Serial.println(Y709);
  Serial.print("Y2020: Luma (UHDTV standard - BT.2020): "); Serial.println(Y2020);

  Serial.print("Y: Luminance (Lv, Ω ): "); Serial.print(Y); Serial.println(" cd/m2");

  
  Serial.print("S_V: Saturation (from Value 0-1): "); Serial.println(S_V);
  Serial.print("S_L: Saturation (from Lightness 0-1): "); Serial.println(S_L);
  Serial.print("S_I: Saturation (from Intensity 0-1): "); Serial.println(S_I);
  Serial.println();

  delay(1000);
}







  float sRGBToLinear(float value) {
      if (value <= 0.04045) {
          return value / 12.92;
      } else {
          return pow((value + 0.055) / 1.055, 2.4);
      }
  }
