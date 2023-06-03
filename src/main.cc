#include "WikiRacer.h"
#include <iostream>

int main () {

    std::string start_page;
    std::string end_page;
    unsigned int links_per_page_limit = -1; // use -1 for all pages
    WikiRacer wikiRacer;

    /* Ask user for start page */
    std::cout << "Starting page: ";
    std::cin >> start_page;

    /* Ask user for end page */
    std::cout << "Ending page: ";
    std::cin >> end_page;
    std::cout << std::endl;

    std::vector<std::string> solution = wikiRacer.getWikiGameSolution(start_page, end_page, links_per_page_limit);

    return 0;
}
