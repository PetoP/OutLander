import matplotlib as mpl
import matplotlib.pyplot as plt
from matplotlib.path import Path
import matplotlib.ticker as plticker
import matplotlib.patches as patches
import matplotlib.dates as mdates
import numpy as np
import sys
import datetime

ocs = ("/run/media/peter/WD/ZIK/diplomovka/snimky/snimkova_statistika/LC8_16_04_2016_ocs.csv",
       "/run/media/peter/WD/ZIK/diplomovka/snimky/snimkova_statistika/LC8_19_06_2016_ocs.csv",
       "/run/media/peter/WD/ZIK/diplomovka/snimky/snimkova_statistika/LC8_21_07_2016_ocs.csv",
       "/run/media/peter/WD/ZIK/diplomovka/snimky/snimkova_statistika/S2A_02_07_2016:20_ocs.csv",
       "/run/media/peter/WD/ZIK/diplomovka/snimky/snimkova_statistika/S2A_22_06_2016:20_ocs.csv",
       "/run/media/peter/WD/ZIK/diplomovka/snimky/snimkova_statistika/S2A_23_05_2016:20_ocs.csv")

dates = ("16.4.2016", "19.6.2016", "21.7.2016",
         "2.7.2016", "22.6.2016", "23.5.2016")

satellites = ("L8", "L8", "L8", "S2", "S2", "S2")

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
    "text.latex.preamble"] = "\\usepackage{{mathpazo}} \n \\usepackage{{siunitx}} \n \\usepackage{{numprint}} \n \\npthousandsep{{\,}} \n \\npdecimalsign{{.}} \n \\npthousandthpartsep{{}} \n \\DeclareMathSymbol{.}{\\mathord}{letters}{\"3B}"
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
                self._bands_um = ((0.440, 0.538),  # B02
                                  (0.537, 0.582),  # B03
                                  (0.646, 0.684),  # B04
                                  (0.760, 0.908))  # B08
            elif resolution == 20:
                self._bands_um = ((0.440, 0.538),  # B02
                                  (0.537, 0.582),  # B03
                                  (0.646, 0.684),  # B04
                                  (0.694, 0.713),  # B05
                                  (0.731, 0.749),  # B06
                                  (0.769, 0.797),  # B07
                                  # B08b (B8A according to product xml)
                                  (0.848, 0.881),
                                  (1.539, 1.682),  # B11
                                  (2.078, 2.320))  # B12
            elif resolution == 60:
                self._bands_um = ((0.430, 0.457),  # B01
                                  (0.440, 0.538),  # B02
                                  (0.537, 0.582),  # B03
                                  (0.646, 0.684),  # B04
                                  (0.694, 0.713),  # B05
                                  (0.731, 0.749),  # B06
                                  (0.769, 0.797),  # B07
                                  # B08b (B8A according to product xml)
                                  (0.848, 0.881),
                                  (0.932, 0.958),  # B09
                                  # (1.337, 1.412),# B10 !!!
                                  (1.539, 1.682),  # B11
                                  (2.078, 2.320))  # B12

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

        if self._satellite == "L8":
            self._bands = np.arange(
                1, (data.shape[1] - leftCols) / variables + 1, dtype=int)
        elif self._satellite == "S2":
            if self._resolution == 10:
                self._bands = np.array([2, 3, 4, 8], dtype=int)
            elif self._resolution == 20:
                self._bands = np.array([2, 3, 4, 5, 6, 7, 8, 11, 12])
            elif self._resolution == 60:
                self._bands = np.array([1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12])

        self._avgs = np.empty((len(self._classes), len(self._bands)))
        self._stdevs = np.empty((len(self._classes), len(self._bands)))

        for i in range(len(self._classes)):
            for j in range(len(self._bands)):
                self._avgs[i][j] = data[i][leftCols + j * variables]
                self._stdevs[i][j] = data[i][leftCols + j * variables + 1]

    def spectPlot(self, specClass=None):
        spectPlot(self, specClass=specClass)

    def pixelCountPlot(self):
        fig = plt.figure(figsize=(15, 10))
        ax = fig.add_subplot(111)
        ax.yaxis.set_ticks_position('left')
        ax.xaxis.set_ticks_position('bottom')
        ticksFormatter = mpl.ticker.FuncFormatter(self.tickFormat)
        ax.yaxis.set_major_formatter(ticksFormatter)
        ax.xaxis.set_major_formatter(ticksFormatter)

        width = 1
        ax.bar([i - 0.5 * width for i in range(len(self._classes))],
               self._counts,
               zorder=5,
               color=lineColors,
               lw=0,
               width=width)

        plt.xlabel("Spectral class", size=14)
        plt.ylabel("Pixel count", size=14)
        plt.xticks(range(len(self._classes)), [
                   r"$\num{{{}}}$".format(i) for i in self._classes])
        plt.xlim(0 - 0.7 * width, len(self._classes) - 1 + 0.7 * width)

        ax.grid()
        plt.show()

    def tickFormat(self, x, pos):
        if isinstance(x, str):
            return x
        if isinstance(x, (int, float)):
            if x % 1 == 0:
                x = int(x)
            return r"$\numprint{{{}}}$".format(x)

    def getDate(self):
        return self._date

    def getClasses(self):
        return self._classes

    def getAverages(self, spectClass=None):
        if spectClass is None:
            return self._avgs
        else:
            assert spectClass in self._classes
            return self._avgs[np.where(self._classes == spectClass)[0][0]]

    def getAverage(self, spectClass, band):
        assert spectClass in self._classes
        assert band in self._bands
        return self._avgs[np.where(self._classes == spectClass)[0][0]][np.where(self._bands == band)][0]

    def getStDevs(self, spectClass=None):
        if spectClass is None:
            return self._stdevs
        else:
            assert spectClass in self._classes
            return self._stdevs[np.where(self._classes == spectClass)[0][0]]

    def getStdev(self, spectClass, band):
        assert spectClass in self._classes
        assert band in self._bands
        return self._stdevs[np.where(self._classes == spectClass)[0][0]][np.where(self._bands == band)][0]

    def getBands(self):
        return self._bands

    def getBandsUm(self):
        return self._bands_um

    def getSatellite(self):
        return self._satellite


