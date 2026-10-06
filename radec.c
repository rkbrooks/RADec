#include "radec.h"

#include <float.h>
#include <math.h>
#include <stddef.h>

static const double radians_per_degree = 0.017453292519943295769;

static double wrap(double value, double period)
{
    double result = fmod(value, period);
    if (result < 0.0)
        result += period;
    /* Adding period to a tiny negative remainder can round up to period. */
    return result >= period || result == 0.0 ? 0.0 : result;
}

double gtToJulian(int month, double day, int year)
{
    static const int month_days[] = {31, 28, 31, 30, 31, 30,
                                    31, 31, 30, 31, 30, 31};
    int days, century;
    double correction;

    if (year < 1 || year > 9999 || month < 1 || month > 12 || !isfinite(day))
        return NAN;
    days = month_days[month - 1];
    if (month == 2 && year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))
        ++days;
    if (day < 1.0 || day >= days + 1.0)
        return NAN;

    if (month <= 2) {
        --year;
        month += 12;
    }
    century = year / 100;
    correction = 2 - century + century / 4;
    return floor(365.25 * (year + 4716)) + floor(30.6001 * (month + 1))
           + day + correction - 1524.5;
}

double utToLST(double julianDate, double ut, double longitude)
{
    double days, centuries, gmst;

    if (!isfinite(julianDate) || julianDate < 1721425.5 ||
        julianDate > 5373483.5 ||
        julianDate - floor(julianDate) != 0.5 ||
        !isfinite(ut) || ut < 0.0 || ut >= 24.0 ||
        !isfinite(longitude) || longitude < -180.0 || longitude > 180.0)
        return NAN;

    /* USNO approximate GMST formula, assuming TT = UT1 = UTC.
     * The midnight date and elapsed UTC hours must remain separate. */
    days = julianDate - 2451545.0;
    centuries = (days + ut / 24.0) / 36525.0;
    gmst = 6.697375 + 0.065709824279 * days + 1.0027379 * ut
           + 0.0000258 * centuries * centuries;
    return wrap(gmst + longitude / 15.0, 24.0);
}

int RADecToAltAz(double raDecimal, double dec, double lst,
                 double latitude, struct sky *result)
{
    double hour_angle, declination, phi, east, north, up, horizontal;

    if (result == NULL || !isfinite(raDecimal) || raDecimal < 0.0 ||
        raDecimal >= 24.0 || !isfinite(dec) || dec < -90.0 || dec > 90.0 ||
        !isfinite(lst) || lst < 0.0 || lst >= 24.0 ||
        !isfinite(latitude) || latitude < -90.0 || latitude > 90.0)
        return -1;

    hour_angle = remainder(lst - raDecimal, 24.0) * 15.0 * radians_per_degree;
    declination = dec * radians_per_degree;
    phi = latitude * radians_per_degree;

    /* Rotate the equatorial unit vector into local east, north, up.
     * This avoids tan(dec) at the celestial poles and asin domain drift. */
    east = -cos(declination) * sin(hour_angle);
    north = sin(declination) * cos(phi)
            - cos(declination) * cos(hour_angle) * sin(phi);
    up = sin(declination) * sin(phi)
         + cos(declination) * cos(hour_angle) * cos(phi);
    horizontal = hypot(east, north);
    result->alt = atan2(up, horizontal) / radians_per_degree;
    result->az = horizontal <= 16.0 * DBL_EPSILON
                 ? NAN : wrap(atan2(east, north) / radians_per_degree, 360.0);
    return 0;
}

#ifndef RADEC_NO_MAIN
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int parse_number(const char *text, double *value)
{
    char *end;
    errno = 0;
    *value = strtod(text, &end);
    return end != text && *end == '\0' && errno == 0 && isfinite(*value);
}

static int parse_timestamp(const char *text, time_t *timer)
{
    char *end;
    intmax_t value;
    errno = 0;
    value = strtoimax(text, &end, 10);
    if (end == text || *end != '\0' || errno != 0 ||
        value < INT64_C(-62135596800) || value > INT64_C(253402300799))
        return 0;
    *timer = (time_t)value;
    return (intmax_t)*timer == value;
}

static void usage(const char *program)
{
    fprintf(stderr, "Usage: %s RA_HOURS DEC_DEG LAT_DEG LON_DEG [UNIX_SECONDS]\n"
            "  RA: [0,24); Dec/latitude: [-90,90]; longitude: [-180,180], east positive.\n"
            "  Omit UNIX_SECONDS to use the current UTC time.\n", program);
}

int main(int argc, char **argv)
{
    struct celestial target;
    struct sky result;
    struct tm *utc;
    time_t timer;
    double latitude, longitude, ut, jd, lst;

    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        usage(argv[0]);
        return EXIT_SUCCESS;
    }
    if ((argc != 5 && argc != 6) ||
        !parse_number(argv[1], &target.ra) ||
        !parse_number(argv[2], &target.dec) ||
        !parse_number(argv[3], &latitude) ||
        !parse_number(argv[4], &longitude)) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }
    if (argc == 6) {
        if (!parse_timestamp(argv[5], &timer)) {
            fprintf(stderr, "Invalid or unsupported Unix timestamp.\n");
            return EXIT_FAILURE;
        }
    } else if (time(&timer) == (time_t)-1) {
        fprintf(stderr, "Unable to obtain the current time.\n");
        return EXIT_FAILURE;
    }
    utc = gmtime(&timer);
    if (utc == NULL) {
        fprintf(stderr, "Unable to convert the timestamp to UTC.\n");
        return EXIT_FAILURE;
    }
    /* Check before adding 1900 to avoid overflow for extreme system times. */
    if (utc->tm_year < -1899 || utc->tm_year > 8099) {
        fprintf(stderr, "Supported calendar years are 1 through 9999.\n");
        return EXIT_FAILURE;
    }
    ut = utc->tm_hour + utc->tm_min / 60.0 + utc->tm_sec / 3600.0;
    jd = gtToJulian(utc->tm_mon + 1, utc->tm_mday, utc->tm_year + 1900);
    lst = utToLST(jd, ut, longitude);
    if (!isfinite(lst) ||
        RADecToAltAz(target.ra, target.dec, lst, latitude, &result) != 0) {
        fprintf(stderr, "Invalid coordinates or UTC date/time.\n");
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    printf("UTC: %04d-%02d-%02d %02d:%02d:%02d\n",
           utc->tm_year + 1900, utc->tm_mon + 1, utc->tm_mday,
           utc->tm_hour, utc->tm_min, utc->tm_sec);
    printf("Julian date: %.8f\n", jd + ut / 24.0);
    printf("Local mean sidereal time (hours): %.8f\n", lst);
    printf("Altitude (degrees): %.8f\n", result.alt);
    if (isnan(result.az))
        printf("Azimuth (degrees): undefined (zenith or nadir)\n");
    else
        printf("Azimuth (degrees east of north): %.8f\n", result.az);
    return EXIT_SUCCESS;
}
#endif
