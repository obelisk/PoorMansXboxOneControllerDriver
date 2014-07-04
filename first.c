#include <CoreFoundation/CoreFoundation.h>
#include <Carbon/Carbon.h>
#include <ApplicationServices/ApplicationServices.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/usb/USB.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/USBSpec.h>
#include <unistd.h>
const int active_conf[26] = {
	kVK_ANSI_S, //A
	kVK_ANSI_D, //B
	kVK_ANSI_A, //X
	kVK_ANSI_W, //Y
	kVK_ANSI_Q,	//LT
	kVK_ANSI_E,	//RT
	kVK_ANSI_Z, //LB
	kVK_ANSI_C,	//RB
	-1,	//LEFT
	-1,	//UP
	-1,	//RIGHT
	-1,	//DOWN
	-1,	//LS_CLICK
	-1,	//RS_CLICK
	-1,	//VIEW
	kVK_Escape,	//MENU
	-1,	//SYNC
	kVK_Return,		//Guide
	kVK_LeftArrow,	//Left Stick
	kVK_UpArrow,
	kVK_RightArrow,
	kVK_DownArrow,
	-1,	//Right Stick
	-1,
	-1,
	-1
	};
const int vendorID = 0x045e;
const int xoneControllerID = 0x02d1;
const int xControllerReadInterface = 2;

//Tolerances
const int LT_TOLERANCE = 0;
const int RT_TOLERANCE = 0;
const int LSX_TOLERANCE = 8000;
const int LSY_TOLERANCE = 8000;
const int RSX_TOLERANCE = 8000;
const int RSY_TOLERANCE = 8000;

