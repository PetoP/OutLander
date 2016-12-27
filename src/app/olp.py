import matplotlib as mpl
import matplotlib.pyplot as plt
from matplotlib.path import Path
import matplotlib.ticker as plticker
import matplotlib.patches as patches
import numpy as np
import sys
import datetime

ocs = ("/run/media/peter/WD/ZIK/diplomovka/snimky/snimkova_statistika/LC8_21_07_2016_ocs.csv")
dates = ("21.07.2016")

mpl.rcParams["xtick.direction"] = "inout"
mpl.rcParams["ytick.direction"] = "inout"
mpl.rcParams["xtick.major.size"] = 8
mpl.rcParams["xtick.minor.size"] = 4
mpl.rcParams["ytick.major.size"] = 8
mpl.rcParams["ytick.minor.size"] = 4
mpl.rcParams["figure.facecolor"] = "white"
mpl.rcParams["grid.linestyle"] = "-"
mpl.rcParams["grid.color"] = "#A0A0A0"
mpl.rcParams["grid.linewidth"] = 0.5
lineColors = ["g", "b", "r", "#800000", "#C3C3C3", "#000000", "#49CC51", "#33913A",
              "#E4EB13", "#ACB10E", "#87A61C", "#FFFF00", "#D59739", "#9CCFCE", "#9C9A31"]
mpl.rcParams[
    "text.latex.preamble"] = "\\usepackage{{mathpazo}} \n \\usepackage{{siunitx}}"
mpl.rcParams['text.usetex'] = True
mpl.rcParams['text.latex.unicode'] = True


