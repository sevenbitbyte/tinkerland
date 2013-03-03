#include <Wire.h>

#define COMM_PACKET_STX  (0x81)
#define COMM_SEND_WATCHDOG_MS (500)

#define CTRL_REG1_A 0x20
#define CTRL_REG2_A 0x21
#define CTRL_REG3_A 0x22
#define CTRL_REG4_A 0x23
#define CTRL_REG5_A 0x24

//CTRL_REG1_A - Ctrl register 1 fields
#define CTRL_POWER_MODE_SHIFT   (5)
#define CTRL_POWER_MODE_MASK    (0x7 << CTRL_POWER_MODE_SHIFT)
#define CTRL_DATA_RATE_SHIFT    (3)
#define CTRL_DATA_RATE_MASK     (0x3 << CTRL_DATA_RATE_SHIFT)
#define CTRL_Z_EA_SHIFT         (2)
#define CTRL_Z_EA_MASK          (0x1 << CTRL_Z_EA_SHIFT)
#define CTRL_Y_EA_SHIFT         (1)
#define CTRL_Y_EA_MASK          (0x1 << CTRL_Y_EA_SHIFT)
#define CTRL_X_EA_SHIFT         (0)
#define CTRL_X_EA_MASK          (0x1 << CTRL_X_EA_SHIFT)

//Data rate values
#define DATA_RATE_50Hz      (0x0 << CTRL_DATA_RATE_SHIFT)
#define DATA_RATE_100Hz     (0x1 << CTRL_DATA_RATE_SHIFT)
#define DATA_RATE_400Hz     (0x2 << CTRL_DATA_RATE_SHIFT)
#define DATA_RATE_1000Hz    (0x3 << CTRL_DATA_RATE_SHIFT)

//Power mode values
#define POWER_MODE_OFF      (0x0 << CTRL_POWER_MODE_SHIFT)
#define POWER_MODE_NORMAL   (0x1 << CTRL_POWER_MODE_SHIFT)
#define POWER_MODE_LOW_05Hz (0x2 << CTRL_POWER_MODE_SHIFT)
#define POWER_MODE_LOW_1Hz  (0x3 << CTRL_POWER_MODE_SHIFT)
#define POWER_MODE_LOW_2Hz  (0x4 << CTRL_POWER_MODE_SHIFT)
#define POWER_MODE_LOW_5Hz  (0x5 << CTRL_POWER_MODE_SHIFT)
#define POWER_MODE_LOW_10Hz (0x6 << CTRL_POWER_MODE_SHIFT)

//CTRL_REG2_A - Ctrl register 2 fields
#define CTRL_BOOT_SHIFT         (7)
#define CTRL_BOOT_MASK          (1 << CTRL_BOOT_SHIFT)
#define CTRL_HIGH_PASS_SHIFT    (5)
#define CTRL_HIGH_PASS_MASK     (0x3 << CTRL_HIGH_PASS_SHIFT)
#define CTRL_FDS_SHIFT          (4)
#define CTRL_FDS_MASK           (0x1 << CTRL_FDS_SHIFT)
#define CTRL_HPen2_SHIFT        (3)
#define CTRL_HPen2_MASK         (0x1 << CTRL_HPen2_SHIFT)
#define CTRL_HPen1_SHIFT        (2)
#define CTRL_HPen1_MASK         (0x1 << CTRL_HPen1_SHIFT)
#define CTRL_HPCF_SHIFT         (0)
#define CTRL_HPCF_MASK          (0x3 << CTRL_HPCF0_SHIFT)

//CTRL_REG3_A - Ctrl register 3 fields
#define CTRL_INT_DIR_SHIFT      (7)
#define CTRL_INT_DIR_MASK       (0x1 << CTRL_INT_DIR_SHIFT)
#define CTRL_OPEN_DRAIN_SHIFT   (6)
#define CTRL_OPEN_DRAIN_MASK    (0x1 << CTRL_OPEN_DRAIN_SHIFT)
#define CTRL_INT2_LATCH_SHIFT   (5)
#define CTRL_INT2_LATCH_MASK    (0x1 << CTRL_INT2_LATCH_SHIFT)
#define CTRL_INT2_SIGNAL_SHIFT  (3)
#define CTRL_INT2_SIGNAL_MASK   (0x3 << CTRL_INT2_LATCH_SHIFT)
#define CTRL_INT1_LATCH_SHIFT   (2)
#define CTRL_INT1_LATCH_MASK    (0x1 << CTRL_INT1_LATCH_SHIFT)
#define CTRL_INT1_SIGNAL_SHIFT  (0)
#define CTRL_INT1_SIGNAL_MASK   (0x3 << CTRL_INT1_LATCH_SHIFT)

