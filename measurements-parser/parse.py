from ast import parse
from os import stat
import sys
import numpy as np
from scipy import integrate
import matplotlib.pyplot as plt
import matplotlib

font = {
    'size': 13}

matplotlib.rc('font', **font)


class Parser:
    def __init__(self, filename, measurement_frequency=1000) -> None:
        fileopt = filename.split(':')
        self.filename = fileopt[0]
        self.legend = fileopt[1]
        self.x = []
        self.y = []
        self.activation_indexes = []
        self.sending_indexes = []
        self.measurement_frequency = measurement_frequency
        self.integrals = {}

    def print(self, value=''):
        print(f'   {value}')

    def parse_file(self) -> None:
        print(f'> Parsing file {self.filename}')

        with open(self.filename, 'r') as input_file:
            line_number = 1
            index = 0

            for line in input_file:
                if line_number < 9:
                    line_number += 1
                    continue

                data = line.strip().split(',')
                data_index = data[0]
                value = data[1]

                timestamp = int(data_index)/self.measurement_frequency
                value = float(value)

                self.x += [timestamp]
                self.y += [value]

                line_number += 1
                index += 1

            self.print(f'Parsed {line_number} lines')
            self.print(
                f'Found {index} data points from timestamp {self.x[0]} to {self.x[-1]}'
            )
            self.print()

    def compute_activation_indexes(self, treshold=0.004):
        if len(self.x) == 0 or len(self.y) == 0:
            self.parse_file()
        if len(self.activation_indexes) != 0:
            return self.activation_indexes

        print(f'> Getting activation intervals using treshold {treshold}')
        last_activation = None
        for i in range(1, len(self.x)):
            value = self.y[i]

            previous_value = self.y[i-1]

            if value > treshold and previous_value < treshold:
                self.print(f'Activation - Index: {i-1} - Value: {value}')
                if last_activation is not None:
                    self.print(
                        f'ERROR: Activation after activation at index {-1}')
                last_activation = i-10

            if value < treshold and previous_value > treshold:
                self.print(
                    f'Deactivation - Index: {i+1} - Value: {value}')
                if last_activation is None:
                    self.print(
                        f'ERROR: Deactivation without activation at index {i+1}')
                if i-last_activation > 100:
                    self.activation_indexes += [(last_activation, i+10)]
                else:
                    self.print('Not an activation. Discarding')
                last_activation = None
                self.print('=====')
        self.print()
        return self.activation_indexes

    def compute_lora_sending_indexes(self, delta=0.07):
        if len(self.x) == 0 or len(self.y) == 0:
            self.parse_file()
        if len(self.sending_indexes) != 0:
            return self.sending_indexes

        print(f'> Computing lora sending intervals using delta {delta}')
        last_activation = None
        for start, stop in self.compute_activation_indexes():
            self.print(f'Checking between {start} and {stop}')
            for i in range(start, stop):
                value = self.y[i]

                p1 = self.y[i-1]
                p2 = self.y[i-2]
                p3 = self.y[i-3]
                p4 = self.y[i-4]
                if last_activation is None and (value - p1 > delta or value - p2 > delta or value - p3 > delta or value - p4 > delta):
                    self.print(
                        f'  Lora sending start - Index: {i-1} - Value: {value}')
                    if last_activation is not None:
                        self.print(
                            f'  ERROR: Activation after activation at index {-1}')
                    last_activation = i-3

                if last_activation is not None and (p1 - value > delta or p2 - value > delta or p3 - value > delta or p4 - value > delta):
                    self.print(
                        f'  Lora sending stop - Index: {i+1} - Value: {value}')
                    if last_activation is None:
                        self.print(
                            f'  ERROR: Deactivation without activation at index {i+1}')
                    if i-last_activation > 5:
                        self.sending_indexes += [(last_activation, i+1)]
                    else:
                        self.print('Not a sending. Discarding')
                    last_activation = None
                    self.print('=====')
        self.print()
        return self.sending_indexes

    def compute_integral(self, indexes=[]):
        x = np.array(self.x)
        y = np.array(self.y)
        print(f'> Computing integrals with indexes {indexes}')
        integrale = 0
        for start, stop in indexes:
            self.print(
                f"Activation from {start} to {stop} ({x[stop] - x[start]}s)")
            integrale += integrate.simps(y[start:stop], x[start:stop])
            self.print(f'Integration value = {integrale}')
            self.print('======')

        self.print()
        return integrale

    def get_activation_data(self):
        if len(self.activation_indexes) == 0:
            self.compute_activation_indexes()
        tmp = []
        for start, stop in self.activation_indexes:
            tmp.append((self.x[start:stop], self.y[start:stop]))
        return tmp

    def compute_activation_integral(self):
        if len(self.activation_indexes) == 0:
            self.compute_activation_indexes()

        if len(self.activation_indexes) == 0:
            return 0  # Still zero => no indexes. Nothing to integrate on

        if self.integrals.get('activation', None) is None:
            self.integrals['activation'] = self.compute_integral(
                [self.activation_indexes[-1]]
            )
        return self.integrals.get('activation') * (1000/3600)

    def compute_lora_integral(self):
        if 'wifi' in self.filename:
            return 0

        if len(self.sending_indexes) == 0:
            self.compute_lora_sending_indexes(
                0.05)  # First time, compute indexes

        if len(self.sending_indexes) == 0:
            return 0  # Still zero => no indexes. Nothing to integrate on

        if self.integrals.get('lora', None) is None:
            self.integrals['lora'] = self.compute_integral(
                [self.sending_indexes[-1]]
            )
        return self.integrals.get('lora') * (1000/3600)

    def get_label(self):
        label = f'{self.legend}\n'
        label += f'Electric charge: {round(self.compute_activation_integral(), 4)}mAh'
        # label += f'LoRa: {round(self.compute_lora_integral(), 4)}mAh'
        return label

    def plot_last_activation(self, ax):
        data_array = parser.get_activation_data()
        label = self.get_label()
        y = [1000*x for x in data_array[-1][1]]
        ax.plot(
            y,
            label=label
        )

    def plot(self, axs, last_activation_only=False):
        label = self.get_label()
        y = [1000*x for x in self.y]
        axs[0].plot(
            y,
            label=label
        )
        axs[0].set_xlabel("Time [ms]")
        axs[0].set_ylabel("Current [mA]")
        axs[0].set_title("")
        axs[0].legend()

        self.plot_last_activation(axs[1])


if __name__ == '__main__':
    plt.rcParams.update({'font.size': 20})
    LAST_ACTIVATION_ONLY = True

    if not LAST_ACTIVATION_ONLY:
        fig, ax = plt.subplots(2)
    for i in range(2, len(sys.argv)):
        print('/------------------------------------------------\\')
        print(f'| {sys.argv[i]}')
        print('\\------------------------------------------------/')

        parser = Parser(sys.argv[i])
        # parser.parse_file()
        # data_array = parser.get_activation_data()

        # parser.compute_lora_sending_indexes(0.06)
        # parser.plot_last_activation()

        if LAST_ACTIVATION_ONLY:
            parser.plot_last_activation(plt)
        else:
            parser.plot(ax)

        print('='*30)
    plt.xlabel('Time [ms]')
    plt.ylabel('Current [mA]')
    plt.legend()
    plt.title(sys.argv[1])
    plt.show()
