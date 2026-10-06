"""Exercise the actual command line, including validation and timezone independence."""
import os
import subprocess
import sys

program = sys.argv[1]
checks = 0


def run(args, success=True, tz='UTC'):
    global checks
    result = subprocess.run([program, *args], capture_output=True, text=True,
                            env={**os.environ, 'TZ': tz})
    checks += 1
    assert (result.returncode == 0) == success, (args, result.stderr)
    if not success:
        assert result.stderr and not result.stdout, (args, result)
    return result.stdout


args = ['18.6973745583', '0', '45', '0', '946728000']
reference = run(args)
assert 'UTC: 2000-01-01 12:00:00' in reference
assert 'Julian date: 2451545.00000000' in reference
values = [float(line.rsplit(': ', 1)[1]) for line in reference.splitlines()[2:]]
assert abs(values[0] - 18.6973745583) < 2e-6
assert abs(values[1] - 45) < 1e-6
assert abs(values[2] - 180) < 1e-4
assert run(args, tz='America/Detroit') == reference
assert run(args, tz='Pacific/Auckland') == reference
assert '1970-01-01 00:00:00' in run(['0', '0', '0', '0', '0'])
assert '1969-12-31 23:59:59' in run(['0', '0', '0', '0', '-1'])
assert 'UTC:' in run(['0', '0', '0', '0'])
run(['--help'])
run([], False)
run(['0'], False)
run(['0', '0', '0', '0', '0', 'extra'], False)
for index, bad_values in enumerate([
    ['24', '-1', 'nan', 'inf', 'abc', '1x', '', '1e999'],
    ['91', '-91', 'nan', 'inf'],
    ['91', '-91', 'nan', 'inf'],
    ['181', '-181', 'nan', 'inf'],
    ['1.5', 'nan', 'abc', '', '999999999999999999999999',
     '-62135596801', '253402300800'],
]):
    for bad in bad_values:
        case = ['0', '0', '0', '0', '0']
        case[index] = bad
        run(case, False)
print(f'Passed {checks} CLI checks.')
