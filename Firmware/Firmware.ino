#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM9DS0.h>

#define SERIAL_OUTPUT

// 1000 = Sensor ID
Adafruit_LSM9DS0 lsm = Adafruit_LSM9DS0( 1000 );

// Chip Select Pins
#define LSM9DS0_XM_CS 10
#define LSM9DS0_GYRO_CS 9

// SPI Pins
#define LSM9DS0_SCLK 13
#define LSM9DS0_MISO 12
#define LSM9DS0_MOSI 11

#define THRESHOLD_LEFT -1050
#define THRESHOLD_RIGHT -500
#define THRESHOLD_FRONT -1100
#define THRESHOLD_BACK 150

int averageX, averageY, averageZ;

bool MidPosition = true;
bool MidPositionTimerRunning = false;
long MidPositionTimer = 0;
const long MidPositionTimerThreshold = 30000;

bool OutOfPosition = false;
bool OutOfPositionTimerRunning = false;
long OutOfPositionTimer = 0;
long OutOfPositionTime = 0;
const long OutOfPositionTimerThreshold = 60000;
const long OutOfPositionRepeatTimerThreshold = 20000;

bool Alert = false;

void setup(void)  {

	#ifdef SERIAL_OUTPUT
		Serial.begin( 9600 );
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

	SensorConfigure();
}

void loop(void) {

	// Get current Values
	SensorAverage();

	MidPosition = true;
	OutOfPosition = false;

	if( averageX <= THRESHOLD_LEFT ) {
		#ifdef SERIAL_OUTPUT
			Serial.print( " LEFT " );
		#endif
		MidPosition = false;
		OutOfPosition = true;
	} else if( averageX >= THRESHOLD_RIGHT ) {
		#ifdef SERIAL_OUTPUT
			Serial.print( " RIGHT" );
		#endif
		MidPosition = false;
		OutOfPosition = true;
	} else {
		#ifdef SERIAL_OUTPUT
			Serial.print( " MID " );
		#endif
	}

	if( averageZ <= THRESHOLD_FRONT ) {
		#ifdef SERIAL_OUTPUT
			Serial.print( " FRONT " );
		#endif
		MidPosition = false;
		OutOfPosition = true;
	} else if( averageZ >= THRESHOLD_BACK ) {
		#ifdef SERIAL_OUTPUT
			Serial.print( " BACK" );
		#endif
		MidPosition = false;
		OutOfPosition = true;
	} else {
		#ifdef SERIAL_OUTPUT
			Serial.print( " MID " );
		#endif
	}

	if( MidPosition == true ) {
		if( Alert == true ) {
			if( MidPositionTimerRunning == false ) {
				MidPositionTimerRunning = true;
				MidPositionTimer = millis();
			} else {
				if( ( millis() - MidPositionTimer ) > OutOfPositionTimerThreshold ) {
					#ifdef SERIAL_OUTPUT
						Serial.println( "In Position For At Least 30000 Seconds > SUPER " );
					#endif
					Alert = false;
				}
			}
		}
	} else {
		MidPositionTimerRunning = false;
	}

	if( OutOfPosition == true ) {
		if( OutOfPositionTimerRunning == false ) {
			OutOfPositionTimerRunning = true;
			OutOfPositionTimer = millis();
		} else {
			if( Alert == false ) {
				OutOfPositionTime = millis() - OutOfPositionTimer;
				OutOfPositionTimer = millis();
				if( OutOfPositionTime > OutOfPositionTimerThreshold ) {
					// Alarm auslösen
					#ifdef SERIAL_OUTPUT
						Serial.print( "Out Of Position Time > " );
						Serial.print( OutOfPositionTime );
						Serial.println( " > WARNING " );
					#endif
					Alert = true;
					OutOfPositionTime = 0;
				}
			} else {
				OutOfPositionTime = millis() - OutOfPositionTimer;
				OutOfPositionTimer = millis();
				if( OutOfPositionTime > OutOfPositionRepeatTimerThreshold ) {
					// Alarm auslösen
					#ifdef SERIAL_OUTPUT
						Serial.print( "Out Of Position Time > " );
						Serial.print( OutOfPositionTime );
						Serial.println( " > WARNING " );
					#endif
					OutOfPositionTime = 0;
				}
			}
		}
	} else {
		if( OutOfPositionTimerRunning == true ) {
			OutOfPositionTimerRunning = false;
			OutOfPositionTime = millis() - OutOfPositionTimer;
			OutOfPositionTimer = millis();
		}
	}

	#ifdef SERIAL_OUTPUT
		Serial.print( "Position Out Of Time > " );
		Serial.print( OutOfPositionTime );
	#endif

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

	for( int i = 0 ; i < 5 ; i++ ) {
		lsm.read();
		totalX += (int)lsm.accelData.x;
		totalY += (int)lsm.accelData.y;
		totalZ += (int)lsm.accelData.z;
		delay( 50 );
	}

	averageX = totalX / 5;
	averageY = totalY / 5;
	averageZ = totalZ / 5;

	#ifdef SERIAL_OUTPUT
		Serial.print( "Average > " );
		Serial.print( averageX ); Serial.print( " / " );
		Serial.print( averageY ); Serial.print( " / " );
		Serial.println( averageZ );
	#endif
}