CGEventRef downEvt;
CGEventRef upEvt;
bool bStat[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
enum BUTTONS {A, B, X, Y, LT, RT, LB, RB, LEFT, UP, RIGHT, DOWN, LS_CLICK, RS_CLICK, VIEW, MENU, SYNC, GUIDE, LS_LEFT, LS_UP, LS_RIGHT, LS_DOWN, RS_LEFT, RS_UP, RS_RIGHT, RS_DOWN};
unsigned short lt = 0;
unsigned short rt = 0;
bool lt_press = 0;
bool rt_press = 0;
bool lsx_move = 0;
bool lsy_move = 0;
bool rsx_move = 0;
bool rsy_move = 0;

short lsx = 0;
short lsy = 0;

short rsx = 0;
short rsy = 0;

void print_mem(void const *vp, size_t n){
    unsigned char const *p = vp;
    for (size_t i=0; i<n; i++)
        printf("%02x ", p[i]);
    putchar('\n');
};

void simulateKey(int button, int pressed){
	if(button == -1){return;}
	if(pressed){
		CGEventRef keydown = CGEventCreateKeyboardEvent(kCGHIDEventTap, active_conf[button], true);
		CGEventPost(kCGHIDEventTap, keydown);
	}else{
		CGEventRef keyup = CGEventCreateKeyboardEvent(kCGHIDEventTap, active_conf[button], false);
		CGEventPost(kCGHIDEventTap, keyup);
	}
	return;
};

void read_controller(struct IOUSBInterfaceStruct300 ** xInterfaceControl, char* read_buffer, unsigned int read_buffer_size){
	if( kIOReturnSuccess != (*xInterfaceControl)->ReadPipe(xInterfaceControl,2,read_buffer,&read_buffer_size)){
		printf("Error\n");
		exit(1);
	}
};

void parse_command_and_echo(char* read_buffer, unsigned int read_buffer_size){
	switch(*read_buffer){
		case 0x20:
			//Sync Button
			if(0x01 & *(read_buffer+4) && bStat[15] == 0){
				printf("Sync Pressed\n");
				bStat[15] = 1;
			}else if(!(0x01 & *(read_buffer+4)) && bStat[15]){
				printf("Sync Released.\n");
				bStat[15] = 0;
			}
			//A Button
			if(0x10 & *(read_buffer+4) && bStat[0] == 0){
				printf("A Pressed\n");
				bStat[0] = 1;
			}else if(!(0x10 & *(read_buffer+4)) && bStat[0]){
				printf("A Released.\n");
				bStat[0] = 0;
			}
			//B Button
			if(0x20 & *(read_buffer+4) && bStat[1] == 0){
				printf("B Pressed\n");
				bStat[1] = 1;
			}else if(!(0x20 & *(read_buffer+4)) && bStat[1]){
				printf("B Released.\n");
				bStat[1] = 0;
			}
			//X Button
			if(0x40 & *(read_buffer+4) && bStat[2] == 0){
				printf("X Pressed\n");
				bStat[2] = 1;
			}else if(!(0x40 & *(read_buffer+4)) && bStat[2]){
				printf("X Released.\n");
				bStat[2] = 0;
			}
			//Y Button
			if(0x80 & *(read_buffer+4) && bStat[3] == 0){
				printf("Y Pressed\n");
				bStat[3] = 1;
			}else if(!(0x80 & *(read_buffer+4)) && bStat[3]){
				printf("Y Released.\n");
				bStat[3] = 0;
			}
			//Menu Button
			if(0x04 & *(read_buffer+4) && bStat[4] == 0){
				printf("Menu Pressed\n");
				bStat[4] = 1;
			}else if(!(0x04 & *(read_buffer+4)) && bStat[4]){
				printf("Menu Released.\n");
				bStat[4] = 0;
			}
			//View Button
			if(0x08 & *(read_buffer+4) && bStat[5] == 0){
				printf("View Pressed\n");
				bStat[5] = 1;
			}else if(!(0x08 & *(read_buffer+4)) && bStat[5]){
				printf("View Released.\n");
				bStat[5] = 0;
			}
			/**************************END BUTTONS, START DPAD*/
			//Up Button
			if(0x01 & *(read_buffer+5) && bStat[6] == 0){
				printf("Up Pressed\n");
				bStat[6] = 1;
			}else if(!(0x01 & *(read_buffer+5)) && bStat[6]){
				printf("Up Released.\n");
				bStat[6] = 0;
			}
			//Down Button
			if(0x02 & *(read_buffer+5) && bStat[7] == 0){
				printf("Down Pressed\n");
				bStat[7] = 1;
			}else if(!(0x02 & *(read_buffer+5)) && bStat[7]){
				printf("Down Released.\n");
				bStat[7] = 0;
			}
			//Left Button
			if(0x04 & *(read_buffer+5) && bStat[8] == 0){
				printf("Left Pressed\n");
				bStat[8] = 1;
			}else if(!(0x04 & *(read_buffer+5)) && bStat[8]){
				printf("Left Released.\n");
				bStat[8] = 0;
			}
			//Right Button
			if(0x08 & *(read_buffer+5) && bStat[9] == 0){
				printf("Right Pressed\n");
				bStat[9] = 1;
			}else if(!(0x08 & *(read_buffer+5)) && bStat[9]){
				printf("Right Released.\n");
				bStat[9] = 0;
			}
			/**********END DPAD CODE, START BUMPER AND STICK CLICK**********/
			if(0x10 & *(read_buffer+5) && bStat[10] == 0){
				printf("LB Pressed\n");
				bStat[10] = 1;
			}else if(!(0x10 & *(read_buffer+5)) && bStat[10]){
				printf("LB Released.\n");
				bStat[10] = 0;
			}
			if(0x20 & *(read_buffer+5) && bStat[11] == 0){
				printf("RB Pressed\n");
				bStat[11] = 1;
			}else if(!(0x20 & *(read_buffer+5)) && bStat[11]){
				printf("RB Released.\n");
				bStat[11] = 0;
			}
			if(0x40 & *(read_buffer+5) && bStat[12] == 0){
				printf("Left Stick Pressed\n");
				bStat[12] = 1;
			}else if(!(0x40 & *(read_buffer+5)) && bStat[12]){
				printf("Left Stick Released.\n");
				bStat[12] = 0;
			}
			if(0x80 & *(read_buffer+5) && bStat[13] == 0){
				printf("Right Stick Pressed\n");
				bStat[13] = 1;
			}else if(!(0x80 & *(read_buffer+5)) && bStat[13]){
				printf("Right Stick Released.\n");
				bStat[13] = 0;
			}
			/*END OF ALL BUTTON CODE, TRIGGERS AND JOYSTICKS NOW*/
			//Left Trigger
			lt = (unsigned char)((*(read_buffer+6)))+((unsigned char)(*(read_buffer+7))<<8);

			//Right Trigger
			rt = (unsigned char)((*(read_buffer+8)))+((unsigned char)(*(read_buffer+9))<<8);

			//Left Stick X
			lsx = (unsigned char)((*(read_buffer+10)))+((unsigned char)(*(read_buffer+11))<<8);

			//Left Stick Y
			lsy = (unsigned char)((*(read_buffer+12)))+((unsigned char)(*(read_buffer+13))<<8);

			//Right Stick X
			rsx = (unsigned char)((*(read_buffer+14)))+((unsigned char)(*(read_buffer+15))<<8);

			//Right Stick Y
			rsy = (unsigned char)((*(read_buffer+16)))+((unsigned char)(*(read_buffer+17))<<8);

			if(lt != 0x00){
				printf("LT: %d\n", lt);
			}
			if(rt != 0x00){
				printf("RT: %d\n", rt);
			}

			break;
		case 0x07:
			//Guide Button
			if(0x01 & *(read_buffer+4) && bStat[14] == 0){
				printf("Guide Pressed\n");
				bStat[14] = 1;
			}else if(!(0x01 & *(read_buffer+4)) && bStat[14]){
				printf("Guide Released.\n");
				bStat[14] = 0;
			}
			break;
		case 0x03:
			break;
		default:
			break;
	}
};

void parse_command_and_simulate(char* read_buffer, unsigned int read_buffer_size){
	switch(*read_buffer){
		case 0x20:
			//Sync Button
			if(0x01 & *(read_buffer+4) && bStat[15] == 0){
				bStat[15] = 1;
			}else if(!(0x01 & *(read_buffer+4)) && bStat[15]){
				bStat[15] = 0;
			}
			//A Button
			if(0x10 & *(read_buffer+4) && bStat[0] == 0){
				bStat[0] = 1;
				simulateKey(A, 1);
			}else if(!(0x10 & *(read_buffer+4)) && bStat[0]){
				bStat[0] = 0;
				simulateKey(A, 0);
			}
			//B Button
			if(0x20 & *(read_buffer+4) && bStat[1] == 0){
				bStat[1] = 1;
				simulateKey(B, 1);
			}else if(!(0x20 & *(read_buffer+4)) && bStat[1]){
				bStat[1] = 0;
				simulateKey(B, 0);
			}
			//X Button
			if(0x40 & *(read_buffer+4) && bStat[2] == 0){
				bStat[2] = 1;
				simulateKey(X, 1);
			}else if(!(0x40 & *(read_buffer+4)) && bStat[2]){
				bStat[2] = 0;
				simulateKey(X, 0);
			}
			//Y Button
			if(0x80 & *(read_buffer+4) && bStat[3] == 0){
				bStat[3] = 1;
				simulateKey(Y, 1);
			}else if(!(0x80 & *(read_buffer+4)) && bStat[3]){
				bStat[3] = 0;
				simulateKey(Y, 0);
			}
			//Menu Button
			if(0x04 & *(read_buffer+4) && bStat[4] == 0){
				bStat[4] = 1;
				simulateKey(MENU, 1);
			}else if(!(0x04 & *(read_buffer+4)) && bStat[4]){
				bStat[4] = 0;
				simulateKey(MENU, 0);
			}
			//View Button
			if(0x08 & *(read_buffer+4) && bStat[5] == 0){
				bStat[5] = 1;
				simulateKey(VIEW, 1);
			}else if(!(0x08 & *(read_buffer+4)) && bStat[5]){
				bStat[5] = 0;
				simulateKey(VIEW, 0);
			}
			/**************************END BUTTONS, START DPAD*/
			//Up Button
			if(0x01 & *(read_buffer+5) && bStat[6] == 0){
				bStat[6] = 1;
				simulateKey(UP, 1);
			}else if(!(0x01 & *(read_buffer+5)) && bStat[6]){
				bStat[6] = 0;
				simulateKey(UP, 0);
			}
			//Down Button
			if(0x02 & *(read_buffer+5) && bStat[7] == 0){
				bStat[7] = 1;
				simulateKey(DOWN, 1);
			}else if(!(0x02 & *(read_buffer+5)) && bStat[7]){
				bStat[7] = 0;
				simulateKey(DOWN, 0);
			}
			//Left Button
			if(0x04 & *(read_buffer+5) && bStat[8] == 0){
				bStat[8] = 1;
				simulateKey(LEFT, 1);
			}else if(!(0x04 & *(read_buffer+5)) && bStat[8]){
				bStat[8] = 0;
				simulateKey(LEFT, 0);
			}
			//Right Button
			if(0x08 & *(read_buffer+5) && bStat[9] == 0){
				bStat[9] = 1;
				simulateKey(RIGHT, 1);
			}else if(!(0x08 & *(read_buffer+5)) && bStat[9]){
				bStat[9] = 0;
				simulateKey(RIGHT, 0);
			}
			/**********END DPAD CODE, START BUMPER AND STICK CLICK**********/
			//LB
			if(0x10 & *(read_buffer+5) && bStat[10] == 0){
				bStat[10] = 1;
				simulateKey(LB, 1);
			}else if(!(0x10 & *(read_buffer+5)) && bStat[10]){
				bStat[10] = 0;
				simulateKey(LB, 0);
			}
			//RB
			if(0x20 & *(read_buffer+5) && bStat[11] == 0){
				bStat[11] = 1;
				simulateKey(RB, 1);
			}else if(!(0x20 & *(read_buffer+5)) && bStat[11]){
				bStat[11] = 0;
				simulateKey(RB, 0);
			}
			//LS CLICK
			if(0x40 & *(read_buffer+5) && bStat[12] == 0){
				bStat[12] = 1;
				simulateKey(LS_CLICK, 1);
			}else if(!(0x40 & *(read_buffer+5)) && bStat[12]){
				bStat[12] = 0;
				simulateKey(LS_CLICK, 0);
			}
			//RS CLICK
			if(0x80 & *(read_buffer+5) && bStat[13] == 0){
				bStat[13] = 1;
				simulateKey(RS_CLICK, 1);
			}else if(!(0x80 & *(read_buffer+5)) && bStat[13]){
				bStat[13] = 0;
				simulateKey(RS_CLICK, 0);
			}
			/*END OF ALL BUTTON CODE, TRIGGERS AND JOYSTICKS NOW*/
			//Left Trigger
			lt = (unsigned char)((*(read_buffer+6)))+((unsigned char)(*(read_buffer+7))<<8);
			if(lt > LT_TOLERANCE){
				if(lt_press == 0){
					lt_press = 1;
					simulateKey(LT, 1);
				}
			}else{
				lt_press = 0;
				simulateKey(LT, 0);
			}

			//Right Trigger
			rt = (unsigned char)((*(read_buffer+8)))+((unsigned char)(*(read_buffer+9))<<8);
			if(rt > RT_TOLERANCE){
				if(rt_press == 0){
					rt_press = 1;
					simulateKey(RT, 1);
				}
			}else{
				rt_press = 0;
				simulateKey(RT, 0);
			}
			
			//Left Stick X
			lsx = (unsigned char)((*(read_buffer+10)))+((unsigned char)(*(read_buffer+11))<<8);
			if(lsx > LSX_TOLERANCE){
				if(lsx_move == 0){
					lsx_move = 1;
					simulateKey(LS_RIGHT, 1);
				}
			}else{
				lsx_move = 0;
				simulateKey(LS_RIGHT, 0);
			}
			if(lsx < -LSX_TOLERANCE){
				if(lsx_move == 0){
					lsx_move = 1;
					simulateKey(LS_LEFT, 1);
				}
			}else{
				lsx_move = 0;
				simulateKey(LS_LEFT, 0);
			}

			//Left Stick Y
			lsy = (unsigned char)((*(read_buffer+12)))+((unsigned char)(*(read_buffer+13))<<8);
			if(lsy > LSY_TOLERANCE){
				if(lsy_move == 0){
					lsy_move = 1;
					simulateKey(LS_UP, 1);
				}
			}else{
				lsy_move = 0;
				simulateKey(LS_UP, 0);
			}
			if(lsy < -LSY_TOLERANCE){
				if(lsy_move == 0){
					lsy_move = 1;
					simulateKey(LS_DOWN, 1);
				}
			}else{
				lsx_move = 0;
				simulateKey(LS_DOWN, 0);
			}
			
			//Right Stick X
			rsx = (unsigned char)((*(read_buffer+14)))+((unsigned char)(*(read_buffer+15))<<8);
			if(rsx > RSX_TOLERANCE){
				if(rsx_move == 0){
					rsx_move = 1;
					simulateKey(RS_RIGHT, 1);
				}
			}else{
				rsx_move = 0;
				simulateKey(RS_RIGHT, 0);
			}
			if(rsx < -RSX_TOLERANCE){
				if(rsx_move == 0){
					rsx_move = 1;
					simulateKey(RS_LEFT, 1);
				}
			}else{
				rsx_move = 0;
				simulateKey(RS_LEFT, 0);
			}

			//Right Stick Y
			rsy = (unsigned char)((*(read_buffer+16)))+((unsigned char)(*(read_buffer+17))<<8);
			if(rsy > RSY_TOLERANCE){
				if(rsy_move == 0){
					rsy_move = 1;
					simulateKey(RS_UP, 1);
				}
			}else{
				rsy_move = 0;
				simulateKey(RS_UP, 0);
			}
			if(rsy < -RSY_TOLERANCE){
				if(rsy_move == 0){
					rsy_move = 1;
					simulateKey(RS_DOWN, 1);
				}
			}else{
				rsy_move = 0;
				simulateKey(RS_DOWN, 0);
			}/**/

			break;
		case 0x07:
			//Guide Button
			if(0x01 & *(read_buffer+4) && bStat[14] == 0){
				bStat[14] = 1;
				simulateKey(GUIDE, 1);
			}else if(!(0x01 & *(read_buffer+4)) && bStat[14]){
				bStat[14] = 0;
				simulateKey(GUIDE, 0);
			}
			break;
		case 0x03:
			break;
		default:
			break;
	}
}

int main (){
	//Class based types
	IOCFPlugInInterface** plugin;
	CFMutableDictionaryRef matchingDict;
	IOUSBDeviceInterface300** xController;
	IOUSBInterfaceInterface300** xInterfaceControl;
	IOReturn ret;
	IOUSBConfigurationDescriptorPtr config;

	//Set up Key up and down states
	downEvt = CGEventCreateKeyboardEvent( NULL, 0, true );
	upEvt = CGEventCreateKeyboardEvent( NULL, 0, false );

	//Interface request and filter
	IOUSBFindInterfaceRequest request;
	request.bInterfaceClass = 255;
	request.bInterfaceProtocol = 208;
	request.bInterfaceSubClass = 71;
	request.bAlternateSetting = kIOUSBFindInterfaceDontCare;

	//Prim
	SInt32 score;
	unsigned int read_buffer_size = 0;
	char * read_buffer;

	//Structs
    io_iterator_t iter;
    kern_return_t kr;
    io_service_t device;
    io_service_t controlInterface;

    //Create a matching dictionary and look for the microsoft vendor and the xbox one product id
    matchingDict = IOServiceMatching(kIOUSBDeviceClassName);
    if (matchingDict == NULL){return -1;}
    CFDictionaryAddValue(matchingDict, CFSTR(kUSBVendorID), CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &vendorID));
	CFDictionaryAddValue(matchingDict, CFSTR(kUSBProductID), CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &xoneControllerID));
    kr = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict, &iter);
    if (kr != KERN_SUCCESS){return -1;}

    //Look for Xbox One controllers
    device = IOIteratorNext(iter);
    if(device == 0){
    	printf("No Xbox One controllers were found! Please make sure it's attached via USB\n");
    	return 1;
    }
    if(0 != IOIteratorNext(iter)){
    	printf("More than one controller was found. I'm just going to use the first one.\n");
    }
    printf("Opening Connection to Controller #1\n");

   	IOCreatePlugInInterfaceForService(device, kIOUSBDeviceUserClientTypeID, kIOCFPlugInInterfaceID, &plugin, &score);
   	(*plugin)->QueryInterface(plugin, CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID300), (LPVOID)&xController);
   	(*plugin)->Release(plugin);

   	ret = (*xController)->USBDeviceOpen(xController);
   	if(ret != KERN_SUCCESS){
		printf("Could not connect to controller.\n");
   		return 1;
   	}

   	printf("Successfully connected to Xbox One Controller.\n");

   	ret = (*xController)->GetConfigurationDescriptorPtr(xController, 0, &config);
    if (ret != KERN_SUCCESS){
        printf("Could not set controller active configuration: %d\n", ret);
        return 1;
    }

    kr = (*xController)->SetConfiguration(xController, config->bConfigurationValue);
    if(kr != KERN_SUCCESS){
    	printf("Could not configure controller.\n");
    	return 1;
    }
    printf("Opened connection to controller.\n");
    printf("Getting Interfaces...\n");

	kr = (*xController)->CreateInterfaceIterator(xController, &request, &iter);

	if(kr != KERN_SUCCESS){
		printf("Could not find interfaces.\n");
		return 1;
	}

	controlInterface = IOIteratorNext(iter);
	if (!controlInterface){
		printf("Could not get controller interface.\n");
		return 1;
	}

	printf("Got controller interface, trying to open it...\n");

	IOCFPlugInInterface **plugin_interface;
	kr = IOCreatePlugInInterfaceForService(	controlInterface,
											kIOUSBInterfaceUserClientTypeID,
											kIOCFPlugInInterfaceID,
											&plugin_interface,
											&score);
	if (kr != KERN_SUCCESS || !plugin_interface){
		printf("Could not create the interface...\n");
		return 1;
	}

	do {
	    IOObjectRelease(controlInterface);
	} while ((controlInterface = IOIteratorNext(iter)));

	HRESULT res = (*plugin_interface)->QueryInterface(
	    	plugin_interface,
	    	CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID300),
	    	(LPVOID *)&xInterfaceControl);

	if (!SUCCEEDED(res) || !xInterfaceControl){
		printf("Could not open the interface\n");
		return 1;
	}else{
		printf("Opened the interface.\n");
	}

	ret = (*xInterfaceControl)->USBInterfaceOpen(xInterfaceControl);
	if (ret != kIOReturnSuccess){
	    printf("Could not open the controller interface. Error: %d\n", ret);
	    return 1;
	}

	CFRunLoopSourceRef source_ref;
	kr = (*xInterfaceControl)->CreateInterfaceAsyncEventSource(xInterfaceControl, &source_ref);

	unsigned char eps;
	kr = (*xInterfaceControl)->GetNumEndpoints(xInterfaceControl, &eps);
	if (kr != KERN_SUCCESS || eps < 2){
		printf("Wrong number of pipes were returned: %d\n", eps);
		return 1;
	}

	for(int i=1; i<= 2; i++){
		unsigned char direction = 0x0;
		unsigned char number = 0x0;
		unsigned char transfer_type = 0x0;
		unsigned short max_packet_size = 0x0;
		unsigned char interval = 0x0;

		kr = (*xInterfaceControl)->GetPipeProperties(xInterfaceControl,i,&direction,&number,&transfer_type,&max_packet_size,&interval);
		if (kr == kIOReturnSuccess) {
			printf("Successfully opened pipe!\n");
			read_buffer_size = max_packet_size;
			read_buffer = malloc(read_buffer_size);

	    }else if(kr == kIOReturnExclusiveAccess){
	    	printf("Could not get exclusive access.\n");
	    }else if(kr == kIOReturnNotOpen){
	    	printf("Interface not open.\n");
	    }
	}
	printf("Max Packet Size: %d\n", read_buffer_size);
	unsigned char *buf = malloc(sizeof(unsigned int) * 2);
	buf[0] = 0x05;
	buf[1] = 0x20;
	ret = (*xInterfaceControl)->WritePipe(xInterfaceControl, 1, buf, 2);
	free(buf);
	sleep(2);	
	/*******************************************************************************************
				Setup complete, now we need to reliably get data off the controller
	*******************************************************************************************/
	printf("Controller Started!\n");
	while(1){
		read_controller(xInterfaceControl, read_buffer, read_buffer_size);
		parse_command_and_simulate(read_buffer, read_buffer_size);
	}

	(*xInterfaceControl)->USBInterfaceClose(xInterfaceControl);
	(*xController)->USBDeviceClose(xController);
	free(read_buffer);
    return 0;
}