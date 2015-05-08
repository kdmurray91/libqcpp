#include <iostream>
#include <string>
#include <thread>

#include "qcpp.hh"
#include "qc-adaptor.hh"
#include "qc-length.hh"

std::mutex _cout_mutex;

void parse_and_print(qcpp::ProcessedReadStream *stream)
{
    qcpp::ReadPair rp;
    bool not_at_end = stream->parse_read_pair(rp);
    while (not_at_end) {
        std::string rp_str = rp.str();
        {
            std::lock_guard<std::mutex> lg(_cout_mutex);
            std::cout << rp_str;
        }
        not_at_end = stream->parse_read_pair(rp);
    }
}

int
main (int argc, char *argv[])
{
    qcpp::ProcessedReadStream stream;
    std::vector<std::thread> threads;
    unsigned int n_threads = std::thread::hardware_concurrency();

    if (argc != 2) {
        std::cerr << "USAGE: " << argv[0] << " <read_file>" << std::endl;
        return EXIT_FAILURE;
    }

    stream.open(argv[1]);
    stream.append_processor<qcpp::ReadLenCounter>("before qc");
    stream.append_processor<qcpp::AdaptorTrimPE>("trim/merge reads", 5);
    stream.append_processor<qcpp::ReadLenCounter>("after Adaptor removal");
    stream.append_processor<qcpp::ReadLenFilter>("trim at 50", 50);
    stream.append_processor<qcpp::ReadLenCounter>("after qc");


    for (size_t i = 0; i < n_threads; i++) {
        threads.push_back(std::thread(parse_and_print, &stream));
    }
    for (size_t i = 0; i < n_threads; i++) {
        threads[i].join();
    }
    std::cerr << stream.report();
    return EXIT_SUCCESS;
}
