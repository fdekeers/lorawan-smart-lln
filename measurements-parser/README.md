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

The [measurements](./measurements/) directory contains all the CSV data files exported from the electric current measurements. Those data files were used to produce the graphs present in the report.
