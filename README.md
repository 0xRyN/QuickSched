# Systems Project - Process Schedulers

## How to run

The Makefile provides 4 targets, `quicklifo`, `quicksteal`, `stealbench` and `fracsteal`:

-   `quicklifo` and `quicksteal` are the two main schedulers, running the quicksort algorithm with LIFO and Steal strategies respectively.

-   `stealbench` is a benchmarking tool that checks how many steals succeed and how many fail, along with the distribution of tasks run by each thread.

-   `fracsteal` is an SDL2 application that visualizes the work-stealing algorithm in action, by rendering beautiful Mandelbrot fractals and benchmarking frame times.

**All targets output an executable file with the same name as the target.**

## How to run the interactive benchmark

You will need Python to run the interactive benchmark. It's used to detect your config, setup variables and run the benchmarking script and to plot the results visually.

### Installing Python

Ensure you have Python 3.11 or newer installed on your system. Download Python from [the official website](https://www.python.org/downloads/).

Lower versions _should_ work though, but we haven't tested them.

### Installing required packages

Run the following command to install the required packages:

`pip install -r requirements.txt`

### Running the benchmark

Run the following command to start the interactive benchmark notebook:

`jupyter notebook InteractiveBenchmark.ipynb`

The notebook should open in your default browser.

If it doesn't, open your browser and navigate to `http://localhost:8888/notebooks/InteractiveBenchmark.ipynb`.

Afterwards, just follow the instructions in the Notebook to get started.