//CTRL_REG4_A - Ctrl register 4 fields
#define CTRL_BLOCK_UPDATE_SHIFT  (7)
#define CTRL_BLOCK_UPDATE_MASK   (0x1 << CTRL_BLOCK_UPDATE_SHIFT)
#define CTRL_ENDIAN_SHIFT        (6)
#define CTRL_ENDIAN_MASK         (0x1 << CTRL_ENDIAN_SHIFT)
#define CTRL_MAG_SCALE_SHIFT     (4)
#define CTRL_MAG_SCALE_MASK      (0x3 << CTRL_MAG_SCALE_SHIFT)
#define CTRL_SELF_TEST_SHIFT     (3)
#define CTRL_SELF_TEST_MASK      (0x1 << CTRL_SELF_TEST_SHIFT)
#define CTRL_SELF_TEST_EA_SHIFT  (1)
#define CTRL_SELF_TEST_EA_MASK   (0x1 << CTRL_SELF_TEST_EA_SHIFT)

//Magnetic scale adjust
#define MAG_SCALE_2g    (0x0 << CTRL_MAG_SCALE_SHIFT)
#define MAG_SCALE_4g    (0x1 << CTRL_MAG_SCALE_SHIFT)
#define MAG_SCALE_8g    (0x3 << CTRL_MAG_SCALE_SHIFT)

//CTRL_REG5_A - Ctrl register 5 fields
#define CTRL_SLEEP_WAKE_SHIFT    (0)
#define CTRL_SLEEP_WAKE_MASK     (0x3 << CTRL_SLEEP_WAKE_SHIFT)

//Sleep wake values
#define SLEEP_WAKE_DISABLED (0x0 << CTRL_SLEEP_WAKE_SHIFT)
#define SLEEP_WAKE_ENABLED  (0x3 << CTRL_SLEEP_WAKE_SHIFT)

//Status register fields
#define STATUS_ZYX_OVERRUN_SHIFT        (7)
#define STATUS_Z_OVERRUN_SHIFT          (6)
#define STATUS_Y_OVERRUN_SHIFT          (5)
#define STATUS_X_OVERRUN_SHIFT          (4)
#define STATUS_ZYX_DATA_AVAILABLE_SHIFT (3)
#define STATUS_Z_DATA_AVAILABLE_SHIFT   (2)
#define STATUS_Y_DATA_AVAILABLE_SHIFT   (1)
#define STATUS_X_DATA_AVAILABLE_SHIFT   (0)

#define STATUS_ZYX_OVERRUN_MASK         (1<<STATUS_ZYX_OVERRUN_SHIFT)
#define STATUS_Z_OVERRUN_MASK           (1<<STATUS_Z_OVERRUN_SHIFT)
#define STATUS_Y_OVERRUN_MASK           (1<<STATUS_Y_OVERRUN_SHIFT)
#define STATUS_X_OVERRUN_MASK           (1<<STATUS_X_OVERRUN_SHIFT)
#define STATUS_ZYX_DATA_AVAILABLE_MASK  (1<<STATUS_ZYX_DATA_AVAILABLE_SHIFT)
#define STATUS_Z_DATA_AVAILABLE_MASK    (1<<STATUS_Z_DATA_AVAILABLE_SHIFT)
#define STATUS_Y_DATA_AVAILABLE_MASK    (1<<STATUS_Y_DATA_AVAILABLE_SHIFT)
#define STATUS_X_DATA_AVAILABLE_MASK    (1<<STATUS_X_DATA_AVAILABLE_SHIFT)

#define HP_FILTER_RESET_A   0x25
#define REFERENCE_A         0x26
#define STATUS_REG_A        0x27
#define OUT_X_LOW_ACCEL     0x28
#define OUT_X_HIGH_ACCEL    0x29
#define OUT_Y_LOW_ACCEL     0x2a
#define OUT_Y_HIGH_ACCEL    0x2b
#define OUT_Z_LOW_ACCEL     0x2c
#define OUT_Z_HIGH_ACCEL    0x2d
#define INT1_CFG_A          0x30
#define INT1_SOURCE_A       0x31
#define INT1_THS_A          0x32
#define INT1_DURATION_A     0x33
#define INT2_CFG_A          0x34
#define INT2_SOURCE_A       0x35
#define INT2_THS_A          0x36
#define INT2_DURATION_A     0x37