class ClassStatistics:

    def __init__(self, filename, date, satellite, resolution=None):
        assert isinstance(date, (str, datetime.date))

        # CSV reading
        try:
            data = np.genfromtxt(filename, skip_header=1, delimiter=",")
        except:
            sys.stderr.write("Unable to open file {}!\n".format(filename))
            exit(1)

        # date parsing
        if isinstance(date, str):
            try:
                dateSplited = date.split(".")
                dateSplited = [int(i) for i in dateSplited]
                self._date = datetime.date(
                    dateSplited[2], dateSplited[1], dateSplited[0])
            except:
                sys.stderr.write(
                    "Unable to convert string \"{}\" to datetime.date object!\n".format(date))
                exit(1)
        else:
            self._date = date

        assert satellite in ("L8", "S2")
        self._satellite = satellite

        # setting wavelenghts for bands
        if satellite == "L8":
            self._resolution = 30
            self._bands_um = ((0.433, 0.453),  # B01
                              (0.450, 0.515),  # B02
                              (0.525, 0.600),  # B03
                              (0.630, 0.680),  # B04
                              (0.845, 0.885),  # B05
                              (1.560, 1.660),  # B06
                              (2.100, 2.300))  # B07
        elif satellite == "S2":
            assert resolution in (10, 20, 60)
            self._resolution = resolution
            if resolution == 10:
                self._bands_um = (0.490,  # B02
                                  0.560,  # B03
                                  0.665,  # B04
                                  0.842)  # B08
            elif resolution == 20:
                self._bands_um = (0.490,  # B02
                                  0.560,  # B03
                                  0.665,  # B04
                                  0.705,  # B05
                                  0.740,  # B06
                                  0.783,  # B07
                                  0.865,  # B08b (B8A according to product xml)
                                  1.610,  # B11
                                  2.190)  # B12
            elif resolution == 60:
                self._bands_um = (0.443,  # B01
                                  0.490,  # B02
                                  0.560,  # B03
                                  0.665,  # B04
                                  0.705,  # B05
                                  0.740,  # B06
                                  0.783,  # B07
                                  0.865,  # B08b (B8A according to product xml)
                                  0.945,  # B09
                                  1.610,  # B11
                                  2.190)  # B12

        # number of statistical variables per band
        variables = 2

        # number of columns from left, which are not statistical variables per
        # band
        leftCols = 2

        self._classes = np.empty((len(data)), dtype=int)
        self._counts = np.empty((len(data)), dtype=int)
        for i in range(len(data)):
            self._classes[i] = int(data[i][0])
            self._counts[i] = int(data[i][1])

        self._bands = np.arange(
            1, (data.shape[1] - leftCols) / variables + 1, dtype=int)

        self._avgs = np.empty((len(self._classes), len(self._bands)))
        self._stdevs = np.empty((len(self._classes), len(self._bands)))
        for i in range(len(self._classes)):
            for j in range(len(self._bands)):
                self._avgs[i][j] = data[i][leftCols + j * variables]
                self._stdevs[i][j] = data[i][leftCols + j * variables + 1]

    def plot(self):

        # figure creation
        fig = plt.figure(figsize=(15, 10))
        ax = fig.add_subplot(111)
        # ax.set_axis_bgcolor("#FAFAFA")

        # plotting each spectral class
        for spectClass in range(len(self._classes)):
            if self._satellite == "L8":
                x = [(x[0] + x[1]) / 2 for x in self._bands_um]
            ax.plot(x,
                    self._avgs[spectClass] / 10000,
                    color=lineColors[self._classes[spectClass - 1]],
                    lw=1.5,
                    zorder=5,
                    label=x[0])
            ax.errorbar(x,
                        self._avgs[spectClass] / 10000,
                        yerr = self._stdevs[spectClass] / 10000,
                        color=lineColors[self._classes[spectClass - 1]],
                        lw=0.5,
                        zorder=5)

        ymin, ymax = ax.get_ylim()
        # plotting band ranges
        if self._satellite == "L8":
            codes = (Path.MOVETO,
                     Path.LINETO,
                     Path.LINETO,
                     Path.LINETO,
                     Path.CLOSEPOLY)
            for band in self._bands_um:
                verts = ((band[0], ymin),
                         (band[0], ymax),
                         (band[1], ymax),
                         (band[1], ymin),
                         (band[0], ymin))
                path = Path(verts, codes)
                patch = patches.PathPatch(path, color="#E0E0E0")
                ax.add_patch(patch)

        # getiing bounding x values and drawing band titles
        if self._satellite == "L8":
            dmin = min([i[0] for i in self._bands_um])
            dmax = max([i[1] for i in self._bands_um])
            dperc = (dmax - dmin) / 100
            for i in range(len(self._bands)):
                bx = (self._bands_um[i][0] + self._bands_um[i][1]) / 2
                by = ymax
                plt.text(bx, by, "B{}".format(
                    self._bands[i]), ha="center", va="bottom")
        elif self._satellite == "S2":
            dmin = min([i for i in self._bands_um])
            dmax = max([i for i in self._bands_um])

        dperc = (dmax - dmin) / 100

        # setting tickers
        xmajstep = 0.25 / 2
        ymajstep = 0.05
        xmloc = plticker.MultipleLocator(base=xmajstep)
        xiloc = plticker.MultipleLocator(base=xmajstep / 2)
        ymloc = plticker.MultipleLocator(base=ymajstep)
        yiloc = plticker.MultipleLocator(base=ymajstep / 2)
        ax.xaxis.set_major_locator(xmloc)
        ax.xaxis.set_minor_locator(xiloc)
        ax.yaxis.set_major_locator(ymloc)
        ax.yaxis.set_minor_locator(yiloc)

        # grid
        # plt.grid()
        # plt.grid(which="minor", lw=0.2, alpha=0.5)

        # labels
        plt.xlabel(r"$\lambda \left(\mu m\right)$", size=14)
        plt.ylabel("Reflectance", size=14)

        # legend
        plt.legend(fontsize=10, ncol=2)

        # setting x limits
        plt.xlim(dmin - 2 * dperc, dmax + 2 * dperc)
        plt.show()


# if __name__ == "__main__":
if True:
    try:
        OCS = ClassStatistics(ocs, dates, "L8")
    except KeyboardInterrupt:
        exit(2)
