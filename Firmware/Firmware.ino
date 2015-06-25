#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM9DS0.h>

#define SERIAL_OUTPUT
#define DEBUG_OUTPUT

// 1000 = Sensor ID
Adafruit_LSM9DS0 lsm = Adafruit_LSM9DS0( 1000 );

// Chip Select Pins
#define LSM9DS0_XM_CS 10
#define LSM9DS0_GYRO_CS 9

// SPI Pins
#define LSM9DS0_SCLK 13
#define LSM9DS0_MISO 12
#define LSM9DS0_MOSI 11

#define THRESHOLD_LEFT -757
#define THRESHOLD_RIGHT -289
#define THRESHOLD_FRONT -80
#define THRESHOLD_BACK -1019

#define AVERAGE_VALUES 100

/*
#define THRESHOLD_LEFT -900
#define THRESHOLD_RIGHT -250
#define THRESHOLD_FRONT -150
#define THRESHOLD_BACK -1200
*/

int averageX, averageY, averageZ;

bool MidPosition = true;
bool MidPositionTimerRunning = false;
long MidPositionTimer = 0;
const long MidPositionTimerThreshold = 15000;

bool OutOfPosition = false;
bool OutOfPositionTimerRunning = false;
long OutOfPositionTimer = 0;
long OutOfPositionTime = 0;
const long OutOfPositionTimerThreshold = 30000;
const long OutOfPositionRepeatTimerThreshold = 15000;

bool Alert = false;

bool ChairIsTaken = false;
bool ChairIsTakenTimerRunning = false;
long ChairIsTakenTimer = 0;
const long ChairIsTakenTimerThreshold = 60000;

void setup(void)  {

	#ifdef SERIAL_OUTPUT
		Serial.begin( 115200 );
	#endif

	bool isDetected = lsm.begin();

	#ifdef SERIAL_OUTPUT
		if( !isDetected ){
			Serial.println( F( "No LSM9DS0 Detected" ) );
			while(1);
		} else {
			Serial.println( F( "LSM9DS0 Detected" ) );
		}
	#endif

	pinMode( 3 , OUTPUT );
	pinMode( 4 , OUTPUT );
	pinMode( 5 , OUTPUT );
	pinMode( 6 , OUTPUT );
	pinMode( 8 , OUTPUT );

	digitalWrite( 3 , HIGH );
	digitalWrite( 4 , HIGH );
	digitalWrite( 5 , HIGH );
	digitalWrite( 6 , HIGH );
	digitalWrite( 8 , HIGH );

	SensorConfigure();

	delay( 2000 );
}

