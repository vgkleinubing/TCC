/*
 * Date: Sat 01/Dez/2012
 * PUC - ENGENHARIA DE COMPUTACAO
 * Autor: Vinicius Grubel Kleinubing
 * e-mail: vgkleinubing@gmail.com
 */

#include <arch/nxp/lpc23xx.h>
#include <string.h>
#include <time.h>
#include "gps_decode.h"
#include "uart.h"
#include "gps_structs.h"
#include "lcd.h"
#include "mprintf.h"
#include "monitor.h"

#include "defines.h"
static char temp[30][50];

//$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
//	RMC			Recommended Minimum sentence C
//	123519		Fix taken at 12:35:19 UTC
//	A			Status A=active or V=Void.
//	4807.038,N	Latitude 48 deg 07.038' N
//	01131.000,E	Longitude 11 deg 31.000' E
//	022.4		Speed over the ground in knots
//	084.4		Track angle in degrees True
//	230394		Date - 23rd of March 1994
//	003.1,W		Magnetic Variation
//	*6A			The checksum data, always begins with *

static void GPRMC_parser(char *packet){
			int i = 0;
			char *pointer;
			memset(temp, 0, sizeof(temp));

			pointer = strtok (packet, ",.*");
	  		while (pointer != NULL)
	 		 {
				strcpy (temp[i], pointer);
	 		    pointer = strtok (NULL, ",.*");
				i++;
	  		}

			/* Store data */
			GPRMC.ack = 1;

			/* Time convert */
			int aux;
			aux = atoi(temp[1]);
			GPRMC.time.h = aux/10000;
			GPRMC.time.m = ((aux-(GPRMC.time.h*10000))/100);
			GPRMC.time.s = aux-((GPRMC.time.h*10000)+(GPRMC.time.m*100));
			GPS_TEXT.hour = GPRMC.time.h;
			GPS_TEXT.min = GPRMC.time.m;
			GPS_TEXT.sec = GPRMC.time.s;

			/* Lat convert */
			aux = atoi(temp[3]);
			GPRMC.lat.g = aux/100;
			GPRMC.lat.m = (aux-(GPRMC.lat.g*100));
			GPRMC.lat.s = ((atoi(temp[4]))*60)/9999;
			GPS_TEXT.lat_d = GPRMC.lat.g;
			GPS_TEXT.lat_mm = GPRMC.lat.m;
			GPS_TEXT.lat_mmm = atoi(temp[4]);
			strcpy(GPRMC.lat.loc, temp[5]);
			if (!strncmp(GPRMC.lat.loc, "S", 1))
				GPS_TEXT.lat_d = 0 - GPS_TEXT.lat_d;

			/* Lon convert */
			aux = atoi(temp[6]);
			GPRMC.lon.g = aux/100;
			GPRMC.lon.m = (aux-(GPRMC.lon.g*100));
			GPRMC.lon.s = ((atoi(temp[7])*60))/9999;
			GPS_TEXT.lon_d = GPRMC.lon.g;
			GPS_TEXT.lon_mm = GPRMC.lon.m ;
			GPS_TEXT.lon_mmm = atoi(temp[7]);
			strcpy(GPRMC.lon.loc, temp[8]);
			if (!strncmp(GPRMC.lon.loc, "W", 1))
				GPS_TEXT.lon_d = 0 - GPS_TEXT.lon_d;
			/* Speed convert */
			GPRMC.speed = ((((atoi(temp[9])*10)+atoi(temp[10]))/10)*0.514444); /* Convert knots to M/s */

			/* Angle */
			GPRMC.angle = (atoi(temp[11])); /* Ignorado parte fracinaria, sempre 0*/

			/* Date */
			aux = atoi(temp[13]);
			GPRMC.date.d = aux/10000;
			GPRMC.date.m = ((aux-(GPRMC.date.d*10000))/100);
			GPRMC.date.y = atoi(temp[13])%100;
			GPS_TEXT.year = GPRMC.date.y;
			GPS_TEXT.month = GPRMC.date.m;
			GPS_TEXT.day = GPRMC.date.d;
			/*Data ready*/
}

static int verify_checksum(char *s) {
    int c = 0;
 
    while(*s && strncmp(s, "*", 1)){
        c ^= *s++;
	}
	s++;
	sprintf(temp[0], "%02x", c);

	if (strncmp(temp[0], s, 2))
 		return -1;
    return 0;
}

static int nmea_parser(char *packet){
	static char identifier[5];
	int ack = -2;
	memset(temp, 0, sizeof(temp));
	memset(identifier, 0, sizeof(identifier));
	/* Parser Packet and store data */
	strncpy (identifier, &packet[1], 5);
	if (!strcmp(identifier, "GPRMC")){ /*id indentifier == GPRMC*/
		if (verify_checksum(&packet[1]) == -1)
			return -1;

		if (packet[14] != 'V') {//se for A pacote valido, se for V invalido, no modo demo fica sempre em V
//			xputs("\n --GPRMC not ACK-- \n");
			GPRMC.ack = 0;
			ack = -1;
			return ack;
		}
		ack = 0;

		GPRMC_parser(packet);
	}
	return ack;
}

// parse_packet - @121113143920S2959898W05108905S015+00088E0000N0056U0000
static int txt_parser(char *packet){
	memset(temp, 0, sizeof(temp));

	xputs("txt_parse_packet - ");
	xputs(packet);
	xputs("\n");

	/* if packet[13] is '_', gps not ready */
	if (packet[13] == '_')
		return -1;

	// year, month, day, hour, min, sec
	strncpy (temp[0], &packet[1], 2);
	strncpy (temp[1], &packet[3], 2);
	strncpy (temp[2], &packet[5], 2);
	strncpy (temp[3], &packet[7], 2);
	strncpy (temp[4], &packet[9], 2);
	strncpy (temp[5], &packet[11], 2);

	GPS_TEXT.year = atoi(temp[0]);
	GPS_TEXT.month = atoi(temp[1]);
	GPS_TEXT.day = atoi(temp[2]);
	GPS_TEXT.hour = atoi(temp[3]);
	GPS_TEXT.min = atoi(temp[4]);
	GPS_TEXT.sec = atoi(temp[5]);

	// latitude
	strncpy (temp[6], &packet[14], 2);
	strncpy (temp[7], &packet[16], 2);
	strncpy (temp[8], &packet[18], 3);

	GPS_TEXT.lat_d = atoi(temp[6]);
	GPS_TEXT.lat_mm = atoi(temp[7]);
	GPS_TEXT.lat_mmm = atoi(temp[8]);

	if (packet[13] == 'S')
		GPS_TEXT.lat_d = 0 - GPS_TEXT.lat_d;

	//longitude
	strncpy (temp[9], &packet[22], 3);
	strncpy (temp[10], &packet[25], 2);
	strncpy (temp[11], &packet[27], 3);

	GPS_TEXT.lon_d = atoi(temp[9]);
	GPS_TEXT.lon_mm = atoi(temp[10]);
	GPS_TEXT.lon_mmm = atoi(temp[11]);

	if (packet[21] == 'W')
		GPS_TEXT.lon_d = 0 - GPS_TEXT.lon_d;

	return 0;
}

int parse_packet(char *packet){
/*	xputs("parse_packet - ");
	xputs(packet);
	xputs("\n");
*/
	/* Parser Packet and store data */
	/* Parse Garmin text packet */
	if (packet[0] == '@')
		return txt_parser(packet);

	/* Parse NMEA packet*/
	if (packet[0] == '$')
		return nmea_parser(packet);
	
	return -1;
}