def spectPlot(cs, cs2=None, specClass=None):
    # figure creation
    fig = plt.figure(figsize=(15, 10))
    ax = fig.add_subplot(111)

    assert isinstance(cs, ClassStatistics)

    if cs2 is not None:
        assert isinstance(cs2, ClassStatistics)

    if cs2 is None:
        if specClass is None:
            ytlim = np.amax(cs.getAverages()) / 10000 * 1.1
        else:
            ytlim = np.amax(cs.getAverages(specClass)) / 10000 * 1.1
    else:
        if specClass is None:
            ytlim = np.amax(cs.getAverages()) / 10000 * 1.1 if np.amax(cs.getAverages()
                                                                       ) > np.amax(cs2.getAverages()) else np.amax(cs2.getAverages()) / 10000 * 1.1
        else:
            ytlim = np.amax(cs.getAverages(specClass)) / 10000 * 1.1 if np.amax(cs.getAverages(specClass)
                                                                                ) > np.amax(cs2.getAverages(specClass)) else np.amax(cs2.getAverages(specClass)) / 10000 * 1.1

    ax.set_ylim(bottom=0, top=ytlim)
    # plotting each spectral class
    for spectClass in range(len(cs.getClasses())):
        if specClass is None or specClass == cs.getClasses()[spectClass]:
            x = [(x[0] + x[1]) / 2 for x in cs.getBandsUm()]
            ax.plot(x,
                    cs.getAverages(cs.getClasses()[spectClass]) / 10000,
                    color=lineColors[cs.getClasses()[spectClass - 1]],
                    lw=1.5,
                    zorder=5,
                    label=cs.getClasses()[spectClass])
            ax.errorbar(x,
                        cs.getAverages(cs.getClasses()[spectClass]) / 10000,
                        yerr=cs.getStDevs(cs.getClasses()[spectClass]) / 10000,
                        color=lineColors[cs.getClasses()[spectClass - 1]],
                        lw=0.5,
                        zorder=5)
            if cs2 is not None:
                x = [(x[0] + x[1]) / 2 for x in cs2.getBandsUm()]
                ax.plot(x,
                        cs2.getAverages(cs2.getClasses()[spectClass]) / 10000,
                        color=lineColors[cs2.getClasses()[spectClass]],
                        lw=1.5,
                        ls=":",
                        zorder=5)
                ax.errorbar(x,
                            cs2.getAverages(cs2.getClasses(
                                [spectClass])) / 10000,
                            yerr=cs.getStDevs(cs2.getClasses()[spectClass]) / 10000,
                            color=lineColors[cs2.getClasses([spectClass])],
                            lw=0.5,
                            ls="",
                            zorder=5)

    ymin, ymax = ax.get_ylim()

    # plotting band ranges
    if cs2 is None or cs.getSatellite() == cs2.getSatellite():
        codes = (Path.MOVETO,
                 Path.LINETO,
                 Path.LINETO,
                 Path.LINETO,
                 Path.CLOSEPOLY)
        for band in cs.getBandsUm():
            verts = ((band[0], ymin),
                     (band[0], ymax),
                     (band[1], ymax),
                     (band[1], ymin),
                     (band[0], ymin))
            path = Path(verts, codes)
            patch = patches.PathPatch(path, color="#E0E0E0")
            ax.add_patch(patch)
        # drawing band titles
        for i in range(len(cs.getBands())):
            bx = (cs.getBandsUm()[i][0] + cs.getBandsUm()[i][1]) / 2
            by = ymax
            plt.text(bx, by, "B{}".format(
                cs.getBands()[i]), ha="center", va="bottom")

    # getiing bounding x values
    dmin = min([i[0] for i in cs.getBandsUm()])
    dmax = max([i[1] for i in cs.getBandsUm()])
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
    ax.yaxis.set_ticks_position('left')
    ax.xaxis.set_ticks_position('bottom')
    ticksFormatter = mpl.ticker.ScalarFormatter(useLocale=True)
    ticksFormatter2 = mpl.ticker.FuncFormatter(cs.tickFormat)
    ax.yaxis.set_major_formatter(ticksFormatter)
    ax.xaxis.set_major_formatter(ticksFormatter2)

    # labels
    plt.xlabel(r"$\lambda \left(\mu m\right)$", size=14)
    plt.ylabel("Reflectance", size=14)

    # legend
    plt.legend(fontsize=10, ncol=2, loc=0)

    # setting x limits
    plt.xlim(dmin - 2 * dperc, dmax + 2 * dperc)
    plt.show()