#define CRA_REG_M           0x00
#define CRB_REG_M           0x01
#define MR_REG_M            0x02
#define OUT_X_HIGH_M        0x03
#define OUT_X_LOW_M         0x04
#define OUT_Y_HIGH_M        0x05
#define OUT_Y_LOW_M         0x06
#define OUT_Z_HIGH_M        0x07
#define OUT_Z_LOW_M         0x08
#define SR_REG_Mg           0x09
#define IRA_REG_M           0x0a
#define IRB_REG_M           0x0b
#define IRC_REG_M           0x0c


//CRA_REG_M Fields
#define CRA_DATA_RATE_SHIFT         (2)
#define CRA_DATA_RATE_MASK          (0x7 << CRA_DATA_RATE_SHIFT)
#define CRA_MEASUREMENT_BIAS_SHIFT  (0)
#define CRA_MEASUREMENT_BIAS_MASK   (0x3 << CRA_MEASUREMENT_BIAS_SHIFT)

//CRA Data Rate values
#define CRA_DATA_RATE_0_75Hz        (0x0)
#define CRA_DATA_RATE_1_5Hz         (0x1)
#define CRA_DATA_RATE_3Hz           (0x2)
#define CRA_DATA_RATE_7_5Hz         (0x3)
#define CRA_DATA_RATE_15Hz          (0x4)
#define CRA_DATA_RATE_30Hz          (0x5)
#define CRA_DATA_RATE_75Hz          (0x6)

//CRA Measurement bias values
#define CRA_MEASUREMENT_BIAS_NORMAL     (0x0)
#define CRA_MEASUREMENT_BIAS_POSITIVE   (0x1)
#define CRA_MEASUREMENT_BIAS_NEGATIVE   (0x2)

#define IRA_REG_M_MASK  (0x48)
#define IRB_REG_M_MASK  (0x34)
#define IRC_REG_M_MASK  (0x33)

#define LSM303_MAGNETOMETER_ADDR    0x1e
#define LSM303_ACCELEROMETER_ADDR   0x18


unsigned int issueDepth = 0;
unsigned int reissues = 0;

#define PWM_COUNTER_MASK	(0x3ff)

#define SHIFT_BRITE_CLK		31
#define SHIFT_BRITE_EA		33
#define SHIFT_BRITE_LATCH	35
#define SHIFT_BRITE_DATA	37


enum I2CStatusType { I2C_Detect=0, I2C_Init=1, I2C_Run=2, I2C_Error=3 };
enum ShiftBriteStatusType { SB_Init=0, SB_Step=1, SB_Run=2, SB_Stop=3 };
enum OrientationType { DirectionNormal=0, DirectionLeft=1, DirectionInverted=2, DirectionRight=3 };

struct SystemStatus{
		//Accelerometer status
		I2CStatusType i2cStatus;
		OrientationType orientation;
		unsigned int accelSampleRate;

		//ShiftBrite status
		ShiftBriteStatusType sbStatus;
		unsigned int sbIndex;
		unsigned int sbFPS;

                unsigned short requestFlags;
};

SystemStatus systemStatus;

void writeShiftBriteCounters(uint32_t r, uint32_t g, uint32_t b){
	uint32_t buffer = (g & PWM_COUNTER_MASK);
	buffer |= ((r & PWM_COUNTER_MASK) << 10);
	buffer |= ((b & PWM_COUNTER_MASK) << 20);

	/*Serial.print("buffer=");
	Serial.print(buffer, HEX);
	Serial.print(" r=");
	Serial.print(r, HEX);
	Serial.print(" g=");
	Serial.print(g, HEX);
	Serial.print(" b=");
	Serial.println(b, HEX);*/

	for(int i=31; i>=0; i--){
		//Setup data output
		if(( (buffer>> i) & 0x1) == 1){
			digitalWrite(SHIFT_BRITE_DATA, HIGH);
		}
		else{
			digitalWrite(SHIFT_BRITE_DATA, LOW);
		}
		//buffer = buffer >> 1;

		//Cycle clock
		//delayMicroseconds(15);
		digitalWrite(SHIFT_BRITE_CLK, HIGH);
		//delayMicroseconds(15);
		digitalWrite(SHIFT_BRITE_CLK, LOW);
		//delayMicroseconds(15);
	}
	//latchShiftBrite();
}

void latchShiftBrite(){
	//Latch in data
	//delayMicroseconds(10);
	digitalWrite(SHIFT_BRITE_LATCH, HIGH);
	//delayMicroseconds(10);
	digitalWrite(SHIFT_BRITE_LATCH, LOW);
	//delayMicroseconds(10);
}

