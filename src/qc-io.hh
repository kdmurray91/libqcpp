/* Copyright (c) 2015-2016 Kevin Murray
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */


#ifndef QC_IO_HH
#define QC_IO_HH

#include "qc-config.hh"

#include <atomic>
#include <mutex>
#include <tuple>

namespace qcpp
{

class IOError : public std::runtime_error
{
public:
    explicit IOError            (const std::string &msg) :
        std::runtime_error(msg) {}

    explicit IOError            (const char        *msg) :
        std::runtime_error(msg) {}
};

class Read
{
public:
    std:: string        name;
    std:: string        sequence;
    std:: string        quality;

    Read                        ();

    Read                        (const std::string &name,
                                 const std::string &sequence,
                                 const std::string &quality);

    void
    clear                       ();

    size_t
    size                        () const;

    std::string
    str                         () const;

    void
    erase                       (size_t             pos=0);
    void
    erase                       (size_t             pos,
                                 size_t             count);
};

bool operator==(const Read &r1, const Read &r2);


class ReadPair: public std::pair<Read, Read>
{
public:
    ReadPair                    ();
    ReadPair                    (const std::string &name1,
                                 const std::string &sequence1,
                                 const std::string &quality1,
                                 const std::string &name2,
                                 const std::string &sequence2,
                                 const std::string &quality2);
    std::string
    str                         ();

    Read                first;
    Read                second;
};


// Declare wrappers from the source. We keep these in obfuscated structs to
// avoid having to install the SeqAn headers, or compile them in every source
// file.

struct SeqAnReadWrapper;
struct SeqAnWriteWrapper;


template<typename SeqAnWrapper>
class ReadIO
{
public:
    ReadIO                      ();
    ~ReadIO                     ();

    void
    open                        (const char        *filename);

    void
    open                        (const std::string &filename);

    size_t
    get_num_reads               ();

protected:

    SeqAnWrapper           *_private;
    std::atomic_ullong      _num_reads;
    bool                    _has_qual;
};

class ReadInputStream
{
public:
    ReadInputStream             ();
    ReadInputStream             (const ReadInputStream &other);

    virtual bool
    parse_read                  (Read              &the_read) = 0;

    virtual bool
    parse_read_pair             (ReadPair          &the_read_pair) = 0;

    bool
    at_end                      ();

protected:
    bool                    _at_end;
    // Locks paired IO to ensure proper pairing
    std::mutex              _pair_mutex;
};

class ReadParser: public ReadInputStream, public ReadIO<SeqAnReadWrapper>
{
public:
    bool
    parse_read                  (Read              &the_read);

    bool
    parse_read_pair             (ReadPair          &the_read_pair);

};

class ReadOutputStream
{
public:
    ReadOutputStream            ();
    ReadOutputStream            (const ReadOutputStream &other);
    void
    write_read                  (Read              &the_read);

    void
    write_read_pair             (ReadPair          &the_read_pair);

protected:
    // Locks paired IO to ensure proper pairing
    std::mutex              _pair_mutex;
};

class ReadWriter: public ReadOutputStream, public ReadIO<SeqAnWriteWrapper>
{
public:
    void
    close                       ();

    void
    write_read                  (Read              &the_read);

    void
    write_read_pair             (ReadPair          &the_read_pair);

};

class ReadInterleaver : public ReadInputStream
{

public:
    ReadInterleaver             ();

    void
    open                        (const char        *r1_filename,
                                 const char        *r2_filename);

    void
    open                        (const std::string &r1_filename,
                                 const std::string &r2_filename);
    bool
    parse_read_pair             (ReadPair          &the_read_pair);

    size_t
    get_num_reads               ();

    size_t
    get_num_pairs               ();

private:
    ReadParser          r1_parser;
    ReadParser          r2_parser;
    size_t              _num_pairs;
    std::mutex          _mutex;

    bool
    parse_read                  (Read              &the_read)
    {
        std::ignore = the_read;
        return false;
    }

};

class ReadDeInterleaver : public ReadOutputStream
{

public:
    ReadDeInterleaver           ();

    void
    open                        (const char        *r1_filename,
                                 const char        *r2_filename);

    void
    open                        (const std::string &r1_filename,
                                 const std::string &r2_filename);
    void
    write_read_pair             (ReadPair          &the_read_pair);

    size_t
    get_num_reads               ();

    size_t
    get_num_pairs               ();

private:
    ReadWriter          r1_writer;
    ReadWriter          r2_writer;
    size_t              _num_pairs;
    std::mutex          _mutex;

    void
    write_read                  (Read              &the_read)
    {
        std::ignore = the_read;
    }

};


} // namespace qcpp

#endif /* QC_IO_HH */
