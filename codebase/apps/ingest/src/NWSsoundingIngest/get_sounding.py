#!/usr/bin/env python3

from datetime import datetime
from siphon.simplewebservice.wyoming import WyomingUpperAir

def write_sharppy_file(df, station, when, path):
    with open(path, "w") as out:
        out.write("%TITLE%\n")
        out.write(f" {station} {when:%y%m%d/%H%M}\n\n")
        out.write("%RAW%\n")

        # SHARPpy/SPC assumed order:
        # pressure_mb, height_m, temp_C, dewpoint_C, wind_dir_deg, wind_speed_kt
        for _, row in df.iterrows():
            out.write(
                f"{row['pressure']:8.2f},"
                f"{row['height']:8.2f},"
                f"{row['temperature']:8.2f},"
                f"{row['dewpoint']:8.2f}
                f"{row['direction']:8.2f},"
                f"{row['speed']:8.2f}\n"
            )

        out.write("%END%\n")

def main():

    when = datetime(2024, 5, 20, 0)
    station = "OUN"

    df = WyomingUpperAir.request_data(when, station)
    write_sharppy_file(df, station, when, "OUN_20240520_0000.sharppy")

########################################################################
# Run - entry point

if __name__ == "__main__":
   main()