class ClassStatisticsContainer:

    def __init__(self, classStatistics):
        assert isinstance(classStatistics, (list, tuple))
        self._classStatistics = list(classStatistics)
        self._sort()

    def _sort(self):
        self._classStatistics = sorted(self._classStatistics, key=lambda x: x.getDate())

    def addCS(self, classStatistics):
        assert isinstance(classStatistics, ClassStatistics)
        self._classStatistics.append(ClassStatistics)
        self._sort()

    def getEntries(self):
        return self._classStatistics

    def __getitem__(self, pos):
        assert isinstance(pos, int)
        return self._classStatistics[pos]

    def timePlot(self, satellite, satelliteBand, spectClass=None, plotStdev=False):
        """
        Plots time plot of classStatistics inside container. Currently only S2 scenes with 20m res are supported.

        satellite -- {"L8", "S2", "all"}
        satelliteBand -- befines which spectral band is going to be plotted. If satellite is "L8", than number of band
                         refers L8 bands, the same with S2. If satellite is "all", number belongs to L8 band numbers.
        spectralClass -- plot only defined spectral class
        """
        assert satellite in ("L8", "S2", "all")
        if satellite == "L8" or satellite == "all":
            bands = range(1, 8)
        elif satellite == "S2":
            bands = (2, 3, 4, 5, 6, 7, 8, 11, 12)
        assert satelliteBand in bands

        spectralClasses = []
        for scene in self._classStatistics:
            [spectralClasses.append(i) for i in scene.getClasses() if i not in spectralClasses]

        # figure creation
        fig = plt.figure(figsize=(15, 10))
        ax = fig.add_subplot(111)

        l8band2s2 = {
            1: 1,
            2: 2,
            3: 3,
            4: 4,
            5: 8,
            6: 11,
            7: 12
        }

        sceneDates = []
        sceneSatellites = []
        sceneBands = []
        save = True
        for spectralClass in spectralClasses:
            avgs = []
            if plotStdev:
                stdevs = []
            dates = []

            try:
                cont = spectralClass not in spectClass
            except:
                cont = spectralClass != spectClass

            if spectClass is not None and cont:
                continue

            for scene in self._classStatistics:

                sceneSatellite = scene.getSatellite()
                if satellite == "L8" or satellite == "S2":
                    if sceneSatellite != satellite:
                        continue

                if sceneSatellite == "L8" or (sceneSatellite == "S2" and satellite == "S2"):
                    sceneBand = satelliteBand
                else:
                    sceneBand = l8band2s2[satelliteBand]

                avgs.append(scene.getAverage(spectralClass, sceneBand) / 10000)
                if plotStdev:
                    stdevs.append(scene.getStdev(spectralClass, sceneBand) / 10000)

                date = scene.getDate()
                dates.append(date)
                if save:
                    sceneDates.append(date)
                    sceneSatellites.append(sceneSatellite)
                    sceneBands.append(sceneBand)

            startDate = dates[0]
            endDate = dates[-1]

            ax.plot(dates,
                    avgs,
                    ".-",
                    markersize=10,
                    color = lineColors[spectralClasses.index(spectralClass)],
                    zorder=5)
            if plotStdev:
                ax.errorbar(dates,
                            avgs,
                            yerr=avgs,
                            color = lineColors[spectralClasses.index(spectralClass)],
                            zorder=5)

            save = False

        # setting xlimits
        startDate = startDate.replace(day=1)
        if endDate.month < 12:
            endDate = endDate.replace(month=endDate.month + 1, day=1)
        else:
            endDate = endDate.replace(year=endDate.year + 1, month=1, day=1)

        ax.set_xlim(startDate, endDate)

        # plotting bands
        xmin, xmax = ax.get_xlim()
        xperc = (xmax - xmin) / 100
        ymin, ymax = ax.get_ylim()
        yperc = (ymax - ymin) / 100
        for i in range(len(sceneDates)):
            bx = sceneDates[i].toordinal()
            by = ymax if bx - sceneDates[i-1].toordinal() > 5 * xperc or i == 0 else ymax + yperc * 5
            ax.text(sceneDates[i],
                    by,
                    "{} B{}\n{}".format(sceneSatellites[i], sceneBands[i], sceneDates[i].strftime("%-d.%-m.%Y")),
                    ha="center",
                    va="bottom")
            ax.axvline(bx, zorder=1, ls="--", c="#E0E0E0")

        # setting tickers
        ymajstep = 0.05
        xmloc = mdates.MonthLocator()
        xiloc = mdates.DayLocator()
        ymloc = plticker.MultipleLocator(base=ymajstep)
        yiloc = plticker.MultipleLocator(base=ymajstep / 2)
        ax.xaxis.set_major_locator(xmloc)
        ax.xaxis.set_minor_locator(xiloc)
        ax.yaxis.set_major_locator(ymloc)
        ax.yaxis.set_minor_locator(yiloc)
        ax.yaxis.set_ticks_position('left')
        ax.xaxis.set_ticks_position('bottom')
        yticksFormatter = mpl.ticker.ScalarFormatter(useLocale=True)
        xticksFormatter = mdates.DateFormatter("%-d.%-m.%Y")
        ax.yaxis.set_major_formatter(yticksFormatter)
        ax.xaxis.set_major_formatter(xticksFormatter)

        plt.xlabel("Date", size=14)
        plt.ylabel("Reflectance", size=14)

        plt.show()


# if __name__ == "__main__":
if True:
    try:
        stats = []
        for i in range(len(ocs)):
            if satellites[i] == "L8":
                stats.append(ClassStatistics(ocs[i], dates[i], "L8"))
            elif satellites[i] == "S2":
                stats.append(ClassStatistics(ocs[i], dates[i], "S2", 20))
        csc = ClassStatisticsContainer(stats)
        csc.timePlot("all", 5)

    except KeyboardInterrupt:
        exit(2)
