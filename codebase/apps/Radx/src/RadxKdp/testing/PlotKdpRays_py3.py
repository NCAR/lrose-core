#!/usr/bin/env python3

#===========================================================================
#
# Plot Ray details for KDP analysis
#
# Python 3 version.
#
#===========================================================================

import argparse
import os
import subprocess
import sys

import matplotlib.pyplot as plt


REQUIRED_COLUMNS = [
    "gateNum",
    "validKdp",
    "validUnfold",
    "snr",
    "dbz",
    "zdr",
    "zdrSdev",
    "rhohv",
    "phidp",
    "phidpMean",
    "phidpMeanValid",
    "phidpJitter",
    "phidpSdev",
    "phidpMeanUnfold",
    "phidpUnfold",
    "phidpFilt",
    "phidpCond",
    "phidpCondFilt",
    "psob",
    "kdp",
    "dbzAtten",
    "zdrAtten",
    "dbzCorrected",
    "zdrCorrected",
    "regrFilt",
    "phidpFftFilt",
]


class KdpRayPlotter:
    def __init__(self, options):
        self.options = options
        self.dir_path = os.path.dirname(os.path.abspath(options.input_file_path))
        self.file_list = []
        self.file_index = 0
        self.col_headers = []
        self.col_index = {}
        self.data = {name: [] for name in REQUIRED_COLUMNS}
        self.fig = None
        self.ax1 = None
        self.ax2 = None
        self.ax3 = None
        self.ax4 = None

    def read_file_list(self):
        if not os.path.isdir(self.dir_path):
            raise FileNotFoundError(f"Input directory does not exist: {self.dir_path}")

        self.file_list = sorted(
            name for name in os.listdir(self.dir_path)
            if os.path.isfile(os.path.join(self.dir_path, name))
        )

        if not self.file_list:
            raise RuntimeError(f"No files found in directory: {self.dir_path}")

        input_basename = os.path.basename(self.options.input_file_path)
        try:
            self.file_index = self.file_list.index(input_basename)
        except ValueError:
            raise FileNotFoundError(
                f"Input file {input_basename!r} was not found in {self.dir_path}"
            ) from None

        if self.options.debug:
            print(f"Dir path: {self.dir_path}", file=sys.stderr)
            print(f"Input file path: {self.options.input_file_path}", file=sys.stderr)
            print(f"File index: {self.file_index}", file=sys.stderr)
            print(f"n files: {len(self.file_list)}", file=sys.stderr)
            print(f"Computed file path: {self.get_file_path()}", file=sys.stderr)

        if self.options.verbose:
            print("Files:", file=sys.stderr)
            for index, filename in enumerate(self.file_list):
                print(f"   {index}: {filename}", file=sys.stderr)

    def get_file_path(self):
        return os.path.join(self.dir_path, self.file_list[self.file_index])

    def read_column_headers(self):
        file_path = self.get_file_path()

        with open(file_path, "r", encoding="utf-8") as fp:
            line = fp.readline()

        if not line.startswith("#"):
            raise ValueError(
                f"First line of {file_path} does not start with '#'; cannot read column headers"
            )

        self.col_headers = line.lstrip("# ").rstrip("\n").split()
        self.col_index = {name: index for index, name in enumerate(self.col_headers)}

        missing = [name for name in REQUIRED_COLUMNS if name not in self.col_index]
        if missing:
            raise ValueError(
                f"File {file_path} is missing required columns: {', '.join(missing)}"
            )

        if self.options.debug:
            print(f"colHeaders: {self.col_headers}", file=sys.stderr)
            print(f"colIndex: {self.col_index}", file=sys.stderr)

    def read_input_data(self):
        self.data = {name: [] for name in REQUIRED_COLUMNS}
        file_path = self.get_file_path()
        max_gates = int(self.options.max_gates)

        with open(file_path, "r", encoding="utf-8") as fp:
            row_num = 0
            for line_num, line in enumerate(fp, start=1):
                stripped = line.strip()

                if not stripped or stripped.startswith("#"):
                    continue

                fields = stripped.split()
                if len(fields) < len(self.col_headers):
                    raise ValueError(
                        f"Line {line_num} in {file_path} has {len(fields)} fields; "
                        f"expected at least {len(self.col_headers)}"
                    )

                for name in REQUIRED_COLUMNS:
                    value = fields[self.col_index[name]]
                    try:
                        if name == "gateNum":
                            self.data[name].append(int(float(value)))
                        else:
                            self.data[name].append(float(value))
                    except ValueError as e:
                        raise ValueError(
                            f"Could not convert column {name!r} value {value!r} "
                            f"to a number on line {line_num} in {file_path}"
                        ) from e

                row_num += 1
                if row_num >= max_gates:
                    break

        if self.options.debug:
            print(f"Read {len(self.data['gateNum'])} data rows from {file_path}", file=sys.stderr)

    def press(self, event):
        if self.options.debug:
            print(f"press: {event.key}", file=sys.stderr)

        if event.key == "left" and self.file_index > 0:
            self.file_index -= 1
            self.reload_and_draw()
        elif event.key == "right" and self.file_index < len(self.file_list) - 1:
            self.file_index += 1
            self.reload_and_draw()

        if self.options.debug:
            print(f"  File index: {self.file_index}", file=sys.stderr)
            print(f"  File path: {self.get_file_path()}", file=sys.stderr)

    def plot_xy(self):
        width_in = float(self.options.fig_width_mm) / 25.4
        height_in = float(self.options.fig_height_mm) / 25.4

        self.fig = plt.figure(1, (width_in, height_in))
        self.fig.canvas.mpl_connect("key_press_event", self.press)

        self.ax1 = self.fig.add_subplot(2, 2, 1, xmargin=0.0)
        self.ax2 = self.fig.add_subplot(2, 2, 2, xmargin=0.0)
        self.ax3 = self.fig.add_subplot(2, 2, 3, xmargin=0.0)
        self.ax4 = self.fig.add_subplot(2, 2, 4, xmargin=0.0)

        self.do_plot()
        self.fig.suptitle(self.options.title)
        plt.tight_layout()
        plt.subplots_adjust(top=0.9)
        plt.show()

    def reload_and_draw(self):
        self.read_column_headers()
        self.read_input_data()
        self.do_plot()
        plt.draw()

    def do_plot(self):
        self.ax1.clear()
        self.ax2.clear()
        self.ax3.clear()
        self.ax4.clear()

        filename = self.file_list[self.file_index]
        name_parts = filename.split("_")
        time_str = "Time " + name_parts[1] if len(name_parts) > 1 else filename
        el_str = name_parts[2] if len(name_parts) > 2 else "elevation"
        az_str = name_parts[3] if len(name_parts) > 3 else "azimuth"

        gate_num = self.data["gateNum"]
        valid_kdp = self.data["validKdp"]
        valid_unfold = self.data["validUnfold"]

        valid_kdp1 = [val * 40.0 for val in valid_kdp]
        valid_unfold1 = [val * -40.0 for val in valid_unfold]
        rhohv50 = [val * 50.0 for val in self.data["rhohv"]]
        zdr_sdev10 = [val * 10.0 for val in self.data["zdrSdev"]]

        self.ax1.set_title(time_str, fontsize=12)
        self.ax1.plot(gate_num, valid_kdp1, "k:", label="validKdp")
        self.ax1.plot(gate_num, valid_unfold1, "b:", label="validUnfold")
        self.ax1.plot(gate_num, self.data["snr"], label="SNR")
        self.ax1.plot(gate_num, self.data["dbz"], label="DBZ")
        self.ax1.plot(gate_num, rhohv50, label="RHOHV*50")
        self.ax1.plot(gate_num, zdr_sdev10, label="ZdrSdev*10")
        self.ax1.plot(gate_num, self.data["dbzAtten"], label="dbzAtten")
        legend1 = self.ax1.legend(loc="upper right")
        for label in legend1.get_texts():
            label.set_fontsize("small")
        self.ax1.set_xlabel("gateNum")
        self.ax1.set_ylabel("SNR, DBZ")

        valid_kdp2 = [val * 20.0 for val in valid_kdp]
        valid_unfold2 = [val * -20.0 for val in valid_unfold]

        self.ax2.set_title(az_str, fontsize=12)
        self.ax2.plot(gate_num, valid_kdp2, "k:", label="validKdp")
        self.ax2.plot(gate_num, valid_unfold2, "b:", label="validUnfold")
        self.ax2.plot(gate_num, self.data["phidpUnfold"], label="unfolded", color="green")
        self.ax2.plot(gate_num, self.data["phidpMeanUnfold"], label="meanUnfolded", color="cyan")
        self.ax2.plot(gate_num, self.data["phidpFilt"], label="Filt", color="red")
        self.ax2.plot(gate_num, self.data["phidpCondFilt"], label="CondFilt", color="black")
        #self.ax2.plot(gate_num, self.data["regrFilt"], label="RegrFilt", color="magenta")
        self.ax2.plot(gate_num, self.data["phidpFftFilt"], label="FftFilt", color="magenta")
        self.ax2.plot(gate_num, self.data["phidpSdev"], label="Sdev", color="pink")
        self.ax2.plot(gate_num, self.data["phidpJitter"], label="Jitter", color="orange")
        legend2 = self.ax2.legend(loc="upper right")
        for label in legend2.get_texts():
            label.set_fontsize("small")
        self.ax2.set_xlabel("gateNum")
        self.ax2.set_ylabel("PHIDP")

        valid_kdp3 = [val * 5.0 for val in valid_kdp]
        valid_unfold3 = [val * -2.0 for val in valid_unfold]

        self.ax3.set_title(el_str, fontsize=12)
        self.ax3.plot(gate_num, valid_kdp3, "k:", label="validKdp")
        self.ax3.plot(gate_num, valid_unfold3, "b:", label="validUnfold")
        self.ax3.plot(gate_num, self.data["psob"], label="psob", color="orange")
        self.ax3.plot(gate_num, self.data["kdp"], label="KDP", color="red")
        self.ax3.plot(gate_num, self.data["rhohv"], label="RHOHV", color="green")
        legend3 = self.ax3.legend(loc="upper left")
        for label in legend3.get_texts():
            label.set_fontsize("small")
        self.ax3.set_xlabel("gateNum")
        self.ax3.set_ylabel("KDP,PSOB")

        self.ax4.set_title(az_str, fontsize=12)
        self.ax4.plot(gate_num, valid_kdp2, "k:", label="validKdp")
        self.ax4.plot(gate_num, valid_unfold2, "b:", label="validUnfold")
        self.ax4.plot(gate_num, self.data["phidp"], label="phidp")
        self.ax4.plot(gate_num, self.data["phidpFftFilt"], label="phidpFftFilt")
        legend2 = self.ax4.legend(loc="upper right")
        for label in legend2.get_texts():
            label.set_fontsize("small")
        self.ax4.set_xlabel("gateNum")
        self.ax4.set_ylabel("PHIDP")