/*void enableShiftGrite(){
	digitalWrite(SHIFT_BRITE_LATCH, HIGH);
}*/

void initShiftBrite(){
	systemStatus.sbStatus = SB_Init;
	//Set pin modes to ouput
	pinMode(SHIFT_BRITE_EA, OUTPUT);
	//digitalWrite(SHIFT_BRITE_EA, HIGH);

	pinMode(SHIFT_BRITE_DATA, OUTPUT);
	pinMode(SHIFT_BRITE_LATCH, OUTPUT);
	pinMode(SHIFT_BRITE_CLK, OUTPUT);

	//Set idle states
	digitalWrite(SHIFT_BRITE_CLK, LOW);
	digitalWrite(SHIFT_BRITE_LATCH, LOW);
	digitalWrite(SHIFT_BRITE_DATA, LOW);

	digitalWrite(SHIFT_BRITE_EA, HIGH);
	delayMicroseconds(15);
	digitalWrite(SHIFT_BRITE_EA, LOW);
	delayMicroseconds(15);

	writeShiftBriteCounters(0, 0, 0);
	//writeShiftBriteCounters(0, 0, 0x3ff);
	latchShiftBrite();
}

byte readRegister(byte regAddr){
	char i2cAddr = LSM303_ACCELEROMETER_ADDR;   //Set i2cAddr for accelerometer
	if(regAddr < 0x20){
		i2cAddr = LSM303_MAGNETOMETER_ADDR;      //Set i2cAddr for magnetometer
	}

	Wire.beginTransmission(i2cAddr);      //Start i2c transaction
	Wire.write(regAddr);                              //Send register address with read bit set
	Wire.endTransmission(false);                            //Send restart
	Wire.requestFrom(i2cAddr, 1, true);   //Begin slave to host transfer and terminate connection

	int count = 0;
	while(Wire.available() < 1){
		delay(5);
		//Serial.println("DEBUG: readRegister - waiting");
		count++;
		if(count > 5){
			//Serial.println("DEBUG: readRegister - reissuing");
			reissues++;
			return readRegister(regAddr);
		}
	}

	return Wire.read();
}

void writeRegister(byte regAddr, char value){
	char i2cAddr = LSM303_ACCELEROMETER_ADDR;   //Set i2cAddr for accelerometer
	if(regAddr < 0x20){
		i2cAddr = LSM303_MAGNETOMETER_ADDR;      //Set i2cAddr for magnetometer
	}

	Wire.beginTransmission(i2cAddr);      //Start i2c transaction
	Wire.write(regAddr & 0x7f);                              //Send register address with write bit set
	Wire.write(value);                                      //Send data
	Wire.endTransmission();
}

void writeRegister(byte regAddr, char* value, unsigned int length){
	char i2cAddr = LSM303_ACCELEROMETER_ADDR;   //Set i2cAddr for accelerometer
	if(regAddr < 0x20){
		i2cAddr = LSM303_MAGNETOMETER_ADDR;      //Set i2cAddr for magnetometer
	}

	Wire.beginTransmission(i2cAddr);      //Start i2c transaction
	Wire.write(regAddr);                              //Send register address with write bit set
	for(int i=0; i<length; i++){
		Wire.write(value[i]);  //Send data
	}
	Wire.endTransmission();
}

/**
  @return   Returns true if an LSM303 was detected on the i2c bus
  */
bool detectLSM303(){
	systemStatus.i2cStatus = I2C_Detect;
	char idRegA = readRegister(IRA_REG_M);
	char idRegB = readRegister(IRB_REG_M);
	char idRegC = readRegister(IRC_REG_M);

	/*Serial.print("IRA: \t");
	Serial.print(idRegA, BIN);
	Serial.print("\t");
	Serial.println((char)IRA_REG_M_MASK, BIN);

	Serial.print("IRB: \t");
	Serial.print(idRegB, BIN);
	Serial.print("\t");
	Serial.println((char)IRB_REG_M_MASK, BIN);

	Serial.print("IRC: \t");
	Serial.print(idRegC, BIN);
	Serial.print("\t");
	Serial.println((char)IRC_REG_M_MASK, BIN);*/


	return (idRegA == IRA_REG_M_MASK) && (idRegB == IRB_REG_M_MASK) && (idRegC == IRC_REG_M_MASK);
}

