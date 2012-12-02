/*
 * Date: Sat 01/Dez/2012
 * PUC - ENGENHARIA DE COMPUTACAO
 * Autor: Vinicius Grubel Kleinubing
 * e-mail: vgkleinubing@gmail.com
 */ 

struct TIME{
	int h;
	int m;
	int s;
};

struct COORDINATE{
	int g;
	int m;
	int s;
	char loc[1];
};

struct DATE{
	int d;
	int m;
	int y;
};

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

struct GPRMC_PACKET{
	struct TIME time;
	int ack;
	struct COORDINATE lat, lon;
	float speed;
	float angle;
	struct DATE date;
}GPRMC;

/* http://www8.garmin.com/support/text_out.html
 * 
 *     FIELD DESCRIPTION:      WIDTH:  NOTES:
 *     ----------------------- ------- ------------------------
 *     Sentence start          1       Always '@'
 *     ----------------------- ------- ------------------------
 *    /Year                    2       Last two digits of UTC year
 *   | ----------------------- ------- ------------------------
 *   | Month                   2       UTC month, "01".."12"
 * T | ----------------------- ------- ------------------------
 * i | Day                     2       UTC day of month, "01".."31"
 * m | ----------------------- ------- ------------------------
 * e | Hour                    2       UTC hour, "00".."23"
 *   | ----------------------- ------- ------------------------
 *   | Minute                  2       UTC minute, "00".."59"
 *   | ----------------------- ------- ------------------------
 *    \Second                  2       UTC second, "00".."59"
 *     ----------------------- ------- ------------------------
 *    /Latitude hemisphere     1       'N' or 'S'
 *   | ----------------------- ------- ------------------------
 *   | Latitude position       7       WGS84 ddmmmmm, with an implied
 *   |                                 decimal after the 4th digit
 *   | ----------------------- ------- ------------------------
 *   | Longitude hemishpere    1       'E' or 'W'
 *   | ----------------------- ------- ------------------------
 *   | Longitude position      8       WGS84 dddmmmmm with an implied
 * P |                                 decimal after the 5th digit
 * o | ----------------------- ------- ------------------------
 * s | Position status         1       'd' if current 2D differential GPS position
 * i |                                 'D' if current 3D differential GPS position
 * t |                                 'g' if current 2D GPS position
 * i |                                 'G' if current 3D GPS position
 * o |                                 'S' if simulated position
 * n |                                 '_' if invalid position
 *   | ----------------------- ------- ------------------------
 *   | Horizontal posn error   3       EPH in meters
 *   | ----------------------- ------- ------------------------
 *   | Altitude sign           1       '+' or '-'
 *   | ----------------------- ------- ------------------------
 *   | Altitude                5       Height above or below mean
 *    \                                sea level in meters
 *     ----------------------- ------- ------------------------
 *    /East/West velocity      1       'E' or 'W'
 *   |     direction
 *   | ----------------------- ------- ------------------------
 *   | East/West velocity      4       Meters per second in tenths,
 *   |     magnitude                   ("1234" = 123.4 m/s)
 * V | ----------------------- ------- ------------------------
 * e | North/South velocity    1       'N' or 'S'
 * l |     direction
 * o | ----------------------- ------- ------------------------
 * c | North/South velocity    4       Meters per second in tenths,
 * i |     magnitude                   ("1234" = 123.4 m/s)
 * t | ----------------------- ------- ------------------------
 * y | Vertical velocity       1       'U' (up) or 'D' (down)
 *   |     direction
 *   | ----------------------- ------- ------------------------
 *   | Vertical velocity       4       Meters per second in hundredths,
 *    \    magnitude                   ("1234" = 12.34 m/s)
 *     ----------------------- ------- ------------------------
 *     Sentence end            2       Carriage return, '0x0D', and
 *                                     line feed, '0x0A'
 *     ----------------------- ------- ------------------------
 */
struct GPS_GARMIN_TEXT {
	int year;
	int month;
	int day;
	int hour;
	int min;
	int sec;
	int lat_d;
	int lat_mm;
	int lat_mmm;
	int lon_d;
	int lon_mm;
	int lon_mmm;
}GPS_TEXT;