#=========================================================================
# Run a command in a shell, wait for it to complete


def run_command(cmd, debug=False):
    if debug:
        print(f"running cmd: {cmd}", file=sys.stderr)

    try:
        completed = subprocess.run(cmd, shell=True, check=False)
    except OSError as e:
        print(f"Execution failed: {e}", file=sys.stderr)
        return -1

    retcode = completed.returncode
    if retcode < 0:
        print(f"Child was terminated by signal: {-retcode}", file=sys.stderr)
    elif debug:
        print(f"Child returned code: {retcode}", file=sys.stderr)

    return retcode


#=========================================================================
# Main program


def parse_args():
    parser = argparse.ArgumentParser(description="Plot ray details for KDP analysis")
    parser.add_argument("--debug", action="store_true", help="Set debugging on")
    parser.add_argument("--verbose", action="store_true", help="Set verbose debugging on")
    parser.add_argument(
        "--file",
        dest="input_file_path",
        default="/tmp/kdp_ray_files/kdp_ray_20130531-231614.500el002.8_az188.0.txt",
        help="Input file path",
    )
    parser.add_argument("--title", default="KDP analysis", help="Title for plot")
    parser.add_argument("--width", dest="fig_width_mm", type=float, default=300.0,
                        help="Width of figure in mm")
    parser.add_argument("--height", dest="fig_height_mm", type=float, default=150.0,
                        help="Height of figure in mm")
    parser.add_argument("--maxgates", dest="max_gates", type=int, default=10000,
                        help="Max number of gates to plot")

    options = parser.parse_args()
    if options.verbose:
        options.debug = True
    return options


def main():
    options = parse_args()

    if options.debug:
        print("Running PlotKdpRays:", file=sys.stderr)
        print(f"  inputFilePath: {options.input_file_path}", file=sys.stderr)
        print(f"  maxGates: {options.max_gates}", file=sys.stderr)

    try:
        plotter = KdpRayPlotter(options)
        plotter.read_file_list()
        plotter.read_column_headers()
        plotter.read_input_data()
        plotter.plot_xy()
    except Exception as e:
        print(f"ERROR - PlotKdpRays failed: {e}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
