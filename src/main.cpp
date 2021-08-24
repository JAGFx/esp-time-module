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
LedControl    lc         = LedControl( dinPin, clkPin, csPin, numDevices );
//const byte    BRIGHTNESS = 2;
unsigned long delaytime  = 250;

char h[2];
char m[2];
char s[2];

// -- Wifi
#define WIFI_MAX_TRY 10

IPAddress ip( 192, 168, 1, 75 );
IPAddress gateway( 192, 168, 1, 1 );
IPAddress subnet( 255, 255, 255, 0 );
uint8_t   wifiCounterTry = 0;

// -- NTPClient
//#define SHOW_TIME_PERIOD 1000
#define ONBOARDLED 2 // Built in LED on ESP-12/ESP-07
#define SHOW_TIME_PERIOD 5000
#define NTP_TIMEOUT 1500

int8_t             timeZone           = 1;
int8_t             minutesTimeZone    = 0;
const PROGMEM char *ntpServer         = "pool.ntp.org";
bool               wifiFirstConnected = false;

void onSTAConnected( WiFiEventStationModeConnected ipInfo ) {
    Serial.printf( "Connected to %s\r\n", ipInfo.ssid.c_str() );
}


// Start NTP only after IP network is connected
void onSTAGotIP( WiFiEventStationModeGotIP ipInfo ) {
    Serial.printf( "Got IP: %s\r\n", ipInfo.ip.toString().c_str() );
    Serial.printf( "Connected: %s\r\n", WiFi.status() == WL_CONNECTED ? "yes" : "no" );
    digitalWrite( ONBOARDLED, LOW ); // Turn on LED
    wifiFirstConnected = true;
}

// Manage network disconnection
void onSTADisconnected( WiFiEventStationModeDisconnected event_info ) {
    Serial.printf( "Disconnected from SSID: %s\n", event_info.ssid.c_str() );
    Serial.printf( "Reason: %d\n", event_info.reason );
    digitalWrite( ONBOARDLED, HIGH ); // Turn off LED
    //NTP.stop(); // NTP sync can be disabled to avoid sync errors
    WiFi.reconnect();
}

void processSyncEvent( NTPSyncEvent_t ntpEvent ) {
    if ( ntpEvent < 0 ) {
        Serial.printf( "Time Sync error: %d\n", ntpEvent );
        if ( ntpEvent == noResponse )
            Serial.println( "NTP server not reachable" );
        else if ( ntpEvent == invalidAddress )
            Serial.println( "Invalid NTP server address" );
        else if ( ntpEvent == errorSending )
            Serial.println( "Error sending request" );
        else if ( ntpEvent == responseError )
            Serial.println( "NTP response error" );
    } else {
        if ( ntpEvent == timeSyncd ) {
            Serial.print( "Got NTP time: " );
            Serial.println( NTP.getTimeDateString( NTP.getLastNTPSync() ) );
        }
    }
}

boolean        syncEventTriggered = false; // True if a time even has been triggered
NTPSyncEvent_t ntpEvent; // Last triggered event

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
    static WiFiEventHandler e1, e2, e3;
    
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
    
    delay( delaytime );
    
    
    
    // --- Wifi
    Serial.println( "Wifi init..." );
    WiFi.config( ip, gateway, subnet );
    WiFi.mode( WIFI_STA );
    WiFi.begin( WIFI_SSID, WIFI_PSWD );
    
    pinMode( ONBOARDLED, OUTPUT ); // Onboard LED
    digitalWrite( ONBOARDLED, HIGH ); // Switch off LED
    
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
//    NTP.setInterval (63);
//    NTP.setNTPTimeout (1000);
    NTP.begin();
    NTP.setTimeZone( 1 );
    NTP.setDayLight( true );
    int counter = 0;
    
    while ( !NTP.SyncStatus()) {
        Serial.print("NTP CHECK: #");
        Serial.println(++counter);
        
        if( counter > 10 ){
            counter = 0;
            NTP.stop();
            NTP.begin();
            NTP.setTimeZone( 1 );
            NTP.setDayLight( true );
            Serial.println( "NTP: Reset" );
        }
        
        delay(500);
    };
    
    
//    NTP.begin();
    
//    NTP.onNTPSyncEvent( []( NTPSyncEvent_t event ) {
//        ntpEvent           = event;
//        syncEventTriggered = true;
//    } );
//
//    e1 = WiFi.onStationModeGotIP( onSTAGotIP );// As soon WiFi is connected, start NTP Client
//    e2 = WiFi.onStationModeDisconnected( onSTADisconnected );
//    e3 = WiFi.onStationModeConnected( onSTAConnected );
    
    lc.clearDisplay( 0 );
    
    // ---
}

void loop() {
    // --- Wifi connexion
    if ( WiFi.status() != WL_CONNECTED )
        ESP.restart();
    
    // --- Ntp client
    static int i    = 0;
    static int last = 0;
    
//    if ( wifiFirstConnected ) {
//        wifiFirstConnected = false;
//        NTP.setInterval( 63 );
//        NTP.setNTPTimeout( NTP_TIMEOUT );
//        NTP.begin( ntpServer, timeZone, true, minutesTimeZone );
//    }
    
//    if ( syncEventTriggered ) {
//        processSyncEvent( ntpEvent );
//        syncEventTriggered = false;
//    }
    
    if ( ( millis() - last ) > SHOW_TIME_PERIOD ) {


//    if ( ( millis() - last ) >= SHOW_TIME_PERIOD ) {
        // --- LedDisplay
//        lc.clearDisplay( 0 );
        
        last = millis();
        Serial.print( i );
        Serial.print( " " );
        Serial.print( NTP.getTimeDateString() );
        Serial.print( " " );
        Serial.print( NTP.isSummerTime() ? "Summer Time. " : "Winter Time. " );
        Serial.print( "WiFi is " );
        Serial.print( WiFi.isConnected() ? "connected" : "not connected" );
        Serial.print( ". " );
        Serial.print( "Uptime: " );
        Serial.print( NTP.getUptimeString() );
        Serial.print( " since " );
        Serial.println( NTP.getTimeDateString( NTP.getFirstSync() ).c_str() );
        Serial.printf( "Free heap: %u\n", ESP.getFreeHeap() );
        i++;
//        sprintf( h, "%02d", timeNtp.getHours() );
//        sprintf( m, "%02d", timeNtp.getMinutes() );
//        sprintf( s, "%02d", timeNtp.getSeconds() );

//        writeHour();
//        writeMinute();
//        writeSecond();
    }
    
//    Serial.print( h );
//    Serial.print( 'h' );
//    Serial.print( m );
//    Serial.print( 'm' );
//    Serial.print( s );
//    Serial.println( 's' );
    
    delay (0);
}
