#include "radec.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static int checks;
#define CHECK(condition) do { \
    ++checks; \
    if (!(condition)) { \
        fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #condition); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

static void near(double actual, double expected, double tolerance)
{
    CHECK(isfinite(actual) && fabs(actual - expected) <= tolerance);
}

static void position(double ra, double dec, double lst, double lat,
                     double alt, double az)
{
    struct sky result;
    CHECK(RADecToAltAz(ra, dec, lst, lat, &result) == 0);
    near(result.alt, alt, 1e-10);
    if (isnan(az))
        CHECK(isnan(result.az));
    else
        near(result.az, az, 1e-10);
}

int main(void)
{
    struct sky result = {123.0, 456.0};
    double lst;
    near(gtToJulian(1, 1.5, 2000), 2451545.0, 1e-9);
    near(gtToJulian(1, 1, 1970), 2440587.5, 1e-9);
    near(gtToJulian(1, 27, 1987), 2446822.5, 1e-9);
    near(gtToJulian(6, 19.5, 1987), 2446966.0, 1e-9);
    near(gtToJulian(1, 1, 1900), 2415020.5, 1e-9);
    near(gtToJulian(10, 15, 1582), 2299160.5, 1e-9);
    near(gtToJulian(1, 1, 1), 1721425.5, 1e-9);
    near(gtToJulian(12, 31, 9999), 5373483.5, 1e-9);
    near(gtToJulian(3, 1, 2000) - gtToJulian(2, 28, 2000), 2, 1e-9);
    near(gtToJulian(3, 1, 1900) - gtToJulian(2, 28, 1900), 1, 1e-9);
    near(gtToJulian(3, 1, 2400) - gtToJulian(2, 28, 2400), 2, 1e-9);
    CHECK(isnan(gtToJulian(2, 29, 1900)));
    CHECK(isnan(gtToJulian(4, 31, 2026)));
    CHECK(isnan(gtToJulian(0, 1, 2000)));
    CHECK(isnan(gtToJulian(13, 1, 2000)));
    CHECK(isnan(gtToJulian(1, 0, 2000)));
    CHECK(isnan(gtToJulian(1, 1, 0)));
    CHECK(isnan(gtToJulian(1, 1, 10000)));
    CHECK(isnan(gtToJulian(1, NAN, 2000)));
    CHECK(isnan(gtToJulian(1, INFINITY, 2000)));

    /* Published J2000 GMST reference: 18h 41m 50.54841s. */
    near(utToLST(2451544.5, 12, 0), 18.6973745583, 2e-6);
    /* Meeus worked example: 1987-04-10 19:21 UT, GMST 8h 34m 57.0896s. */
    near(utToLST(gtToJulian(4, 10, 1987), 19.35, 0),
         8 + 34 / 60.0 + 57.0896 / 3600.0, 2e-6);
    near(utToLST(2451544.5, 0, 0), 6.6645196458, 2e-6);
    near(utToLST(2451544.5, 0, 30) - utToLST(2451544.5, 0, 0), 2, 1e-10);
    near(utToLST(2451544.5, 0, -180), 18.6645196458, 2e-6);
    near(utToLST(2451544.5, 0, 180), 18.6645196458, 2e-6);
    lst = utToLST(2451544.5, 23.999, -88.3);
    CHECK(lst >= 0 && lst < 24);
    near(remainder(utToLST(2451545.5, 0, -88.3) - lst, 24),
         0.0010027379, 1e-6);
    CHECK(isnan(utToLST(2451545.0, 12, 0))); /* Reject non-midnight JD. */
    CHECK(isnan(utToLST(2451544.5, 24, 0)));
    CHECK(isnan(utToLST(2451544.5, -1, 0)));
    CHECK(isnan(utToLST(2451544.5, NAN, 0)));
    CHECK(isnan(utToLST(2451544.5, 0, 181)));
    CHECK(isnan(utToLST(INFINITY, 0, 0)));
    CHECK(isnan(utToLST(2451544.5, 0, NAN)));

    position(6, 0, 0, 0, 0, 90);    /* Eastern horizon. */
    position(18, 0, 0, 0, 0, 270);  /* Western horizon. */
    position(0, 0, 0, 45, 45, 180); /* Southern meridian. */
    position(0, 0, 0, -45, 45, 0);  /* Southern hemisphere. */
    position(0, 90, 0, 45, 45, 0);  /* North celestial pole. */
    position(0, -90, 0, 45, -45, 180);
    position(0, 0, 0, 0, 90, NAN);  /* Zenith. */
    position(0, 0, 12, 0, -90, NAN);/* Nadir. */
    position(0, 45, 0, 45, 90, NAN);
    position(0, 0, 0, 90, 0, 180);  /* Polar observer. */
    position(0, 0, 0, -90, 0, 0);
    CHECK(RADecToAltAz(NAN, 0, 0, 0, &result) == -1);
    CHECK(RADecToAltAz(24, 0, 0, 0, &result) == -1);
    CHECK(RADecToAltAz(-1, 0, 0, 0, &result) == -1);
    CHECK(RADecToAltAz(0, 91, 0, 0, &result) == -1);
    CHECK(RADecToAltAz(0, NAN, 0, 0, &result) == -1);
    CHECK(RADecToAltAz(0, 0, 24, 0, &result) == -1);
    CHECK(RADecToAltAz(0, 0, INFINITY, 0, &result) == -1);
    CHECK(RADecToAltAz(0, 0, 0, -91, &result) == -1);
    CHECK(RADecToAltAz(0, 0, 0, NAN, &result) == -1);
    CHECK(RADecToAltAz(0, 0, 0, 0, NULL) == -1);
    CHECK(result.alt == 123.0 && result.az == 456.0);
    /* Independent inverse spherical relations check a grid across all
     * azimuth quadrants, both hemispheres, and below-horizon positions. */
    for (int latitude = -80; latitude <= 80; latitude += 20) {
        for (int dec = -80; dec <= 80; dec += 20) {
            for (int ra = 0; ra < 24; ++ra) {
                const double rad = acos(-1.0) / 180.0;
                const double local_time = 3.25;
                double alt, az, phi, recovered_dec, recovered_hour_angle;
                CHECK(RADecToAltAz(ra, dec, local_time, latitude, &result) == 0);
                CHECK(result.alt >= -90 && result.alt <= 90);
                CHECK(result.az >= 0 && result.az < 360);
                alt = result.alt * rad;
                az = result.az * rad;
                phi = latitude * rad;
                recovered_dec = asin(sin(alt) * sin(phi)
                                     + cos(alt) * cos(phi) * cos(az)) / rad;
                recovered_hour_angle = atan2(-cos(alt) * sin(az),
                    sin(alt) * cos(phi) - cos(alt) * sin(phi) * cos(az)) / rad;
                near(recovered_dec, dec, 1e-10);
                near(remainder(recovered_hour_angle -
                               (local_time - ra) * 15.0, 360.0), 0, 1e-10);
            }
        }
    }
    printf("Passed %d numerical and input checks.\n", checks);
    return EXIT_SUCCESS;
}
