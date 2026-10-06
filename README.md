# RADec

Convert equatorial right ascension and declination to geometric altitude and
azimuth for an observer's location and UTC time.

## Build and run

Requires a C11 compiler and the standard C math library.

```sh
make
./radec RA_HOURS DEC_DEG LAT_DEG LON_DEG [UNIX_SECONDS]
```

- RA is decimal hours, from 0 inclusive to 24 exclusive (degrees / 15).
- Declination and latitude are decimal degrees in [-90, 90], north positive.
- Longitude is decimal degrees in [-180, 180], east positive; Wisconsin's
  example longitude is -88.3. Supply your actual latitude and longitude.
- Optional Unix seconds are an integer since 1970-01-01 00:00:00 UTC.
  Omit them to use the current time. System timezone does not affect results.
- Altitude is degrees above the horizon; negative means below it.
- Azimuth is degrees clockwise from true north: east 90, south 180, west 270.
  At the zenith/nadir (within floating-point tolerance), azimuth is undefined.

Reproducible example at J2000 noon, Greenwich longitude, latitude 45 north:

```sh
./radec 18.6973745583 0 45 0 946728000
```

This gives approximately 45 degrees altitude and 180 degrees azimuth.
Invalid input produces a diagnostic and nonzero exit status. `--help` displays
usage. Direct compilation also works:

```sh
cc -std=c11 -Wall -Wextra -Wpedantic radec.c -lm -o radec
```

## Functions

Declarations, units, and error contracts are in `radec.h`. To use the routines
without the command-line main function, compile `radec.c` with
`-DRADEC_NO_MAIN` and link with `-lm`.

- `gtToJulian(month, day, year)` returns a Julian date. Fractional days are
  supported. Calendar dates use the proleptic Gregorian calendar for years
  1..9999, including before 1582; no historical calendar cutover is applied.
- `utToLST(julianDate, ut, longitude)` returns local **mean** sidereal time in
  hours. Pass the midnight Julian date (ending in .5) and elapsed UTC hours
  separately, so the time is counted exactly once.
- `RADecToAltAz(ra, dec, lst, latitude, result)` fills a `struct sky` and returns
  0 on success or -1 on invalid input. At zenith/nadir, `result->az` is `NAN`.

The latter two signatures changed from the original program to accept observer
coordinates explicitly and return the computed altitude/azimuth.

## Accuracy and scope

The Julian-date terms use `floor`, not rounding. Sidereal time follows the
[USNO approximate GMST formula](https://aa.usno.navy.mil/faq/GAST), treating
UTC as UT1 and TT for a simple approximation. The horizon transformation uses
the [USNO altitude/azimuth relations](https://aa.usno.navy.mil/faq/alt_az),
expressed as a vector rotation with `atan2` to preserve azimuth quadrants and
handle celestial poles.

Supply RA/Dec referred to the mean equator and equinox of the observation date
for consistency with mean sidereal time. This program does not convert J2000
catalog coordinates to the observation epoch. It omits precession, nutation,
aberration, proper motion, parallax, atmospheric refraction, and UT1/TT
corrections. It is a basic geometric conversion, not a precision pointing or
solar-system ephemeris package. Calendar support over years 1..9999 does not
imply that the approximate sidereal model has uniform accuracy over that range.

## Tests

Python 3 is required only for the CLI tests.

```sh
make test
make clean
make test CFLAGS='-O1 -g -std=c11 -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined -fno-omit-frame-pointer'
```

Tests cover reference Julian dates and sidereal times, leap-year rules, midnight
rollover, longitude wrapping, cardinal sky positions, celestial and observer
poles, undefined azimuth, general spherical-coordinate round trips, invalid
inputs, deterministic timestamps, and timezone independence.
