/*
 * Copyright (c) 2009 Sean C. Rhea (srhea@srhea.net)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "RideSummaryWindow.h"
#include "MainWindow.h"
#include "RideFile.h"
#include "RideItem.h"
#include "RideMetric.h"
#include "Settings.h"
#include "TimeUtils.h"
#include "Units.h"
#include "Zones.h"
#include "MetricAggregator.h"
#include "DBAccess.h"
#include <QtGui>
#include <QtXml/QtXml>
#include <assert.h>
#include <math.h>

RideSummaryWindow::RideSummaryWindow(MainWindow *mainWindow) :
    GcWindow(mainWindow), mainWindow(mainWindow)
{
    setInstanceName("Ride Summary Window");
    setControls(NULL);
    setRideItem(NULL);

    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->setSpacing(0);
    vlayout->setContentsMargins(10,10,10,10);
    rideSummary = new QWebView(this);
    rideSummary->setContentsMargins(0,0,0,0);
    rideSummary->page()->view()->setContentsMargins(0,0,0,0);
    rideSummary->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    rideSummary->setAcceptDrops(false);

    QFont defaultFont; // mainwindow sets up the defaults.. we need to apply
    rideSummary->settings()->setFontSize(QWebSettings::DefaultFontSize, defaultFont.pointSize()+1);
    rideSummary->settings()->setFontFamily(QWebSettings::StandardFont, defaultFont.family());

    vlayout->addWidget(rideSummary);

    connect(this, SIGNAL(rideItemChanged(RideItem*)), this, SLOT(rideItemChanged()));
    connect(mainWindow, SIGNAL(zonesChanged()), this, SLOT(refresh()));
    connect(mainWindow, SIGNAL(intervalsChanged()), this, SLOT(refresh()));
    setLayout(vlayout);
}

void
RideSummaryWindow::rideSelected()
{
    refresh();
}

void
RideSummaryWindow::rideItemChanged()
{
    // disconnect from previous
    static QPointer<RideItem> _connected = NULL;
    if (_connected) {
        disconnect(_connected, SIGNAL(rideMetadataChanged()), this, SLOT(metadataChanged()));
    }
    _connected=myRideItem;
    if (_connected) { // in case it was set to null!
        connect (_connected, SIGNAL(rideMetadataChanged()), this, SLOT(metadataChanged()));
        // and now refresh
        refresh();
    }
}

void
RideSummaryWindow::metadataChanged()
{
    refresh();
}

void
RideSummaryWindow::refresh()
{
    // XXX: activeTab is never equaly to RideSummaryWindow right now because
    // it's wrapped in the summarySplitter in MainWindow.
    if (!myRideItem) {
	    rideSummary->page()->mainFrame()->setHtml("");
        return;
    }
    RideItem *rideItem = myRideItem;
    setSubTitle(rideItem->dateTime.toString(tr("dddd MMMM d, yyyy, h:mm AP")));
    rideSummary->page()->mainFrame()->setHtml(htmlSummary());
}

QString
RideSummaryWindow::htmlSummary() const
{
    QString summary("");

    RideItem *rideItem = myRideItem;
    RideFile *ride = rideItem->ride();

    // ridefile read errors?
    if (!ride) {
        summary = tr("<p>Couldn't read file \"");
        summary += rideItem->fileName + "\":";
        QListIterator<QString> i(rideItem->errors());
        while (i.hasNext())
            summary += "<br>" + i.next();
        return summary;
    }

    summary = ("<center><p><h3>" + tr("Device Type: ") + ride->deviceType() + "</h3>");

    QVariant unit = appsettings->value(this, GC_UNIT);

    summary += "<p>";

    bool metricUnits = (unit.toString() == "Metric");

    QStringList columnNames = QStringList() << "Totals" <<"Averages" <<"Maximums" <<"Metrics*";
    QStringList totalColumn = QStringList()
        << "workout_time"
        << "time_riding"
        << "total_distance"
        << "total_work"
        << "elevation_gain";

    QStringList averageColumn = QStringList()
        << "average_speed"
        << "average_power"
        << "average_hr"
        << "average_cad";

    QStringList maximumColumn = QStringList()
        << "max_speed"
        << "max_power"
        << "max_heartrate"
        << "max_cadence";

    QString s = appsettings->value(this, GC_SETTINGS_SUMMARY_METRICS, GC_SETTINGS_SUMMARY_METRICS_DEFAULT).toString();

    // in case they were set tand then unset
    if (s == "") s = GC_SETTINGS_SUMMARY_METRICS_DEFAULT;
    QStringList metricColumn = s.split(",");

    /* ORIGINAL HARDCODED (AND NOW DEFAULT METRICS)
    QStringList metricColumn = QStringList()
        << "skiba_xpower"
        << "skiba_relative_intensity"
        << "skiba_bike_score"
        << "daniels_points"
        << "daniels_equivalent_power"
        << "trimp_points"
        << "aerobic_decoupling";
    */

    QStringList timeInZones = QStringList()
        << "time_in_zone_L1"
        << "time_in_zone_L2"
        << "time_in_zone_L3"
        << "time_in_zone_L4"
        << "time_in_zone_L5"
        << "time_in_zone_L6"
        << "time_in_zone_L7"
        << "time_in_zone_L8"
        << "time_in_zone_L9"
        << "time_in_zone_L10";

    QStringList timeInZonesHR = QStringList()
        << "time_in_zone_H1"
        << "time_in_zone_H2"
        << "time_in_zone_H3"
        << "time_in_zone_H4"
        << "time_in_zone_H5"
        << "time_in_zone_H6"
        << "time_in_zone_H7"
        << "time_in_zone_H8";

    // Use pre-computed and saved metric values if the ride has not
    // been edited. Otherwise we need to re-compute every time.
    SummaryMetrics metrics;
    RideMetricFactory &factory = RideMetricFactory::instance();
    if (rideItem->isDirty()) {
        // make a list if the metrics we want computed
        // instead of calculating them all, just do the
        // ones we display
        QStringList worklist;
        worklist += totalColumn;
        worklist += averageColumn;
        worklist += maximumColumn;
        worklist += metricColumn;
        worklist += timeInZones;
        worklist += timeInZonesHR;

        // go calculate them then...
        QHash<QString, RideMetricPtr> computed = RideMetric::computeMetrics(mainWindow, ride, mainWindow->zones(), mainWindow->hrZones(), worklist);
        for(int i = 0; i < worklist.count(); ++i) {
            if (worklist[i] != "") {
                RideMetricPtr m = computed.value(worklist[i]);
                if (m) metrics.setForSymbol(worklist[i], m->value(true));
                else metrics.setForSymbol(worklist[i], 0.00);
            }
        }
    } else {

        // just use the metricDB versions, nice 'n fast
        metrics = mainWindow->metricDB->getRideMetrics(rideItem->fileName);
    }

    //
    // 3 top columns - total, average, maximums and metrics for entire ride
    //
    summary += "<table border=0 cellspacing=10><tr>";
    for (int i = 0; i < columnNames.count(); ++i) {
        summary += "<td align=\"center\" valign=\"top\" width=\"%1%\"><table>"
            "<tr><td align=\"center\" colspan=2><h3>%2</h3></td></tr>";
        summary = summary.arg(90 / columnNames.count());
        summary = summary.arg(columnNames[i]);

        QStringList metricsList;
        switch (i) {
            case 0: metricsList = totalColumn; break;
            case 1: metricsList = averageColumn; break;
            case 2: metricsList = maximumColumn; break;
            case 3: metricsList = metricColumn; break;
            default: assert(false);
        }
        for (int j = 0; j< metricsList.count(); ++j) {
            QString symbol = metricsList[j];

             if (symbol == "") continue;

             const RideMetric *m = factory.rideMetric(symbol);
             if (!m) break;
 
             // HTML table row
             QString s("<tr><td>%1%2:</td><td align=\"right\">%3</td></tr>");
 
             // Maximum Max and Average Average looks nasty, remove from name for display
             s = s.arg(m->name().replace(QRegExp(tr("^(Average|Max) ")), ""));
 
             // Add units (if needed)  and value (with right precision)
             if (m->units(metricUnits) == "seconds") {
                 s = s.arg(""); // no units
                 s = s.arg(time_to_string(metrics.getForSymbol(symbol)));
             } else {
                 if (m->units(metricUnits) != "") s = s.arg(" (" + m->units(metricUnits) + ")");
                 else s = s.arg("");
                 s = s.arg(metrics.getForSymbol(symbol) * (metricUnits ? 1 : m->conversion()), 0, 'f', m->precision());
            }

            summary += s;
        }
        summary += "</table></td>";
    }
    summary += "</tr></table>";

    //
    // Time In Zones
    //
    if (rideItem->numZones() > 0) {
        QVector<double> time_in_zone(rideItem->numZones());
        for (int i = 0; i < rideItem->numZones(); ++i)
            time_in_zone[i] = metrics.getForSymbol(timeInZones[i]);
        summary += tr("<h3>Power Zones</h3>");
        summary += mainWindow->zones()->summarize(rideItem->zoneRange(), time_in_zone);
    }

    //
    // Time In Zones HR
    //
    if (rideItem->numHrZones() > 0) {
        QVector<double> time_in_zone(rideItem->numHrZones());
        for (int i = 0; i < rideItem->numHrZones(); ++i)
            time_in_zone[i] = metrics.getForSymbol(timeInZonesHR[i]);
        summary += tr("<h3>Heart Rate Zones</h3>");
        summary += mainWindow->hrZones()->summarize(rideItem->hrZoneRange(), time_in_zone);
    }

    //
    // Interval Summary (recalculated on every refresh since they are not cached at present)
    //
    if (ride->intervals().size() > 0) {
        bool firstRow = true;
        QString s;
        if (appsettings->contains(GC_SETTINGS_INTERVAL_METRICS))
            s = appsettings->value(this, GC_SETTINGS_INTERVAL_METRICS).toString();
        else
            s = GC_SETTINGS_INTERVAL_METRICS_DEFAULT;
        QStringList intervalMetrics = s.split(",");
        summary += "<p><h3>"+tr("Intervals")+"</h3>\n<p>\n";
        summary += "<table align=\"center\" width=\"90%\" ";
        summary += "cellspacing=0 border=0>";
        bool even = false;
        foreach (RideFileInterval interval, ride->intervals()) {
            RideFile f(ride->startTime(), ride->recIntSecs());
            for (int i = ride->intervalBegin(interval); i < ride->dataPoints().size(); ++i) {
                const RideFilePoint *p = ride->dataPoints()[i];
                if (p->secs >= interval.stop)
                    break;
                f.appendPoint(p->secs, p->cad, p->hr, p->km, p->kph, p->nm,
                              p->watts, p->alt, p->lon, p->lat, p->headwind, p->slope, p->temp, 0);
            }
            if (f.dataPoints().size() == 0) {
                // Interval empty, do not compute any metrics
                continue;
            }

            QHash<QString,RideMetricPtr> metrics =
                RideMetric::computeMetrics(mainWindow, &f, mainWindow->zones(), mainWindow->hrZones(), intervalMetrics);
            if (firstRow) {
                summary += "<tr>";
                summary += "<td align=\"center\" valign=\"bottom\">Interval Name</td>";
                foreach (QString symbol, intervalMetrics) {
                    RideMetricPtr m = metrics.value(symbol);
                    if (!m) continue;
                    summary += "<td align=\"center\" valign=\"bottom\">" + m->name();
                    if (m->units(metricUnits) == "seconds")
                        ; // don't do anything
                    else if (m->units(metricUnits).size() > 0)
                        summary += " (" + m->units(metricUnits) + ")";
                    summary += "</td>";
                }
                summary += "</tr>";
                firstRow = false;
            }
            if (even)
                summary += "<tr>";
            else {
                QColor color = QApplication::palette().alternateBase().color();
                color = QColor::fromHsv(color.hue(), color.saturation() * 2, color.value());
                summary += "<tr bgcolor='" + color.name() + "'>";
            }
            even = !even;
            summary += "<td align=\"center\">" + interval.name + "</td>";
            foreach (QString symbol, intervalMetrics) {
                RideMetricPtr m = metrics.value(symbol);
                if (!m) continue;
                QString s("<td align=\"center\">%1</td>");
                if (m->units(metricUnits) == "seconds")
                    summary += s.arg(time_to_string(m->value(metricUnits)));
                else
                    summary += s.arg(m->value(metricUnits), 0, 'f', m->precision());
            }
            summary += "</tr>";
        }
        summary += "</table>";
    }

    if (!rideItem->errors().empty()) {
        summary += tr("<p><h2>Errors reading file:</h2><ul>");
        QStringListIterator i(rideItem->errors());
        while(i.hasNext())
            summary += " <li>" + i.next();
        summary += "</ul>";
    }
    summary += "<br><hr width=\"80%\">";

    // The extra <center> works around a bug in QT 4.3.1,
    // which will otherwise put the following above the <hr>.
    summary += "<br>BikeScore is a trademark of Dr. Philip "
        "Friere Skiba, PhysFarm Training Systems LLC";

    summary += "<br>TSS, NP and IF are trademarks of Peaksware LLC</center>";
    return summary;
}

