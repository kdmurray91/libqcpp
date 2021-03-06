# Copyright 2015 Kevin Murray <spam@kdmurray.id.au>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

from __future__ import absolute_import, division, print_function
import base64
from cStringIO import StringIO
import jinja2
import json
import matplotlib as mpl
import matplotlib.pyplot as plt
import mpld3
import numpy as np
import yaml
from sys import stderr

from qcpp.util import (
    pothole2title,
    nice_params,
    QCPP_ENV,
    MPLD3,
    D3,
)


class PlotResult(object):

    def render(self, report):
        """Renders a report with simple key:value params and output"""
        name = report['name']
        params = nice_params(report['parameters'])
        output = nice_params(report['output'])

        return self.template.render(name=name,
                                    parameters=params,
                                    output=output)


class PlotPerBaseQuality(PlotResult):
    """Render PerBaseQuality results"""

    percentiles = [0.05, 0.25, 0.5, 0.75, 0.95]
    ranges = [50, 90]

    def _plot(self, qual_array, name):
        # array of read numbers per base
        n_reads = [sum(base.values()) for base in qual_array]
        percentile_vals = {p: [] for p in self.percentiles}
        # for each base
        for i in range(len(n_reads)):
            for percentile in self.percentiles:
                # the idx'th read is this percentile's value
                idx = int(n_reads[i] * percentile)
                # find the quality score of the idx'th read
                up_to = 0
                for qual, count in qual_array[i].items():
                    up_to += count
                    if idx < up_to:
                        percentile_vals[percentile].append(qual)
                        break

        # create np arrays from each percentile vector
        lower_p, lower_q, med, upper_q, upper_p = [
            np.array(percentile_vals[p]) for p in self.percentiles
        ]
        # read position is 1-based here, for user friendlyness
        read_pos = np.arange(len(med)) + 1

        # Make figure
        fig, ax = plt.subplots(figsize=(10,6))

        # Plot median, save line for the legend
        line, = ax.plot(read_pos,
                        med,
                        color='black',
                        label="median")
        # Plot percentile range
        ax.fill_between(read_pos,
                        lower_p, upper_p,
                        alpha=0.1,
                        edgecolor='black',
                        facecolor='blue',
                        label="{}%-ile range".format(self.ranges[1]))
        # Plot quartile range
        ax.fill_between(read_pos,
                        lower_q, upper_q,
                        alpha=0.7,
                        edgecolor='black',
                        facecolor='lightgreen',
                        label="{}%-ile range".format(self.ranges[0]))

        # Set plot limits
        ax.set_xlim((1, len(read_pos)))
        ax.set_ylim((1, upper_p.max() + 2))

        # set titles
        ax.set_title("Per-base PHRED Qualities: " + name)
        ax.set_ylabel("PHRED score", size='large', labelpad=10)
        ax.set_xlabel("Read Position", size='large', labelpad=10)
        ax.set_xticks(np.arange(0, max(read_pos), 10))

        # ax.grid(axis='x')

        # generate legend patches for the ranges
        ipr = mpl.patches.Patch(color='blue',
                                label="{}%-ile range".format(self.ranges[1]),
                                alpha=0.1)
        iqr = mpl.patches.Patch(color='lightgreen',
                                label="{}%-ile range".format(self.ranges[0]),
                                alpha=0.7)

        # Make legend
        ax.legend(loc='lower left', handles=[line, iqr, ipr], framealpha=1)

        # Save and base64 the image
        sio = StringIO()
        fig.savefig(sio, format='png', dpi=60)
        return (base64.b64encode(sio.getvalue()),
                json.dumps(mpld3.fig_to_dict(fig)))

    def render(self, report):
        name = report['name']
        params = nice_params(report['parameters'])

        output = report['output']

        # Detect paired-end mode
        paired = False
        if output["r2_phred_scores"]:
            paired = True

        # Create images
        r1_name = name
        if paired:
            r1_name += " (R1)"
        r1_image, r1_dict = self._plot(output['r1_phred_scores'], r1_name)
        r2_image = None
        if paired:
            r2_name = name + " (R2)"
            r2_image, r2_dict = self._plot(output['r2_phred_scores'], r2_name)

        template = QCPP_ENV.get_template('perbasequality.html')
        return template.render(name=name,
                               parameters=params,
                               paired=paired,
                               r1_image=r1_image,
                               r1_dict=r1_dict,
                               r2_image=r2_image,
                               r2_dict=r2_dict)


class PlotWindowedQualTrim(PlotResult):
    """Render WindowedQualTrim results.
    We can use the default render function."""
    template = QCPP_ENV.get_template('windowedqualtrim.html')


class PlotAdaptorTrimPE(PlotResult):
    """Render AdaptorTrimPE results.
    We can use the default render function."""
    template = QCPP_ENV.get_template('adaptortrim.html')


def render_metadata(section):
    metadata = section['QCPP']
    template = QCPP_ENV.get_template('metadata.html')
    return template.render(metadata=metadata)


RENDERERS = {
    "PerBaseQuality": PlotPerBaseQuality,
    "AdaptorTrimPE": PlotAdaptorTrimPE,
    "WindowedQualTrim": PlotWindowedQualTrim,
}

def render_all(yml_file):
    with open(yml_file) as fh:
        reports = yaml.load(fh)

    report_names = []
    out_reports = []

    # Save the root metatdata report (always the first report in the YAML).
    metadata = render_metadata(reports[0])


    for report in reports[1:]:
        processor, proc_report = report.items()[0]
        renderer = RENDERERS[processor]()
        name = proc_report['name']
        div = renderer.render(proc_report)
        report_names.append(name)
        out_reports.append((processor, name, div))

    template = QCPP_ENV.get_template('root.html')
    html = template.render(metadata_div=metadata,
                           reports=out_reports,
                           report_names=report_names,
                           mpld3=MPLD3,
                           d3=D3)
    print(html)