void initLSM303(){
	systemStatus.i2cStatus = I2C_Init;
	//Setup accelerometer
	Serial.println("DEBUG: Setting up accelerometer");
	writeRegister(CTRL_REG1_A, DATA_RATE_50Hz | POWER_MODE_NORMAL | CTRL_Z_EA_MASK | CTRL_Y_EA_MASK | CTRL_X_EA_MASK);
	writeRegister(CTRL_REG2_A, 0x0 /*| CTRL_FDS_MASK*/);
	//Set magnitude of accelerometer and enable block data update
	writeRegister(CTRL_REG4_A, 0x0 | MAG_SCALE_2g | CTRL_BLOCK_UPDATE_MASK);
}

struct LEDState{
		uint8_t channelMask;
		int16_t startValue;
		int16_t endValue;

		unsigned long startTime;
		unsigned long endTime;

		uint16_t durationMs;
		//int16_t intervalUs;	//Only used when in BLINK_MODE
		char mode;

		struct LEDState* next;
} ;//LEDState;

enum Led_Channel{RED_CHANNEL=0x1, GREEN_CHANNEL=0x2, BLUE_CHANNEL=0x4};
enum Led_States{FADEIN_MODE, FADEOUT_MODE, CONSTANT_MODE, BLINK_MODE};

struct LEDState* root = NULL;

struct LEDState allFadeIn = /*(struct LEDState)*/{0x1, (uint16_t)0, (uint16_t)1024, 0, 0, (uint16_t)5000, FADEIN_MODE, NULL};
struct LEDState allFadeIn2 = /*(struct LEDState)*/{0x1, (uint16_t)0, (uint16_t)1024, 0, 0, (uint16_t)5000, FADEIN_MODE, NULL};
struct LEDState allFadeIn3 = /*(struct LEDState)*/{0x1, (uint16_t)0, (uint16_t)1024, 0, 0, (uint16_t)5000, FADEIN_MODE, NULL};
struct LEDState allFadeIn4 = /*(struct LEDState)*/{0x1, (uint16_t)0, (uint16_t)1024, 0, 0, (uint16_t)5000, FADEIN_MODE, NULL};
struct LEDState allFadeIn5 = /*(struct LEDState)*/{0x1, (uint16_t)0, (uint16_t)1024, 0, 0, (uint16_t)5000, FADEIN_MODE, NULL};
struct LEDState allOff = /*(struct LEDState)*/{0x1, 0, 0, 0, 0, (uint16_t)500, FADEOUT_MODE, &allFadeIn};
struct LEDState allFadeOut = /*(struct LEDState)*/{0x1, 1024, 0, 0, 0, (uint16_t)5000, FADEOUT_MODE, &allOff};


struct LEDState* currentState;

enum PacketTypes{PktOrientation=1, PktAccValues=2, PktAccStatus=4, PktMagValues=8, PktMagStatus=16, PktDeviceStatus=32, PktHostCmd=64};

void setup(){
	systemStatus.orientation = DirectionNormal;
	systemStatus.accelSampleRate = 0;
	systemStatus.sbIndex = 0;
	systemStatus.sbFPS = 0;
	systemStatus.requestFlags = PktDeviceStatus;

	//Setup serial
	Serial.begin(115200);
	Serial.println("LSM303 Tester");

	//Setup shift brite
	initShiftBrite();

	allFadeIn.channelMask = RED_CHANNEL;
	allFadeIn.durationMs = 2000;
	allFadeIn.endValue = 1024;
	allFadeIn.startValue = 0;
	allFadeIn.mode = FADEIN_MODE;
	allFadeIn.next = &allFadeIn2;

	allFadeIn2.channelMask = GREEN_CHANNEL;
	allFadeIn2.durationMs = 2000;
	allFadeIn2.endValue = 1024;
	allFadeIn2.startValue = 0;
	allFadeIn2.mode = FADEIN_MODE;
	allFadeIn2.next = &allFadeOut;

	allFadeOut.channelMask = BLUE_CHANNEL;
	allFadeOut.durationMs = 2000;
	allFadeOut.endValue = 1024;
	allFadeOut.startValue = 0;
	allFadeOut.mode = FADEOUT_MODE;
	allFadeOut.next = &allFadeIn3;

	allFadeIn3.channelMask = RED_CHANNEL | BLUE_CHANNEL | GREEN_CHANNEL;
	allFadeIn3.durationMs = 3000;
	allFadeIn3.endValue = 512;
	allFadeIn3.startValue = 0;
	allFadeIn3.mode = FADEIN_MODE;
	allFadeIn3.next = &allFadeIn4;

	allFadeIn4.channelMask = RED_CHANNEL | BLUE_CHANNEL | GREEN_CHANNEL;
	allFadeIn4.durationMs = 2000;
	allFadeIn4.endValue = 0;
	allFadeIn4.startValue = 512;
	allFadeIn4.mode = FADEIN_MODE;
	allFadeIn4.next = &allFadeIn5;

	allFadeIn5.channelMask = RED_CHANNEL | BLUE_CHANNEL | GREEN_CHANNEL;
	allFadeIn5.durationMs = 500;
	allFadeIn5.endValue = 0;
	allFadeIn5.startValue = 0;
	allFadeIn5.mode = FADEIN_MODE;
	allFadeIn5.next = &allFadeIn3;

	allOff.channelMask = 0x7;
	allOff.durationMs = 50;
	allOff.endValue = 0;
	allOff.startValue = 0;
	allOff.mode = FADEOUT_MODE;
	allOff.next = &allFadeIn;

	root = &allFadeIn;
	//allFadeIn.next = &allFadeIn;

	currentState = root;

	//root->startTime = millis();
	//root->endTime = root->startTime + root->durationMs;



	//Setup i2c bus
	Wire.begin();

	if(detectLSM303()){
		//Serial.println("LSM303 detected");
		initLSM303();
	}
	else{
		//Serial.println("ERROR: Failed to detect LSM303!");
		systemStatus.i2cStatus = I2C_Error;
	}

	//Serial.println("DEBUG: Polling...");
	delay(2000);
}

