#include <stdio.h>
#include <math.h>
#include <time.h>
	
#define LAT				// Decimal Degress
#define LONG  ((double) -88.3)		// Decimal Degrees, Wisconsin, USA, e.g.
#define DEBUG

struct celestial {
	double ra;
	double dec; 
};

struct sky {
	double alt;
	double az;
};


double gtToJulian(int month, double day, int year) {		// Calculate Julian Date (decimal) from GT
	double a,b,c,e;

	if(month<=2) {
			year--;
			month = month+12;
		}

	a = (int)(year/100);			// TODO need to change this to round() ?
	b = 2 - a + round((a/4));		// Only works for dates after 1582, call Jim.
	c = round((365.25*year));
	e = round ((30.6001*(month+1)));

	return(b+c+e+1720994.5+day);

}

double utToLST(double julianDate, double ut) {
	double t, t0,t1;

	t = (julianDate-2451545.0)/36525.0;
	t0 = 6.697374558+ (2400.051336*t)+(0.000025862*t*t)+(ut*1.0027379093);
	t1 = fmod(fmod(t0,24)+LONG/15,24);
	if (t1<0) { t1=t1+24; }
	return(t1);

}


int RADecToAltAz(double raDecimal, double dec, double lst) {

	// RA and Dec need to be decimal

	#ifdef DEBUG
	printf("RA: %f, Dec: %f\n, LST: %f\n",raDecimal, dec, lst);
	#endif

	return(0);
}




int main() {

	double	gmt, ut, lst, ra, dec;
	double  julianDate;
	struct celestial currentRADec;
	struct sky currentAltAz;
    	int day;
    	int month;
    	int year;
    	double dayFraction;

    	time_t timer;
	struct tm* tm_info;

	time(&timer);

    	tm_info = gmtime(&timer);
    	day = tm_info->tm_mday;
    	month = tm_info->tm_mon+1;
    	year = tm_info->tm_year+1900;

	#ifdef DEBUG
    	printf("GMT date -> %i/%i/%i\n", month, day, year);
    	#endif


    ut = (double)tm_info->tm_hour + (double)tm_info->tm_min/60 +(double)tm_info->tm_sec/(60*60);		// TO DO: move to function
    julianDate = gtToJulian(month, (double) day, year );

    #ifdef DEBUG
    printf("UTC decimal -> %f\n",ut);
    #endif

    lst = utToLST(julianDate, ut);

    #ifdef DEBUG
    printf("LST decimal -> %f\n",lst);
    #endif

    RADecToAltAz(ra, dec, lst);

	return(0);

}
