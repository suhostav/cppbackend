import argparse
import subprocess
import time
import random
import shlex
import signal

RANDOM_LIMIT = 1000
SEED = 123456789
random.seed(SEED)

AMMUNITION = [
    'localhost:8080/api/v1/maps/map1',
    'localhost:8080/api/v1/maps'
]

SHOOT_COUNT = 100
COOLDOWN = 0.1


def start_server():
    parser = argparse.ArgumentParser()
    parser.add_argument('server', type=str)
    return parser.parse_args().server


def run(command, output=None):
    process = subprocess.Popen(shlex.split(command), stdout=output, stderr=subprocess.DEVNULL)
    return process

# def run_server(command, output=None):
#     process = subprocess.Popen(shlex.split(command), stdout=output, stderr=subprocess.DEVNULL, close_fds=True)
#     return process


def stop(process, wait=False):
    if process.poll() is None and wait:
        process.wait()
    process.terminate()


def shoot(ammo):
    hit = run('curl ' + ammo, output=subprocess.DEVNULL)
    time.sleep(COOLDOWN)
    stop(hit, wait=True)


def make_shots():
    for _ in range(SHOOT_COUNT):
        ammo_number = random.randrange(RANDOM_LIMIT) % len(AMMUNITION)
        shoot(AMMUNITION[ammo_number])
    print('Shooting complete')


server = run(start_server())
time.sleep(1)
# perf_str = ['sudo', 'perf', 'record', '-g', '-o', 'perf.data', '-p', str(server.pid)]
perf_str = 'sudo perf record -o perf.data -g -p ' + str(server.pid) + ' sleep 5'

# perf_str = ['sudo', 'perf', 'record', '-g', '-o', 'perf.data', '-p', str(server.pid)]
perf = subprocess.Popen(shlex.split(perf_str), close_fds=True)
# perf = subprocess.Popen(perf_str)
# perf = run(perf_str)

make_shots()

stop(server)
time.sleep(1)
stop(perf,True)
# perf.wait()
time.sleep(5)
print('make graph.svg')
output = subprocess.check_output('sudo perf script -i perf.data | ./FlameGraph/stackcollapse-perf.pl | ./FlameGraph/flamegraph.pl > graph.svg', shell=True)
print('Job done')
