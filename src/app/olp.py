import matplotlib as mpl
import matplotlib.pyplot as plt
from matplotlib.path import Path
import matplotlib.ticker as plticker
import matplotlib.patches as patches
import matplotlib.dates as mdates
import numpy as np
import sys
import datetime

ocs = ("/run/media/peter/WD/ZIK/diplomovka/snimky/snimkova_statistika/LC8_16_04_16_ocs.csv",
       "/run/media/peter/WD/ZIK/diplomovka/snimky/snimkova_statistika/LC8_19_06_16_ocs.csv",
       "/run/media/peter/WD/ZIK/diplomovka/snimky/snimkova_statistika/S2A_02_07_16_ocs.csv")

oos = ("/run/media/peter/WD/ZIK/diplomovka/snimky/snimkova_statistika/LC8_16_04_16_oos.csv",
       "/run/media/peter/WD/ZIK/diplomovka/snimky/snimkova_statistika/LC8_19_06_16_oos.csv",
       "/run/media/peter/WD/ZIK/diplomovka/snimky/snimkova_statistika/S2A_02_07_16_oos.csv")

dates = ("16.4.2016", "19.6.2016", "2.7.2016")

satellites = ("L8", "L8", "S2")

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
# lineColors = ["g", "b", "r", "#800000", "#C3C3C3", "#000000", "#49CC51", "#33913A",
#               "#E4EB13", "#ACB10E", "#87A61C", "#FFFF00", "#D59739", "#9CCFCE", "#9C9A31"]
lineColors = ("#006000",  # les
              "#008cfe",  # voda
              "#b70000",  # umelé plochy
              "#83d183",  # nízke porasty, trávy
              "#c9b590",  # holá zem
              "#ebeaf4",  # oblačnosť
              "#000000",  # tiene
              "#7eae54",  # obilniny zelené
              "#e4eb13",  # obilniny svetlé
              "#acb10e",  # kukurica
              "#b7b700",  # slnečnica
              "#d59739",  # repa
              "#517c2c",  # repka oziminná zelená
              "#adb00d")  # repka oziminná svetlá
mpl.rcParams[
    "text.latex.preamble"] = "\\usepackage{{mathpazo}} \n \\usepackage{{siunitx}} \n \\usepackage{{numprint}} \n \\npthousandsep{{\,}} \n \\npdecimalsign{{.}} \n \\npthousandthpartsep{{}} \n \\DeclareMathSymbol{.}{\\mathord}{letters}{\"3B}"
mpl.rcParams['text.usetex'] = True
mpl.rcParams['text.latex.unicode'] = True


class TrSitesStatistics:

    def __init__(self, ocsfilename, oosfilename, date, satellite, resolution=None):
        assert isinstance(date, (str, datetime.date))

        # ocs CSV reading
        try:
            ocsdata = np.genfromtxt(ocsfilename, skip_header=1, delimiter=",")
        except:
            sys.stderr.write("Unable to open file {}!\n".format(ocsfilename))
            exit(1)

        # oos CSV reading
        try:
            oosdata = np.genfromtxt(oosfilename, skip_header=1, delimiter=",")
        except:
            sys.stderr.write("Unable to open file {}!\n".format(oosfilename))
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
        ocsLeftCols = 2
        oosLeftCols = 3

        self._classes = np.empty((len(ocsdata)), dtype=int)
        self._counts = np.empty((len(ocsdata)), dtype=int)
        for i in range(len(ocsdata)):
            self._classes[i] = int(ocsdata[i][0])
            self._counts[i] = int(ocsdata[i][1])

        if self._satellite == "L8":
            self._bands = np.arange(
                1, (ocsdata.shape[1] - ocsLeftCols) / variables + 1, dtype=int)
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
                self._avgs[i][j] = ocsdata[i][ocsLeftCols + j * variables]
                self._stdevs[i][j] = ocsdata[i][ocsLeftCols + j * variables + 1]

        self._oos = {}
        for specClass in self._classes:
            self._oos[specClass] = {}
            for band in range(len(self._bands)):
                self._oos[specClass][band] = []
                for trSite in oosdata:
                    if trSite[1] == specClass:
                        self._oos[specClass][band].append(trSite[oosLeftCols + band * variables])

        for key in self._oos.keys():
            for key2 in self._oos[key].keys():
                self._oos[key][key2] = np.array(self._oos[key][key2], dtype=float)


    def spectPlot(self, specClass=None, stdev=False):
        spectPlot(self, specClass=specClass, stdev=stdev)

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

    def getTrSitesAverages(self, spectClass, band):
        assert spectClass in self._classes
        assert band in self._bands
        return self._oos[spectClass][np.where(self._bands == band)[0][0]]

    def scatterPlot(self, bandX, bandY, spectClasses=None):
        assert bandX in self._bands
        assert bandY in self._bands

        if spectClasses is not None:
            assert isinstance(spectClasses, (int, tuple, list))
            if isinstance(spectClasses, int):
                assert spectClasses in self._classes
            else:
                for i in spectClasses:
                    assert i in self._classes

        # figure creation
        fig = plt.figure(figsize=(15, 10))
        ax = fig.add_subplot(111)

        if spectClasses is None:
            classes = self._classes
        else:
            classes = spectClasses

        # setting limits
        ax.set_xlim(0, 0.8)
        ax.set_ylim(0, 0.8)

        # setting tickers
        majStep = 0.1

        mloc = plticker.MultipleLocator(base=majStep)
        iloc = plticker.MultipleLocator(base=majStep / 2)
        ax.xaxis.set_major_locator(mloc)
        ax.xaxis.set_minor_locator(iloc)
        ax.yaxis.set_major_locator(mloc)
        ax.yaxis.set_minor_locator(iloc)

        # plotting
        for spcl in classes:
            ax.scatter(self.getTrSitesAverages(spcl, bandX) / 10000,
                       self.getTrSitesAverages(spcl, bandY) / 10000,
                       s=5,
                       color=lineColors[spcl - 1],
                       label=spcl)

        # labels
        plt.xlabel("Reflectnace B{}".format(bandX), size=14)
        plt.ylabel("Reflectnace B{}".format(bandY), size=14)

        # legend
        plt.legend(fontsize=10, ncol=2, loc=0, scatterpoints=1)

        plt.show()


