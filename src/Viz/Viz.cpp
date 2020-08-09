#include "pch.h"
#include "../OS/OS.h"
#include "../Path.h"
#include "Viz.h"
#include "../Text/StringUtils.h"

using namespace std;

namespace bmhpal {
namespace viz {

std::string Dir() {
	return path::Join(os::UserHomeDir(), "viz");
}

std::string Filename(std::string name) {
	return path::Join(Dir(), name);
}

void DumpTimeSeriesD(const std::vector<std::pair<double, double>>& ts, Flags flags, std::string title) {
	auto fnData = Filename(title + ".csv");
	auto fnPlot = Filename(title + ".py");

	bool logY = !!(flags & Flags::LogY);

	string str;
	str += tsf::fmt("%v,%v\n", "x", "y");

	for (size_t i = 0; i < ts.size(); i++) {
		double y = ts[i].second;
		str += tsf::fmt("%.6f,%.6f\n", ts[i].first, y);
	}
	os::MkDir(Dir(), false);
	auto err = os::WriteFile(fnData, str);
	BMHPAL_ASSERT(err.OK());

	//	auto plot = tsf::fmt(R"(set datafile separator ","
	//set ylabel "%v"
	//%v
	//plot "%v" using 1:2 with lines lw 1 title "%v"
	//	)",
	//	                     name, logY ? "set logscale y" : "", fnData, name);

	string plot = R"(
import matplotlib.pyplot as plt
import csv

x = []
y = []
xlabel = ''
ylabel = ''

with open('FILENAME', 'r') as csvfile:
    plots = csv.reader(csvfile, delimiter=',')
    irow = 0
    for row in plots:
        if irow == 0:
            xlabel = row[0]
            ylabel = row[1]
        else:
            x.append(float(row[0]))
            y.append(float(row[1]))
        irow += 1

plt.plot(x, y, label='series', marker='o', markersize=4)
plt.xlabel(xlabel)
plt.ylabel(ylabel)
plt.title('TITLE')
plt.legend()
plt.yscale('log')
plt.grid(True)
plt.show()
	)";
	plot        = strings::Replace(plot, string("FILENAME"), fnData);
	plot        = strings::Replace(plot, string("TITLE"), title);

	err = os::WriteFile(fnPlot, plot);
	BMHPAL_ASSERT(err.OK());
	//tsf::print("gnuplot -p %v\n", fnPlot);
}

} // namespace viz
} // namespace bmhpal