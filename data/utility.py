from enum import Enum
import numpy as np
import re
import matplotlib.pyplot as plt
import polars as pl
import seaborn as sns
from enum import Enum
import itertools

class OutputType(Enum):
    """Enum for the different output types"""
    ENERGY = 1
    ENERGY_FULL = 2
    TIMING = 3

"""
Get a bounded list of colors from a colormap
"""
def get_colors_from_cmap(num_colors: int, cmap, color_lims=[]):
    # if color limits are empty, then use a range proportional to the number of colors
    # the higher the number of colors, the larger the range, approaching the full range of the colormap
    if len(color_lims) == 0:
        percentage_range = (1 - (1 / num_colors)) * 0.75
        color_lims = [0.5 * (1 - percentage_range), 0.5 * (1 + percentage_range)]

    return [cmap(i) for i in np.linspace(color_lims[0], color_lims[1], num_colors)]

# Measurement class
class Measurements:
    # def __init__(self, outputs: list[str], output_types: list[OutputType]) -> None:
    def __init__(self, energy_output: tuple[str, OutputType] = None, timing_output: tuple[str, OutputType] = None) -> None:
        self._dfs = dict()

        if energy_output is None and timing_output is None:
            print("No output files specified")
            return

        outputs = []
        if energy_output is not None:
            outputs.append(energy_output)
        if timing_output is not None:
            outputs.append(timing_output)

        for (output, output_type) in outputs:
            match output_type:
                case OutputType.ENERGY | OutputType.ENERGY_FULL:
                    self.energy_output_type = output_type
                    self.import_energy(output)

                case OutputType.TIMING:
                    self.import_timings(output)
                case _:
                    raise ValueError("Invalid output type")

    States = {
        "CPU": 25, # 25 mA as per https://ftdichip.com/wp-content/uploads/2020/08/DS_FT232BL_BQ.pdf
        # "CPU": 1.8, # mA as per https://www.willow.co.uk/TelosB_Datasheet.pdf
        "LPM": 0.200, # 200 uA as per https://ftdichip.com/wp-content/uploads/2020/08/DS_FT232BL_BQ.pdf
        # "LPM": 0.0051 + 0.021, # 26.1 uA as per https://www.willow.co.uk/TelosB_Datasheet.pdf
        "DEEP_LPM": 0.0061, # 6.1 uA as per https://www.willow.co.uk/TelosB_Datasheet.pdf
        "RADIO_RX": 18.8, # https://www.ti.com/lit/ds/symlink/cc2420.pdf?ts=1670166938154&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FCC2420
        "RADIO_TX": 17.4 # https://www.ti.com/lit/ds/symlink/cc2420.pdf?ts=1670166938154&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FCC2420
    }
    VOLTAGE = 3
    TICKS_PER_SECOND = 32768
    
    # class PowerStates():
    #     STATE_TIME = "state_time"
    #     AVG_CURRENT_MA = "avg_current_mA"
    #     CHARGE_MC = "charge_mC"
    #     CHARGE_MAH = "charge_mAh"
    #     POWER_MW = "power_mW"
    #     ENERGY_MJ = "energy_mJ"

    #     def as_list(self) -> list[str]:
    #         return [self.STATE_TIME, self.AVG_CURRENT_MA, self.CHARGE_MC, self.CHARGE_MAH, self.POWER_MW, self.ENERGY_MJ]

    """
    Getter for the dataframes
    """
    def get_dfs(self) -> dict:
        return self._dfs
    
    """
    Import the energy measurements from the output file
    """
    def import_energy(self, path: str) -> None:
        # Extract the summaries
        summaries = self.extract_summaries(path)

        match self.energy_output_type:
            case OutputType.ENERGY:            
                # Extract the values
                sensor_values = [self.extract_values(row) for row in summaries["Sensor energy"]]
                aes_values = [self.extract_values(row) for row in summaries["AES energy"]]
                tx_values = [self.extract_values(row) for row in summaries["TX energy"]]
                # Calculate state energy stats
                # calculate_state_energy
                sensor_state_energy = [self.calculate_state_energy(row) for row in sensor_values]
                aes_state_energy = [self.calculate_state_energy(row) for row in aes_values]
                tx_state_energy = [self.calculate_state_energy(row) for row in tx_values]

                # Create the DataFrames
                self._dfs["Energy raw"] = {
                    "Sensor": pl.DataFrame(sensor_values),
                    "AES": pl.DataFrame(aes_values),
                    "TX": pl.DataFrame(tx_values)
                }
                self._dfs["Energy state"] = {
                    "Sensor": pl.DataFrame(sensor_state_energy),
                    "AES": pl.DataFrame(aes_state_energy),
                    "TX": pl.DataFrame(tx_state_energy)
                }

            case OutputType.ENERGY_FULL:
                # Extract the values
                values = [self.extract_values(row) for row in summaries["full energy"]]

                # Calculate state energy stats
                state_energy = [self.calculate_state_energy(row) for row in values]

                self._dfs["Energy raw"] = pl.DataFrame(values)
                self._dfs["Energy state"] = pl.DataFrame(state_energy)
    
    """
    Import the timing measurements from the output file
    """
    def import_timings(self, path: str) -> None:
        self._dfs["Timings"] = self.extract_timings(path)

    """
    Takes a path to a file containing the timing measurements
    Returns a DataFrame with the timing measurements
    """
    def extract_timings(self, path: str) -> pl.DataFrame:
        """Extract the timings from the output timing file"""
        # Read the file
        with open(path, "r") as f:
            file = f.read()
        # Extract Sensor timings
        sensor_timings = re.findall(r"Sensor time: (\d+)", file)
        sensor_timings = [int(t) for t in sensor_timings]
        # Extract AES encryption timings
        aes_timings = re.findall(r"AES time: (\d+)", file)
        aes_timings = [int(t) for t in aes_timings]
        # Extract TX timings
        tx_timings = re.findall(r"TX time: (\d+)", file)
        tx_timings = [int(t) for t in tx_timings]

        # Create a DataFrame
        df = pl.DataFrame(
            {
                "Sensor": sensor_timings,
                "AES Encryption": aes_timings,
                "TX": tx_timings,
            }
        )
        return df

    """
    Takes a summary string and returns a list of values
    Returns a list of values
    """
    def extract_values(self, summary: str, samples=1) -> dict:
        # extract the values
        match = re.findall(r":\s+(\d+)", summary)
        # return the values
        return dict({
            state: int(x) for state, x in zip(["Total"] + list(Measurements.States.keys()), match)
        })

        # return [int(x) / samples if i > 0 else int(x) for i, x in enumerate(match)]

    """
    Takes a list of values and returns a dictionary with the state values
    Returns a dictionary with the state values
    """
    def calculate_state_energy(self, values: dict) -> dict:
        power_states = ["state_time", "avg_current_mA", "charge_mC", "charge_mAh", "power_mW", "energy_mJ"]
        # calculate the values
        total_time = values.pop("Total")
        states_values = {
            f"{state}": {
                "state_time": state_time,
                "avg_current_mA": (state_time * state_current) / total_time,
                "charge_mC": (state_time * state_current) / Measurements.TICKS_PER_SECOND,
                "charge_mAh": (state_time * state_current) / Measurements.TICKS_PER_SECOND / 3600,
                "power_mW": (state_time * state_current) / total_time * Measurements.VOLTAGE,
                "energy_mJ": (state_time * state_current) / Measurements.TICKS_PER_SECOND * Measurements.VOLTAGE
            } for state_time, state_current, state in zip(values.values(), Measurements.States.values(), Measurements.States.keys())
        }

        # accumulate the values
        states_values_accummulated = {
            f"{state}": sum([x[f"{state}"] for x in states_values.values()])
            for state in power_states
        }
        
        # return the values
        return states_values_accummulated

    """
    Takes a path to a file containing the energy measurements
    Returns a DataFrame with the energy measurements
    """
    def extract_summaries(self, energest_output: str) -> list:
        # open the file to apply regex on
        with open(energest_output, "r") as f:
            # read the file
            file = f.read()

        reg_suf = r".*?Radio total\s+:\s+\d+/\s+\d+ \(\d+ permil\)"

        match self.energy_output_type:
            case OutputType.ENERGY:
                # extract the sensor energy summaries
                sensor_summaries = re.findall(r"Sensor energy:" + reg_suf, file, re.DOTALL)
                # extract the AES energy summaries
                aes_summaries = re.findall(r"AES energy:" + reg_suf, file, re.DOTALL)
                # extract the TX energy summaries
                tx_summaries = re.findall(r"TX energy:" + reg_suf, file, re.DOTALL)

                # return the summaries
                return dict({
                    "Sensor energy": sensor_summaries,
                    "AES energy": aes_summaries,
                    "TX energy": tx_summaries
                })
            case OutputType.ENERGY_FULL:
                # extract the sensor energy summaries
                summaries_full = re.findall(r"--- Period summary" + reg_suf, file, re.DOTALL)

                # return the summaries
                return dict({
                    "full energy": summaries_full
                })
            case _:
                raise Exception("Invalid energy output type")
