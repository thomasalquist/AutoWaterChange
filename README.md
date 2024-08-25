# AutoWaterChange
Custom Arduino-based AutoWaterChange System

This AWC system uses two arduinos to initiate, monitor and complete an automated water change at the push of a button.
"No off-the-shelf product was able to keep me from carrying buckets of water upstairs from the garbage can reservoir I keep in the basement. This system is intended to keep me from carrying buckets of water. 
It's not intended to be integrated into any other aquarium controller intentionally. When desiring a water change, I shut off the return pump and let the sump settle. Once settled press the button on Arduino1. Arduino 1 will recorded the level of water in the sump, and begin to drain a calculated volume of water based on readings from the water level sensor. Once the volume of water is removed, Arduino1 will signal Arduino 2 to close the relays for a fill pump and valve which will send water upstairs into the sump. Once the water level in the sump reaches the intial water level, Arduino1 will stop sending the fill command to Arduino 2, and the cycle is complete. 

Due to the close proximity of Arduino2 to my RODI system, I chose to integrate a RODI TDS measurement/sorting system into Arduino2. It's pretty canned, so it can be discluded if not needed."

Arduino 1 is located near the sump of the tank. 
  Has a button to initiate the cycle
  eTape Water Level Sensor to monitor drain level and fill level
  Relay board to control 12V drain pump and 12V drain solenoid. 
  Sends fill command to Arduino 2 during the fill cycle

Arduino 2 is located in the basement. 
  Recieves fill command from Arduino 1
  Automatically circulates reservoir water through a sediment filter at interval
  (Bonus Feature) Measures and sorts RODI water with TDS sensor
    Uses a pressure sensor to intiate sorting when RODI is requested either to the reservoir, ATO, or to a manual fill station
    