def spectPlot(cs, cs2=None, specClass=None, stdev=False):
    # figure creation
    fig = plt.figure(figsize=(15, 10))
    ax = fig.add_subplot(111)

    assert isinstance(cs, TrSitesStatistics)

    if cs2 is not None:
        assert isinstance(cs2, TrSitesStatistics)

    if cs2 is None:
        ytlim = np.amax(cs.getAverages()) / 10000 * 1.1
    else:
        ytlim = np.amax(cs.getAverages()) / 10000 * 1.1 if np.amax(cs.getAverages()
                                                                       ) > np.amax(cs2.getAverages()) else np.amax(cs2.getAverages()) / 10000 * 1.1

    if specClass is not None:
        assert isinstance(specClass, (int, tuple, list))
        if isinstance(specClass, int):
            assert specClass in cs.getClasses()
            if cs2 is not None:
                assert specClass in cs2.getClasses()
        if isinstance(specClass, (tuple, list)):
            for i in specClass:
                assert i in cs.getClasses()
                if cs2 is not None:
                    assert specClass in cs2.getClasses()

    ax.set_ylim(bottom=0, top=ytlim)
    # plotting each spectral class
    for spectClass in cs.getClasses():

        kresli = True
        if specClass is not None:
            kresli = False
            try:
                if spectClass == specClass:
                    kresli = True
            except:
                if spectClass in specClass:
                    kresli = True

        if kresli:
            x = [(x[0] + x[1]) / 2 for x in cs.getBandsUm()]
            ax.plot(x,
                    cs.getAverages(spectClass) / 10000,
                    color=lineColors[spectClass - 1],
                    lw=1.5,
                    zorder=5,
                    label=spectClass)
            if stdev:
                ax.errorbar(x,
                            cs.getAverages(spectClass) / 10000,
                            yerr=cs.getStDevs(spectClass) / 10000,
                            color=lineColors[spectClass - 1],
                            lw=0.5,
                            zorder=5)
                if cs2 is not None:
                    x = [(x[0] + x[1]) / 2 for x in cs2.getBandsUm()]
                    ax.plot(x,
                            cs2.getAverages(spectClass) / 10000,
                            color=lineColors[spectClass - 1],
                            lw=1.5,
                            ls=":",
                            zorder=5)
                    if stdev:
                        ax.errorbar(x,
                                    cs2.getAverages(spectClass) / 10000,
                                    yerr=cs.getStDevs(spectClass) / 10000,
                                    color=lineColors[spectClass - 1],
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


class TrSitesStatisticsContainer:

    def __init__(self, classStatistics):
        assert isinstance(classStatistics, (list, tuple))
        self._classStatistics = list(classStatistics)
        self._sort()

    def _sort(self):
        self._classStatistics = sorted(
            self._classStatistics, key=lambda x: x.getDate())

    def addCS(self, classStatistics):
        assert isinstance(classStatistics, TrSitesStatistics)
        self._classStatistics.append(TrSitesStatistics)
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
            [spectralClasses.append(
                i) for i in scene.getClasses() if i not in spectralClasses]

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
                    stdevs.append(scene.getStdev(
                        spectralClass, sceneBand) / 10000)

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
                    color=lineColors[spectralClasses.index(spectralClass)],
                    zorder=5)
            if plotStdev:
                ax.errorbar(dates,
                            avgs,
                            yerr=avgs,
                            color=lineColors[
                                spectralClasses.index(spectralClass)],
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
            by = ymax if bx - \
                sceneDates[i - 1].toordinal() > 5 * xperc or i == 0 else ymax + yperc * 5
            ax.text(sceneDates[i],
                    by,
                    "{} B{}\n{}".format(sceneSatellites[i], sceneBands[
                                        i], sceneDates[i].strftime("%-d.%-m.%Y")),
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
                stats.append(TrSitesStatistics(ocs[i], oos[i], dates[i], "L8"))
            elif satellites[i] == "S2":
                stats.append(TrSitesStatistics(
                    ocs[i], oos[i], dates[i], "S2", 20))
        csc = TrSitesStatisticsContainer(stats)
        # csc.timePlot("all", 5)
        # csc[0].scatterPlot(5, 6)
        # csc[1].spectPlot()

    except KeyboardInterrupt:
        exit(2)
