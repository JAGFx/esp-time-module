#include <Arduino.h>
#include <LedControl.h>
#include <ESP8266WiFi.h>
//#include <TimeLib.h> //TimeLib library is needed https://github.com/PaulStoffregen/Time
#include <NtpClientLib.h> //Include NtpClient library header
#include <Settings.h>

// ----------------------
// ---- Modules

// -- MAX7219 - 7Seg
#define dinPin     D8
#define clkPin     D7
#define csPin      D6
#define numDevices  1
LedControl    lc        = LedControl( dinPin, clkPin, csPin, numDevices );
unsigned long delaytime = 250;

char h[2];
char m[2];
char s[2];

// -- Wifi
#define WIFI_MAX_TRY 10

uint8_t wifiCounterTry = 0;

// ---- ./Modules
// ----------------------


// ----------------------
// ---- Common method

void writeHour() {
    lc.setChar( 0, 7, h[ 0 ], false );
    lc.setChar( 0, 6, h[ 1 ], false );
}

void writeMinute() {
    lc.setChar( 0, 4, m[ 0 ], false );
    lc.setChar( 0, 3, m[ 1 ], false );
}

void writeSecond() {
    lc.setChar( 0, 1, s[ 0 ], false );
    lc.setChar( 0, 0, s[ 1 ], false );
}

// ---- ./Common method
// ----------------------

// ----- Minimal functions

void setup() {
    Serial.begin( 115200 );
    
    // --- LedDisplay
    lc.shutdown( 0, false );
    lc.setIntensity( 0, 8 );
    lc.clearDisplay( 0 );
    
    lc.setChar( 0, 0, '-', false );
    lc.setChar( 0, 1, '-', false );
    lc.setChar( 0, 3, '-', false );
    lc.setChar( 0, 4, '-', false );
    lc.setChar( 0, 6, '-', false );
    lc.setChar( 0, 7, '-', false );
    
    delay( 100 );
    
    // --- Wifi
    Serial.println( "Wifi init..." );
    WiFi.mode( WIFI_STA );
    WiFi.begin( WIFI_SSID, WIFI_PSWD );
    
    while ( WiFi.status() != WL_CONNECTED ) {
        Serial.print( "Wifi connecting #" );
        Serial.println( ++wifiCounterTry );
        
        if ( wifiCounterTry > WIFI_MAX_TRY ) {
            wifiCounterTry = 0;
            WiFi.mode( WIFI_STA );
            WiFi.begin( WIFI_SSID, WIFI_PSWD );
            Serial.println( "Wifi: Reset" );
        }
        
        delay( 500 );
    }
    
    Serial.print( "Wifi: " );
    Serial.println( WiFi.localIP() );
    delay( 1000 );
    
    // --- Ntp client
    NTP.begin();
    NTP.setTimeZone( 1 );
    NTP.setDayLight( true );
    int counter = 0;
    
    // ---
    lc.clearDisplay( 0 );
}

const static char *deleimiter = ":";

void loop() {
    // --- Wifi connexion
    if ( WiFi.status() != WL_CONNECTED )
        ESP.restart();
    
    // --- Ntp client
    lc.clearDisplay( 0 );
    String time = NTP.getTimeStr();
    Serial.println( time );
    
    sprintf( s, "%s", time.substring( 6, 8 ).c_str() );
    sprintf( m, "%s", time.substring( 3, 5 ).c_str() );
    sprintf( h, "%s", time.substring( 0, 2 ).c_str() );
    
    writeHour();
    writeMinute();
    writeSecond();
    
    Serial.print( h );
    Serial.print( 'h' );
    Serial.print( m );
    Serial.print( 'm' );
    Serial.print( s );
    Serial.println( 's' );
    
    delay( 1000 );
}
