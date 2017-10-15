# Heart-Rate-Monitor
# Contains Arduino code for prior academic project.

Working Principle
The microcontroller is supported by an electrical circuit that functions like a photoplethysmograph, with a LED emitting light
through a finger, then measuring the reflected light. Each time the heart rate beats, it induces a pulse that can measured as peaks
in voltage, the signal then undergoes several signal processing steps before being fed into the Arduino controller's input sensor
as a range of analog voltage values between 0-5V which are then assigned as values of 0-1023 by Arduino's 8 bit ADC.

General Flow of code:
The microcontroller starts with a initial prediced value for peak spikes. As the peak spikes are not all of the same amplitude,
a variance of 25% from the peak is allowed, denoted by threshold values. The program runs in a endless loop, reading values from
Arduino's input sensors.

The values from previous loop are constantly retained in variables, if a new higher amplitude peak is found, the new peak is taken
to be the new peak for interpreting heart rates. However, spikes in peak values may be due to noise thus after a short period of
time this peak value also drops by a certain % if no heartbeats are detected within the time frame.

The actual heartrate detection in the algorithm stems from a logic boolean, which changes state to 1 whenever the heartbeat
threshold value is crossed, and stays as 1 until the value falls below threshold, interpreting 1 heartbeat for each cycle of such
an occurance. This prevents double counting. For the actual calculation of heartbeats/min (bpm), the no. of heartbeats is measured
in time frames of 5s, with heartbeat counts of a full minute stored within a 12 element array. Every 5s, recorded heartbeats
increment respective elements of the array by 1. After 1 minute, the first element of the array is then reset then new recorded
heartbeats are once again recorded in that element of the array and repeating every 5s, resetting next element and storing data. 
For each update of the bpm section, the array is copied, sorted in ascending order, the median of the dataset is determined and
used to calculate the bpm.

Other features
The program supports LCD module, which performs several functions:
1. "flashing heart" icon whenever a heartbeat is detected
2. battery indicator
3. User prompts
4. Initialising messages

The first 2 features are generated using byte data structures, which assign shapes to a 8x5 pixel of the lcd column. for instance
the heart_h byte character generates a empty heart shape in the lcd. This proceedure is repeated to generate the characters for
various scenarios to fulfill the functionality of the first 2. An extra wire is fed into another arduino sensor to check voltage
of the battery which the battery indicator based its reading from.

User prompts form part of the peak drop features. Whenever no peaks are detected after a time frame period, the peak value drops
by a certain % and instead of showing bpm, the lcd prints string that prompts user to "place finger". This feature also occurs
if readings are consistently below initial set values of peak detection such as during initialisation.

Lastly initialising messages are very simple lcd print commands during initialisation.

Known issues:
- Using median to calculate bpm may be fairly inaccurate, other methods such as moving average may work better or alternatively
  other techniques of orgainising data.
- The initial values of peak detection present difficulties stemming from hard coding, in addition also making program
  susceptible to environmental factors that may contribute to noise that cause background readings to be above the threshold and
  erronously be counted as heartbeats.