short accel[3] = {0, 0, 0};

bool accelUpdated = false;
uint32_t accelOverruns = 0;
uint32_t accelReads = 0;



void sendStatusPacket(){
	byte packet[11];

	packet[0] = COMM_PACKET_STX;
	packet[1] = PktDeviceStatus;
	packet[2] = 8;
	packet[3] = systemStatus.i2cStatus;
	packet[4] = systemStatus.orientation;
	((unsigned short*)(&packet[5]))[0] = systemStatus.accelSampleRate;
	packet[7] = systemStatus.sbStatus;
	packet[8] = systemStatus.sbIndex;
	((unsigned short*)(&packet[9]))[0] = systemStatus.sbIndex;

	Serial.write(packet, 11);
}

void sendOrientationPacket(){
	// [STX][TYPE][LENGTH]{[LSM_303_STATUS] [DIRECTION] [ACCEL] }
	byte packet[4];

	packet[0] = COMM_PACKET_STX;  //129 decimal
	packet[1] = PktOrientation;
	packet[2] = 0x1;
	packet[3] = systemStatus.orientation;

	Serial.write(packet, 4);
}


void sendAccelValues(){
	byte packet[3 + (3*sizeof(short))];

	packet[0] = COMM_PACKET_STX;
	packet[1] = PktAccValues;
	packet[2] = sizeof(short) * 3;

	memcpy(&packet[3], (void*) &accel[0], sizeof(short));
	memcpy(&packet[3 + sizeof(short)], (void*) &accel[1], sizeof(short));
	memcpy(&packet[3 + 2*sizeof(short)], (void*) &accel[2], sizeof(short));

	Serial.write(packet, 3 + (sizeof(short)*3));
}


/*void sendAccelAxisValue(int index){
	byte packet[3 + 1 + sizeof(int)];

	packet[0] = COMM_PACKET_STX;
	packet[1] = PktAccValues;
	packet[2] = sizeof(int) + 1;
	packet[3] = index;
    memcpy(&packet[4], (void*) &accel[index], sizeof(int));

	Serial.write(packet, 4 + sizeof(int));
}*/

unsigned long previousTimeMs = 0;
unsigned long currentTimeMs = 0;

uint16_t colorChannels[3];// = {0, 0, 0};
uint16_t updateCount = 0;
//unsigned long swapTime = 0;

struct DevicePacket{
	PacketTypes type;
	unsigned char length;
	unsigned char index;	//! index is the location that the next read byte will be placed. Reading is complete when (index == length)
	byte data[10];
};

enum RecvState { Rx_Idle=0, Rx_PacketLength, Rx_PacketType, Rx_PacketData};

struct DevicePacket packetBuffer;
RecvState packetRxState = Rx_Idle;

void processSerialCommand(){
	/*
	 *		[STX] [Type] [Length] [RequestFlags]
	 */

	if( ((PacketTypes)packetBuffer.type) == PktHostCmd ){
		systemStatus.requestFlags = ((unsigned short*)&packetBuffer.data)[0];
	}
}

