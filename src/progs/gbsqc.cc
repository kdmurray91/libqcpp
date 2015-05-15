/*
 * ============================================================================
 *
 *       Filename:  gbsqc.cc
 *    Description:  A GBS quality control pipeline
 *        License:  LGPL-3+
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

/* Copyright (c) 2015 Kevin Murray
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>

#include <getopt.h>

#include "qcpp.hh"

#include "qc-measure.hh"
#include "qc-length.hh"
#include "qc-qualtrim.hh"
#include "qc-gbs.hh"

using std::chrono::system_clock;

inline void
progress(size_t n, system_clock::time_point start)
{
    std::chrono::duration<double> tdiff = system_clock::now() - start;
    double secs = tdiff.count();
    double k_reads = n / 1000.0;
    double rate = k_reads / secs;
    std::cerr << "\x1b[2K" << "Kept " << k_reads << "K read pairs in "
              << (int)secs << "s (" << std::setprecision(3) << rate
              << std::setprecision(9) << "K RP/sec)\r";
}

int
usage_err()
{
    std::cerr << "USAGE: gbsqc [-y REPORT -o OUTPUT] <read_file>" << std::endl;
    return EXIT_FAILURE;
}

const char *cli_opts = "y:o:";

int
main (int argc, char *argv[])
{
    std::string                 yaml_fname;
    bool                        use_stdout = true;
    std::ofstream               read_output;

    int c = 0;
    while ((c = getopt(argc, argv, cli_opts)) > 0) {
        switch (c) {
            case 'y':
                yaml_fname = optarg;
                break;
            case 'o':
                read_output.open(optarg);
                use_stdout = false;
                break;
            default:
                std::cerr << "Bad arg '" << std::string(1, optopt) << "'"
                          << std::endl << std::endl;
                return usage_err();
        }
    }

    if (optind + 1 > argc) {
        std::cerr << "Must provide filename" << std::endl << std::endl;
        return usage_err();
    }

    qcpp::ReadPair              rp;
    qcpp::ProcessedReadStream   stream(argv[optind]);
    uint64_t                    n_pairs = 0;

    //stream.append_processor<qcpp::PerBaseQuality>("before qc");
    stream.append_processor<qcpp::GBSTrimPE>("trim Pst1 read-through", "CTGCAG", 1);
    stream.append_processor<qcpp::WindowedQualTrim>("QC", 28, 33, 50);
    //stream.append_processor<qcpp::PerBaseQuality>("after qc");

    system_clock::time_point start = system_clock::now();
    while (stream.parse_read_pair(rp)) {
        if (n_pairs % 100000 == 0) {
            progress(n_pairs, start);
        }
        n_pairs++;
        if (use_stdout) {
            std::cout << rp.str();
        } else {
            read_output << rp.str();
        }
    }
    progress(n_pairs, start);
    std::cerr << std::endl;
    if (yaml_fname.size() > 0) {
        std::ofstream yml_output(yaml_fname);
        yml_output << stream.report();
    }
    return EXIT_SUCCESS;
}