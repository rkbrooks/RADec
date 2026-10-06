#ifndef RADEC_H
#define RADEC_H

struct celestial {
    double ra;  /* Decimal hours, [0, 24). */
    double dec; /* Decimal degrees, [-90, 90]. */
};

struct sky {
    double alt; /* Degrees above the geometric horizon. */
    double az;  /* Degrees east of north, [0, 360); NAN at zenith/nadir. */
};

/* Proleptic Gregorian calendar, years 1..9999; day may include a fraction.
 * Invalid dates return NAN. */
double gtToJulian(int month, double day, int year);

/* julianDate is the Julian date at UTC midnight; ut is hours in [0, 24).
 * Longitude is degrees east, [-180, 180]. Returns local MEAN sidereal
 * time in hours, using UTC as an approximation to UT1; invalid input -> NAN. */
double utToLST(double julianDate, double ut, double longitude);

/* raDecimal and lst are hours in [0, 24); dec and latitude are degrees.
 * Returns 0 on success, -1 on invalid input (without changing *result). */
int RADecToAltAz(double raDecimal, double dec, double lst,
                 double latitude, struct sky *result);

#endif
