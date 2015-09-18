/*
 * ============================================================================
 *
 *       Filename:  qc-qualtrim.hh
 *    Description:  Trim low quality sequences via various methods
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

#ifndef QC_QUALTRIM_HH
#define QC_QUALTRIM_HH

#include "qc-config.hh"
#include "qc-processor.hh"
#include "qc-quality.hh"

namespace qcpp
{


class WindowedQualTrim: public ReadProcessor
{
    // This class implements a similar algorithm to Nik Joshi's sickle tool
public:
    WindowedQualTrim                (const std::string &name,
                                     const QualityEncoding &encoding,
                                     int8_t             min_quality,
                                     size_t             min_length,
                                     size_t             window_size);
    WindowedQualTrim                (const std::string &name,
                                     const QualityEncoding &encoding,
                                     int8_t             min_quality,
                                     size_t             min_length);

    void
    process_read                    (Read              &the_read);

    void
    process_read_pair               (ReadPair          &the_read_pair);

    std::string
    yaml_report                          ();

private:
    int8_t                  _min_quality;
    size_t                  _min_length;
    size_t                  _window_size;
    size_t                  _num_reads_trimmed;
    size_t                  _num_reads_dropped;
};


} // namespace qcpp


#endif /* QC_QUALTRIM_HH */
