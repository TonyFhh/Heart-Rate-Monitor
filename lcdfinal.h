#include <LiquidCrystal.h>
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

//Creation of new set of characters
byte heart_h[8] = {
B00000,
B00000,
B01010,
B10101,
B10001,
B01010,
B00100,
B00000
};
byte heart_f[8] = {
B00000,
B00000,
B01010,
B11111,
B11111,
B01110,
B00100,
B00000
};

byte battery_empty[8] = {
    0b00000,
    0b00100,
    0b01110,
    0b01010,
    0b01010,
    0b01010,
    0b01110,
    0b00000
};

byte battery_full[8] = {
    0b00000,
    0b00100,
    0b01110,
    0b01110,
    0b01110,
    0b01110,
    0b01110,
    0b00000
};

byte battery_80[8] = {
    0b00000,
    0b00100,
    0b01110,
    0b01010,
    0b01110,
    0b01110,
    0b01110,
    0b00000
};

byte battery_60[8] = {
    0b00000,
    0b00100,
    0b01110,
    0b01010,
    0b01010,
    0b01110,
    0b01110,
    0b00000
};

void setup() {
Serial.begin(9600);
analogReference(EXTERNAL); //sets AREF to be based on wire input instead of internal voltage
lcd.begin(16, 2); // setup the LCD's number of columns and rows:
lcd.createChar (1, heart_h); // load heart_h to memory 1
lcd.createChar (2, heart_f); // load heart_f to memory 2
lcd.createChar(3, battery_full); //load battery_full to memory 3
lcd.createChar(4, battery_empty); //load battery_empty to memory 4
lcd.createChar(5, battery_80); //load battery_80 to memory 5
lcd.createChar(6, battery_60); //load battery_60 to memory 6
lcd.clear();
lcd.setCursor(0, 0); // set the cursor to column 0, row 0
lcd.print("Welcome");
delay(2000);
lcd.clear();
lcd.setCursor(0, 0); // set the cursor to column 0, row 0
lcd.print("LUB DUB's");
lcd.setCursor(0, 1);
lcd.print("Heart Monitor");
delay(3000);
lcd.clear();
}

/* Declaring Global variables */
const int nfilter = 100; //Adjust during 3d print testing
const float thrset = 0.75; //Adjust during 3d print testing
int peak = nfilter;
int threshold = thrset * peak;
int sensorvalue = 0;
unsigned long lastrefresh = 5000;
unsigned long lastarryreset = 5000;
unsigned long lastpeakcheck = 5000;
unsigned long hardreset = 5000;
int freqc[14] = {0};
int t = 0;
int bpm;
int laststate = 0;
int newstate = 0;
int trapcount = 0;

