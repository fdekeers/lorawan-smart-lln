# Energy consumption measurements

This directory contains the data files exported from the energy consumption measurements of our end devices, carried out at the UCLouvain Welcome platform. It also contains the Python scripts used to generate the graphs and compute the electric charge values from the electric current measurements.

The [parse.py](parse.py) script is the main script, that computes the electric charge consumed by the device wake-up, and produces the graphs. This script takes multiple command line arguments: first the title of the graph, then a variable number of CSV data files, containing the electric current measurements. All the data files will have a waveform present on the graph. For each data file, a name must be provided, that will be used to identify the data set on the graph legend. In practice, this script must be run as follows:
```shell
python3 parse.py GRAPH_TITLE DATA_FILE_A:NAME_A DATA_FILE_B:NAME_B DATA_FILE_C:NAME_C ...
```
Python version 3+ is required, as well as the following packages:
- `ast`
- `os`
- `sys`
- `numpy`
- `scipy`
- `matplotlib`

The [parse-sleep.py](parse-sleep.py) is a simplified version of the main script, that simply shows the waveform related to data files. It was used to visualize the deep sleep current. Its utilization is similar to the main script, but the first command line argument, i.e. the graph title, must not be provided.

The [measurements](./measurements/) directory contains all the CSV data files exported from the electric current measurements. Those data files were used to produce the graphs present in the report. They are organised with the following structure:
- [batch_measure_only](./measurements/batch_measure_only/): reading of environmental values, without transmission
- [batch_v2](./measurements/batch_v2/): LoRaWAN transmission using packet format V2, with and without ADR
- [batch_v3](./measurements/batch_v3/): LoRaWAN transmission using packet format V3, with and without ADR
- [cpu_freq](./measurements/cpu_freq/): measurements with different CPU frequencies
- [direct_v1](./measurements/direct_v1/): LoRaWAN transmission using packet format V1, with and without ADR
- [join](./measurements/join): setup operations (Wi-Fi retrieving of geo-location, LoRaWAN join procedure, etc.)
- [sensors](./measurements/sensors/): measurements with different connected sensor modules
- [sleep](./measurements/sleep): deep sleep current measurements
- [wifi](./measurements/wifi/): Wi-Fi transmission