void readSerialCommand(){
	int bytesReady = Serial.available();

	if(bytesReady > 0){
		for(int i=0; i<bytesReady; i++){
			byte temp = Serial.read();

			if(packetRxState == Rx_Idle && temp == COMM_PACKET_STX){
				//Reset packet buffer
				packetBuffer.type = (PacketTypes) 0;
				packetBuffer.length = 0;
				packetBuffer.index = 0;
				packetRxState = Rx_PacketType;
			}
			else if(packetRxState == Rx_PacketType){
				if( (PacketTypes) temp != PktHostCmd){
					//Invalid packet type, start over
					packetRxState = Rx_Idle;
					continue;
				}

				packetBuffer.type = (PacketTypes) temp;
				packetRxState = Rx_PacketLength;
			}
			else if(packetRxState == Rx_PacketLength){
				if(temp > sizeof(packetBuffer.data)){
					//Invalid packet lenght, start over
					packetRxState = Rx_Idle;
					continue;
				}

				packetBuffer.length = temp;
				packetBuffer.index = 0;
				packetRxState = Rx_PacketData;
			}
			else if(packetRxState == Rx_PacketData){
				if(packetBuffer.index < packetBuffer.length){
					packetBuffer.data[packetBuffer.index++] = temp;

					if(packetBuffer.index == packetBuffer.length){
						//Run command
						processSerialCommand();

						//Get ready for next packet
						packetRxState = Rx_Idle;
					}
				}
				else{
					//Somethings not right, start over
					packetRxState = Rx_Idle;
				}
			}
		}
	}
}