void loop() {
/* Define local variables, cleared when the iteration of the loop ends */
float calc = 0;
unsigned long sum = 0;
int nzelements = 0;

/* Bluetooth section */
sensorvalue = analogRead(0);
Serial.println(sensorvalue);

/* Highest peak value detection & adjustment */
if (sensorvalue > peak) {
peak = sensorvalue;
threshold = thrset * peak;
}

/* Peak detection based on boolean variable state change of 0 to 1 beyond threshold */
if (sensorvalue >= threshold) {
newstate = 1;
lcd.setCursor(0, 0); // set the cursor to column 0, row 0
lcd.print("              "); //clear row of text by printing space strings
lcd.setCursor(0, 0);
lcd.print("Heartbeats/Min");
if (newstate != laststate && newstate == 1) {
lcd.setCursor(15,0);
lcd.print(" ");
lcd.setCursor(15,0);
lcd.print(char(2));
Serial.print("t= ");
Serial.println(t);
freqc[t]++; //if peak detected, record data point in freqc array
trapcount=0; //variable to keep track of peak value and lower when peak not detected for some time
lastpeakcheck = millis();
}
}
else { //else if sensorvalue <= threshold
newstate = 0;
lcd.setCursor(15,0);
lcd.print(" ");
lcd.setCursor(15,0);
lcd.print(char(1));
trapcount++;
}

/* Array management, data processing and lcd display refresh section, performed every 3s */
if (millis() - lastrefresh > 5000) {
lastrefresh = millis();
sum = 0;

/* First 12s, as data is limited to 4 or less points, estimate bpm based on the available data */
if (millis() - hardreset < 21000) {
//if (millis() - hardreset < 21000) {
//for ( int i = 0; i < 12; i++ ) {
for ( int i = 0; i < 12; i++ ) {
if (freqc[i] != 0) { //set condition to only count non zero elements
sum += freqc[i];
nzelements ++;
}
}
/* Float variable "calc" used to allow decimals in bpm calculation, later truncated by integer bpm */
calc = sum * ( 12 / nzelements);
//calc = sum * ( 12 / nzelements);
bpm = int(calc);
}
/* After 12s have elapsed, start data processing. Sorts the data into a second array then
calculate median from 4, 6 or 8 values depending on available data out of the 20 element freqc array.
The original array is not used so earlier chronological entries can be overwritten after program
has elapsed for over 1min */
else {
/* variable dpi is used to determine the no. of elements of the new array, the array is
dynamic and clears from memory after each loop iteration */
int dpi = int ((millis() - hardreset) / 5000);
if (dpi > 11) { //restricts dpi to maximum value to 19, correct maximum no. of elements for new array
dpi = 11;
}
Serial.print("dpi = ");
Serial.println(dpi);
int dataproc[dpi] = {0};
for ( int w = 0; w < dpi; w++) {
    dataproc[w] = freqc[w]; //creates array and copies values from freqc
}
sort(dataproc,dpi); //sorts the array in ascending order, "sort" is custom function at bottom.
int m = 4; //m determines no. of values to use for median calc, varies based on available values (dpi).
if (dpi < 8 && dpi > 5) {
m = 6;
}
else if (dpi > 7) {
m = 8;
}
/* determines index to start summing values to calculate median, if dataproc has odd number of elements,
median sum starting point is shifted by 1 element index to the left */
int sizeofdp = dpi + 1;
float sumstart = floor((sizeofdp - m)/2); //floor rounds down value to closest integer
for ( int s = sumstart; s<(sumstart + m); s++) {
    sum += dataproc[s];
}

Serial.print("dataproc[");
for ( int z = 0; z < dpi; z++ ) {
Serial.print(dataproc[z]);
Serial.print(", ");
}
Serial.println("]");
/* Float variable "calc" used to allow decimals in bpm calculation, later truncated when passed to
integer bpm this eliminates possibility of error as bpm integer type could truncate some
 mathematical operations early ie. sum/m decimal truncated to 0 before multiplying by 20 */
calc = sum / m * 12;
//calc = sum / m * 12;
bpm = int(calc);
}
if (peak != nfilter) { //this condition prevents bpm from showing on lcd when "place finger" is printed
lcd.setCursor(0, 1);
lcd.print("              "); //clears the lcd row before printing new values
lcd.setCursor(0, 1);
lcd.print(bpm);
Serial.print("more bpm print = ");
Serial.println(bpm);
}
/* Array management for freqc, each element holds peak detected frequency for intervals of 3s
every 3s the index variable "t" shifts to next, and resets to 0 after 60s has elapsed.
Values held in that index of array also reset to 0 before frequency is redetermined. */
t++;
freqc[t] = 0;
if (millis() - lastarryreset > 60000) {
lastarryreset = millis();
t = 0;
freqc[t] = 0;
}
}
/* Peak adjustment section if no peaks have been detected for past 6s
    -Adjusts for stray noise peaks that may raise peaks unusually high
    -Performs a hard reset if peak resets to base value, simulating
     situations which user removes finger from sensor */
if (trapcount >= 300 && millis() - lastpeakcheck >= 2000) {
peak = 0.75 * peak; //peak adjustment, reduce peak by 25%, threshold recalculated later on
if (peak <= nfilter) { //Condition check to perform a hard reset
peak = nfilter;
lcd.setCursor(0, 0);
lcd.print("              ");
lcd.setCursor(0, 0);
lcd.print("Place Finger");
lcd.setCursor(0, 1);
lcd.print("              ");
hardreset = millis();
memset(freqc, 0, sizeof(freqc)); //sets all values of freqc to 0.
t = 0;
lastarryreset = millis();
lastrefresh = millis();
}
threshold = thrset * peak;
lastpeakcheck = millis();
trapcount=0;
}

batterycheck();


laststate = newstate; //prepares state variable for next loop iteration
delay(1);
}


/* Custom Functions */

/* Input arguments (array name and size) then sorts array in ascending order */
void sort(int a[], int size) {
    for(int i=0; i<(size-1); i++) {
        for(int o=0; o<(size-(i+1)); o++) {
                if(a[o] > a[o+1]) {
                    int t = a[o];
                    a[o] = a[o+1];
                    a[o+1] = t;
                }
        }
    }
}

/* Reads A1 value and checks number ranges to determine remaining battery level */
void batterycheck() {
int batterylevel = analogRead(A1);
if (batterylevel >= 657)
{
lcd.setCursor(15,1);
lcd.print(char(3));
}
else if (batterylevel <657 && batterylevel >=632) {
lcd.setCursor(15,1);
lcd.print(" ");
lcd.setCursor(15,1);
lcd.print(char(5));
}
else if (batterylevel <632 && batterylevel >=607)
{
lcd.setCursor(15,1);
lcd.print(" ");
lcd.setCursor(15,1);
lcd.print(char(6));
}
else if (batterylevel <607)
{
lcd.setCursor(15,1);
lcd.print(" ");
lcd.setCursor(15,1);
lcd.print(char(4));
}
}