void loop(void) {

	// Get current Values
	SensorAverage();

	MidPosition = true;
	OutOfPosition = false;

	if( averageZ <= THRESHOLD_LEFT ) {
		#ifdef DEBUG_OUTPUT
			Serial.print( " LEFT " );
		#endif
		MidPosition = false;
		OutOfPosition = true;
	} else if( averageZ >= THRESHOLD_RIGHT ) {
		#ifdef DEBUG_OUTPUT
			Serial.print( " RIGHT" );
		#endif
		MidPosition = false;
		OutOfPosition = true;
	} else {
		#ifdef DEBUG_OUTPUT
			Serial.print( " MID " );
		#endif
	}

	if( averageX >= THRESHOLD_FRONT ) {
		#ifdef DEBUG_OUTPUT
			Serial.print( " FRONT " );
		#endif
		MidPosition = false;
		OutOfPosition = true;
	} else if( averageX <= THRESHOLD_BACK ) {
		#ifdef DEBUG_OUTPUT
			Serial.print( " BACK" );
		#endif
		MidPosition = false;
		OutOfPosition = true;
	} else {
		#ifdef DEBUG_OUTPUT
			Serial.print( " MID " );
		#endif
	}

	if( MidPosition == true ) {
		if( Alert == true ) {
			if( MidPositionTimerRunning == false ) {
				MidPositionTimerRunning = true;
				MidPositionTimer = millis();
			} else {
				Serial.print( " > Time In Mid Position > " ); Serial.print( ( millis() - MidPositionTimer ) );
				if( ( millis() - MidPositionTimer ) > MidPositionTimerThreshold ) {
					#ifdef SERIAL_OUTPUT
						Serial.print( " > Long Enough In Mid Position > SUPER > Reset Alert" );
					#endif
					Alert = false;
					OutOfPositionTime = 0;
					//OutOfPositionTimerRunning = false;
				}
			}
		}
		if( ChairIsTaken == true ) {
			if( ChairIsTakenTimerRunning = false ) {
				ChairIsTakenTimerRunning = true;
				ChairIsTakenTimer = millis();
			} else {
				Serial.print( " > Chair Is Taken > " ); Serial.print( ( millis() - ChairIsTakenTimer ) );
				
				if( ( millis() - ChairIsTakenTimer ) > MidPositionTimerThreshold ) {
					#ifdef SERIAL_OUTPUT
						Serial.print( " > Reset Out Of Position Time" );
					#endif
					Alert = false;
					OutOfPositionTime = 0;
					//OutOfPositionTimerRunning = false;
				}
				if( ( millis() - ChairIsTakenTimer ) > ChairIsTakenTimerThreshold ) {
					#ifdef SERIAL_OUTPUT
						Serial.print( " > No One Sitting On The Swivel Chair!" );
					#endif
					digitalWrite( 6 , LOW ); delay( 250 );
					digitalWrite( 6 , HIGH );
					ChairIsTaken = false;
					ChairIsTakenTimer = millis();
					ChairIsTakenTimerRunning = false;
				}
			}
		}
	} else {
		MidPositionTimerRunning = false;
	}

	if( OutOfPosition == true ) {

		if( ChairIsTaken == false ) {
			#ifdef SERIAL_OUTPUT
				Serial.print( " > WELCOME ON THE SWIVEL CHAIR" );
			#endif
			digitalWrite( 3 , LOW ); delay( 250 );
			digitalWrite( 3 , HIGH );
			ChairIsTaken = true;
		} else {
			ChairIsTakenTimer = millis();
			ChairIsTakenTimerRunning = false;
		}

		if( OutOfPositionTimerRunning == false ) {
			OutOfPositionTimerRunning = true;
			OutOfPositionTimer = millis();
		} else {
			#ifdef SERIAL_OUTPUT		
				Serial.print( " > Out Of Position Time > " );
				Serial.print( OutOfPositionTime );	
			#endif
			if( Alert == false ) {
				OutOfPositionTime += ( millis() - OutOfPositionTimer );
				OutOfPositionTimer = millis();
				if( OutOfPositionTime > OutOfPositionTimerThreshold ) {
					// Alarm auslösen
					#ifdef SERIAL_OUTPUT
						Serial.print( " > WARNING " );
					#endif
					Alert = true;
					OutOfPositionTime = 0;
					digitalWrite( 4, LOW ); delay( 250 );
					digitalWrite( 4, HIGH );
				}
			} else {
				OutOfPositionTime += ( millis() - OutOfPositionTimer );
				OutOfPositionTimer = millis();
				if( OutOfPositionTime > OutOfPositionRepeatTimerThreshold ) {
					// Alarm auslösen
					#ifdef SERIAL_OUTPUT
						Serial.print( " > WARNING (AGAIN) " );
					#endif
					OutOfPositionTime = 0;
					digitalWrite( 5 , LOW ); delay( 250 );
					digitalWrite( 5 , HIGH );
				}
			}
		}
	} else {
		if( OutOfPositionTimerRunning == true ) {
			OutOfPositionTimerRunning = false;
			OutOfPositionTime += (millis() - OutOfPositionTimer);
			OutOfPositionTimer = millis();
		}
	}

	#ifdef SERIAL_OUTPUT
		Serial.println();
	#endif

}

// Configure the sensor
void SensorConfigure( void ) {
	lsm.setupAccel( lsm.LSM9DS0_ACCELRANGE_4G );
	lsm.setupMag( lsm.LSM9DS0_MAGGAIN_2GAUSS );
	lsm.setupGyro( lsm.LSM9DS0_GYROSCALE_500DPS );
}

// Determine average to soften variation
void SensorAverage(){

	long totalX = 0;
	long totalY = 0;
	long totalZ = 0;

	for( int i = 0 ; i < AVERAGE_VALUES ; i++ ) {
		lsm.read();
		totalX += (int)lsm.accelData.x;
		totalY += (int)lsm.accelData.y;
		totalZ += (int)lsm.accelData.z;
		//delay( ( 500 / AVERAGE_VALUES ) );
	}

	averageX = totalX / AVERAGE_VALUES;
	averageY = totalY / AVERAGE_VALUES;
	averageZ = totalZ / AVERAGE_VALUES;

	#ifdef DEBUG_OUTPUT
		Serial.print( "X: " ); Serial.print( averageX ); Serial.print( " / " );
		Serial.print( "Y: " ); Serial.print( averageY ); Serial.print( " / " );
		Serial.print( "Z: " ); Serial.print( averageZ ); Serial.print( " / " );
	#endif
}