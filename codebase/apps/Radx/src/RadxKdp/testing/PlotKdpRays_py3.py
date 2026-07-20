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
    "kdpSC",
    "kdpZZdr",
    "dbzAtten",
    "zdrAtten",
    "dbzCorrected",
    "zdrCorrected",
    "regrFilt",
    "phidpFftFilt",
    "phidpFftCond",
    "phidpFiltTrend",
    "scBlock",
    "phidpSC",
]

############################################################################
# initializer

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
        self.ax1r = None
        self.ax2 = None
        self.ax3 = None
        self.ax4 = None
        self.ax4r = None
        self.first_valid = None
        self.last_valid = None

    # determine and apply the range to be plotted

    def set_plot_limits(self):
        valid_kdp = self.data["validKdp"]

        if not valid_kdp:
            self.first_valid = None
            self.last_valid = None
            return

        if not self.options.valid_only:
            self.first_valid = 0
            self.last_valid = len(valid_kdp) - 1
            return

        self.first_valid = None
        self.last_valid = None

        for i, value in enumerate(valid_kdp):
            if value:
                if self.first_valid is None:
                    self.first_valid = i
                self.last_valid = i

    def trim_valid(self, data):
        if self.first_valid is None or self.last_valid is None:
            return []

        return data[self.first_valid:self.last_valid + 1]

    # read list of ray files

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

    # get file path
    
    def get_file_path(self):
        return os.path.join(self.dir_path, self.file_list[self.file_index])

    # read column headers in input file
    
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

    # read input data from file
    
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

        self.set_plot_limits()

        if self.options.debug:
            print(
                f"Plot index range: {self.first_valid} to {self.last_valid}",
                file=sys.stderr,
            )

    # get key press
    
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

    # do plot
    
    def plot_xy(self):
        width_in = float(self.options.fig_width_mm) / 25.4
        height_in = float(self.options.fig_height_mm) / 25.4

        self.fig = plt.figure(1, (width_in, height_in))
        self.fig.canvas.mpl_connect("key_press_event", self.press)

        self.ax1 = self.fig.add_subplot(2, 2, 1, xmargin=0.0)
        self.ax1r = self.ax1.twinx()
        self.ax2 = self.fig.add_subplot(2, 2, 2, xmargin=0.0)
        self.ax3 = self.fig.add_subplot(2, 2, 3, xmargin=0.0)
        self.ax4 = self.fig.add_subplot(2, 2, 4, xmargin=0.0)
        self.ax4r = self.ax4.twinx()

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
        self.ax1r.clear()
        self.ax2.clear()
        self.ax3.clear()
        self.ax4.clear()
        self.ax4r.clear()

        filename = self.file_list[self.file_index]
        name_parts = filename.split("_")
        time_str = "Time " + name_parts[1] if len(name_parts) > 1 else filename
        el_str = name_parts[2] if len(name_parts) > 2 else "elevation"
        az_str = name_parts[3] if len(name_parts) > 3 else "azimuth"

        # Every field uses exactly the same inclusive plot slice.
        plot_data = {
            name: self.trim_valid(values)
            for name, values in self.data.items()
        }

        gate_num = plot_data["gateNum"]
        valid_kdp = plot_data["validKdp"]

        if not gate_num:
            message = "No valid KDP gates" if self.options.valid_only else "No data"
            for ax in (self.ax1, self.ax2, self.ax3, self.ax4):
                ax.text(
                    0.5, 0.5, message,
                    transform=ax.transAxes,
                    ha="center", va="center",
                )
            self.ax1.set_title(time_str, fontsize=12)
            self.ax2.set_title(az_str, fontsize=12)
            self.ax3.set_title(el_str, fontsize=12)
            self.ax4.set_title(az_str, fontsize=12)
            return

        zdr_sdev10 = [value * 10.0 for value in plot_data["zdrSdev"]]

        # PLOT 1 - moments

        self.ax1.set_title(time_str, fontsize=12)
        self.ax1.plot(gate_num, plot_data["phidpSdev"], label="Sdev", color="pink")
        self.ax1.plot(gate_num, plot_data["phidpJitter"], label="Jitter", color="orange")
        self.ax1r.plot(
            gate_num, plot_data["rhohv"],
            label="RHOHV", color="seagreen",
        )
        self.ax1.plot(gate_num, zdr_sdev10, label="ZdrSdev*10", color="blue")
        self.ax1.plot(gate_num, plot_data["snr"], label="SNR", color="black")
        self.ax1.plot(gate_num, plot_data["dbz"], label="DBZ", color="red")
        self.ax1.set_xlabel("gateNum")
        self.ax1.set_ylabel("SNR, DBZ")

        self.ax1r.set_ylabel("RHOHV", color="seagreen")
        self.ax1r.yaxis.set_label_position("right")
        self.ax1r.yaxis.tick_right()
        self.ax1r.set_ylim(-0.2, 1.5)
        self.ax1r.tick_params(axis="y", labelcolor="seagreen")

        lines1, labels1 = self.ax1.get_legend_handles_labels()
        lines2, labels2 = self.ax1r.get_legend_handles_labels()
        legend1 = self.ax1.legend(
            lines1 + lines2, labels1 + labels2, loc="upper right"
        )
        for label in legend1.get_texts():
            label.set_fontsize("small")

        draw_valid_regions(
            self.ax1, gate_num, valid_kdp,
            color="lightgray", alpha=0.4,
        )
        draw_block_limits(self.ax1, gate_num, self.data["scBlock"])

        # PLOT 2 - PHIDP processing

        self.ax2.set_title(az_str, fontsize=12)
        self.ax2.plot(gate_num, plot_data["phidpUnfold"], label="unfolded", color="green")
        self.ax2.plot(gate_num, plot_data["phidpFilt"], label="Filt", color="red")
        self.ax2.plot(gate_num, plot_data["phidpCondFilt"], label="CondFilt", color="black")
        self.ax2.plot(gate_num, plot_data["phidpFftFilt"], label="FftFilt", color="magenta")
        self.ax2.plot(gate_num, plot_data["phidpSC"], label="phidpSC", color="orange")
        draw_block_limits(self.ax2, gate_num, self.data["scBlock"])
        self.ax2.set_xlabel("gateNum")
        self.ax2.set_ylabel("PHIDP")

        legend2 = self.ax2.legend(loc="upper right")
        for label in legend2.get_texts():
            label.set_fontsize("small")

        draw_valid_regions(
            self.ax2, gate_num, valid_kdp,
            color="lightgray", alpha=0.4,
        )

        # PLOT 3 - KDP and PSOB

        self.ax3.set_title(el_str, fontsize=12)
        self.ax3.plot(gate_num, plot_data["psob"], label="PSOB", color="orange")
        self.ax3.plot(gate_num, plot_data["kdp"], label="KDP", color="red")
        self.ax3.plot(gate_num, plot_data["kdpSC"], label="KDP_SC", color="blue")
        self.ax3.plot(gate_num, plot_data["kdpZZdr"], label="KDP_ZZDR", color="green")
        self.ax3.plot(gate_num, plot_data["phidpFiltTrend"], label="TREND", color="magenta")
        self.ax3.set_xlabel("gateNum")
        self.ax3.set_ylabel("KDP, PSOB")

        draw_valid_regions(
            self.ax3, gate_num, valid_kdp,
            color="lightgray", alpha=0.4,
        )
        draw_block_limits(self.ax3, gate_num, self.data["scBlock"])
                
        legend3 = self.ax3.legend(loc="upper right")
        for label in legend3.get_texts():
            label.set_fontsize("small")

        # PLOT 4 - PHIDP FFT filtering

        self.ax4.set_title(az_str, fontsize=12)
        self.ax4.plot(gate_num, plot_data["phidp"], label="phidp")
        self.ax4.plot(gate_num, plot_data["phidpFftFilt"], label="phidpFftFilt")
        self.ax4.set_xlabel("gateNum")
        self.ax4.set_ylabel("PHIDP")
        draw_block_limits(self.ax4, gate_num, self.data["scBlock"])

        self.ax4r.plot(
            gate_num, plot_data["zdr"],
            label="ZDR", color="red",
        )
        self.ax4r.set_ylabel("ZDR", color="red")
        self.ax4r.yaxis.set_label_position("right")
        self.ax4r.yaxis.tick_right()
        self.ax4r.set_ylim(-5, 10)
        self.ax4r.tick_params(axis="y", labelcolor="red")

        draw_valid_regions(
            self.ax4, gate_num, valid_kdp,
            color="lightgray", alpha=0.4,
        )

        legend4 = self.ax4.legend(loc="upper right")
        for label in legend4.get_texts():
            label.set_fontsize("small")


#=========================================================================
# draw valid regions on plots

def draw_valid_regions(ax, x, valid,
                       color='lightgray',
                       alpha=0.4):

    if not x or not valid:
        return

    in_run = False

    for i in range(len(valid)):

        if valid[i] and not in_run:
            start = x[i]
            in_run = True

        elif not valid[i] and in_run:
            end = x[i]
            ax.axvspan(start, end,
                       facecolor=color,
                       edgecolor='none',
                       alpha=alpha)
            ax.axvline(start, color='black', lw=1)
            ax.axvline(end,   color='black', lw=1)
            in_run = False

    # Final run reaches end of data
    if in_run:
        ax.axvspan(start, x[-1],
                   facecolor=color,
                   edgecolor='none',
                   alpha=alpha)
        ax.axvline(start, color='black', lw=1)
        ax.axvline(end,   color='black', lw=1)

#=========================================================================
# draw block limit lines

def draw_block_limits(ax, x, valid):
    if not x or not valid:
        return
    for i in range(len(valid)):
        if valid[i]:
            ax.axvline(x[i], color='pink', lw=1)

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
    parser.add_argument("--valid_only", action="store_true",
                        help="Plot only from the first through last valid KDP gate")
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