void loop(){


	currentTimeMs = millis();
	unsigned long deltaMs = currentTimeMs - previousTimeMs;
	unsigned long spentMs = currentTimeMs - currentState->startTime;

	if(spentMs <= currentState->durationMs){
		//Update color channels
		uint16_t value = 0;

		if(currentState->mode == FADEIN_MODE || currentState->mode == FADEOUT_MODE){
			float percent = (((float)currentTimeMs - (float)currentState->startTime)) / ((float)currentState->durationMs);
			percent = (1 + (-cos(percent/* * 2.0*/ * M_PI))) / 2.0f;
			value = (percent * (currentState->endValue - currentState->startValue)) + currentState->startValue;
		}
		else if(currentState->mode == CONSTANT_MODE){
			value = currentState->startValue;
		}


		colorChannels[0] = currentState->endValue;
		colorChannels[1] = currentState->endValue;
		colorChannels[2] = currentState->endValue;

		if((currentState->channelMask & (uint8_t)RED_CHANNEL) != 0){
			colorChannels[0] = value;
		}
		if((currentState->channelMask & GREEN_CHANNEL) != 0){
			colorChannels[1] = value;
		}
		if((currentState->channelMask & BLUE_CHANNEL) != 0){
			colorChannels[2] = value;
		}

		/*colorChannels[0] = millis()%1024;
		colorChannels[1] = 1024 - (millis()%1024);
		colorChannels[2] = 0;*/

		/*Serial.print("Brite = ");
		Serial.print(value, DEC);
		Serial.print(" {");
		Serial.print(colorChannels[0], DEC);
		Serial.print(",\t");
		Serial.print(colorChannels[1], DEC);
		Serial.print(",\t");
		Serial.print(colorChannels[2], DEC);
		Serial.println("}");*/

		//writeShiftBriteCounters(colorChannels[0], colorChannels[1], colorChannels[2]);
		//latchShiftBrite();
	}
	else{
		if(currentState->next != NULL){
			currentState = currentState->next;
			currentState->startTime = currentTimeMs;
			currentState->endTime = currentTimeMs + currentState->durationMs;

			//Serial.print("\t updates=");
			//Serial.println(updateCount, DEC);

			updateCount = 1;	//Very strange, set	to 0 does not compile set to 1 it compiles WTF?

			/*Serial.print("Switched, start=");
			Serial.print(currentState->startTime, DEC);
			Serial.print("\t end=");
			Serial.print(currentState->endTime, DEC);
			Serial.print("\t duration=");
			Serial.print(currentState->durationMs, DEC);
*/

		}
		else{
			//Serial.println("Else");
			currentState = root;
		}


	}

	writeShiftBriteCounters(colorChannels[0], colorChannels[1], colorChannels[2]);
	//writeShiftBriteCounters(accel[0], accel[1], accel[2]);
	latchShiftBrite();
	updateCount++;
	//return;

	//writeShiftBriteCounters(millis()%1024, 1024 - (millis()%1024), 0);
	//latchShiftBrite();


	char accelStatus = readRegister(STATUS_REG_A);
	//Serial.print("Status: ");
	//Serial.println(accelStatus, HEX);
	//delay(1500);

	if((accelStatus & (STATUS_ZYX_OVERRUN_MASK | STATUS_Y_OVERRUN_MASK | STATUS_X_OVERRUN_MASK)) != 0){
		accelOverruns++;
	}

	//Check if all axis have been updated
	if((accelStatus & STATUS_ZYX_DATA_AVAILABLE_MASK) != 0){
		accel[1] = (( (short)readRegister(OUT_Y_HIGH_ACCEL) << 8) | readRegister(OUT_Y_LOW_ACCEL));// + accel[0]) >> 2;
		accel[2] = (( (short)readRegister(OUT_X_HIGH_ACCEL) << 8) | readRegister(OUT_X_LOW_ACCEL));// + accel[0]) >> 2;
		accel[0] = (( (short)readRegister(OUT_Z_HIGH_ACCEL) << 8) | readRegister(OUT_Z_LOW_ACCEL));// + accel[0]) >> 2;
		accelUpdated = true;
		accelReads++;

		if((systemStatus.requestFlags&PktAccValues)!= 0){
			sendAccelValues();
		}
	}
	else if((accelStatus & STATUS_Z_DATA_AVAILABLE_MASK) != 0){
		accel[0] = (( (short)readRegister(OUT_Z_HIGH_ACCEL) << 8) | readRegister(OUT_Z_LOW_ACCEL));// + accel[0]) >> 2;
		accel[1] = (( (short)readRegister(OUT_Y_HIGH_ACCEL) << 8) | readRegister(OUT_Y_LOW_ACCEL));// + accel[0]) >> 2;
		accelUpdated = true;
		accelReads++;

		if((systemStatus.requestFlags&PktAccValues)!= 0){
			sendAccelValues();
		}
	}
	else if((accelStatus & STATUS_Y_DATA_AVAILABLE_MASK) != 0){
		accel[1] = (( (short)readRegister(OUT_Y_HIGH_ACCEL) << 8) | readRegister(OUT_Y_LOW_ACCEL));// + accel[0]) >> 2;
		accelUpdated = true;
		accelReads++;

		if((systemStatus.requestFlags&PktAccValues)!= 0){
			sendAccelValues();
		}
	}
	else if((accelStatus & STATUS_X_DATA_AVAILABLE_MASK) != 0){
		accel[2] = (( (short)readRegister(OUT_X_HIGH_ACCEL) << 8) | readRegister(OUT_X_LOW_ACCEL));// + accel[0]) >> 2;
		accelUpdated = true;
		accelReads++;

		if((systemStatus.requestFlags&PktAccValues)!= 0){
			sendAccelValues();
		}
	}



	if(accelUpdated){
		unsigned int x = abs(accel[2]);
		unsigned int y = abs(accel[1]);
		unsigned int sum = x + y + abs(accel[0]);

		systemStatus.i2cStatus = I2C_Run;

		if(sum < 22000/* && abs(x-y) > 5000*/){
			bool sendUpdate = false;
			if(x > y){
				if(accel[2] > 0){
					if(systemStatus.orientation != DirectionRight){
						//Serial.print("right\t");
						//Serial.println(sum, DEC);
						sendUpdate = true;
					}
					systemStatus.orientation = DirectionRight;
				}
				else{
					if(systemStatus.orientation != DirectionLeft){
						//Serial.print("left\t");
						//Serial.println(sum, DEC);
						sendUpdate = true;
					}
					systemStatus.orientation = DirectionLeft;
				}
			}
			else{
				if(accel[1] > 0){
					if(systemStatus.orientation != DirectionNormal){
						//Serial.print("normal\t");
						//Serial.println(sum, DEC);
						sendUpdate = true;
					}
					systemStatus.orientation = DirectionNormal;
				}
				else{
					if(systemStatus.orientation != DirectionInverted){
						//Serial.print("inverted\t");
						//Serial.println(sum, DEC);
						sendUpdate = true;
					}
					systemStatus.orientation = DirectionInverted;
				}
			}

			if((systemStatus.requestFlags&PktOrientation)!= 0 && sendUpdate){
				sendOrientationPacket();
				previousTimeMs = currentTimeMs;
			}
		}


		accelUpdated = false;
	}


	if((systemStatus.requestFlags & PktDeviceStatus) != 0 && deltaMs > COMM_SEND_WATCHDOG_MS){
		sendStatusPacket();
		previousTimeMs = currentTimeMs;
	}

	readSerialCommand();
}



