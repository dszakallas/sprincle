#!/usr/bin/env python3

__author__ = 'Midiparse'

import argparse
import sys
import os
from subprocess import Popen, PIPE
import csv
import re
import datetime

description = 'Run Sprincle queries'

#queries = ["ConnectedSegment", "PosLength", "RouteSensor", "SemaphoreNeighbor", "SwitchSensor", "SwitchSet"]
queries = ["RouteSensor"]
#sizes = [2**i for i in range(0, 14)]
sizes = [2**i for i in range(0, 12)]


def main():
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument('modelpath',    type=str,   action='store', help="Path to the models")
    parser.add_argument('--count',    type=int,   action='store', help="Run count", default=5)

    args = parser.parse_args()

    date = datetime.datetime.now().isoformat()

    fieldnames = ['Scenario', 'Tool', 'Run', 'Case', 'Artifact',
                  'Phase', 'Iteration', 'Metric', 'Value']

    with open('results-{0}.csv'.format(date), 'w') as result:
        writer = csv.DictWriter(result, delimiter=',', fieldnames=fieldnames)
        writer.writeheader()

    for n in range(1, args.count):
        for query in queries:
            for size in sizes:
                print("{0}/{1}: Running {2}, size: {3}".format(n, args.count, query, size))
                file = os.path.join(args.modelpath, 'railway-repair-{0}-inferred.ttl'.format(size))
                p = Popen(['./{0}QueryCLI'.format(query), file], stdout=PIPE)
                output, err = p.communicate()
                matches = re.findall('^([0-9]+)$', output.decode("utf-8"), re.MULTILINE)

                with open('results-{0}.csv'.format(date), 'a') as result:
                    writer = csv.DictWriter(result, delimiter=',', fieldnames=fieldnames)
                    writer.writerow(
                        {
                            'Scenario': 'Batch',
                            'Tool': 'Sprincle',
                            'Run': n,
                            'Case': query,
                            'Artifact': size,
                            'Phase': 'Read',
                            'Iteration': 0,
                            'Metric': 'Time',
                            'Value': matches[0]

                        })
                    writer.writerow(
                        {
                            'Scenario': 'Batch',
                            'Tool': 'Sprincle',
                            'Run': n,
                            'Case': query,
                            'Artifact': size,
                            'Phase': 'Check',
                            'Iteration': 0,
                            'Metric': 'Time',
                            'Value': matches[1]

                        })
                    writer.writerow(
                        {
                            'Scenario': 'Batch',
                            'Tool': 'Sprincle',
                            'Run': n,
                            'Case': query,
                            'Artifact': size,
                            'Phase': 'Check',
                            'Iteration': 0,
                            'Metric': 'Matches',
                            'Value': matches[2]

                        })
        print("Done")


if __name__ == '__main__':
    main()
