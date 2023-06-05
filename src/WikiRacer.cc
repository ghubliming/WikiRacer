#include "WikiRacer.h"
#include "UrlRequest.h"

#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <utility>
#include <omp.h>

#include <string>
#include <vector>
#include <queue>
#include <unordered_set>
#include <unordered_map>

/* Uncomment to enable paralellization */
// #define PARALLELIZE_THREADS 4

WikiRacer::WikiRacer() {
    std::cout << R"(
    -----------------------------------------------------------------------------------------------

               __      __             _                                                          
         o O O \ \    / /   ___      | |      __       ___     _ __      ___                     
        o       \ \/\/ /   / -_)     | |     / _|     / _ \   | '  \    / -_)                    
       TS__[O]   \_/\_/    \___|    _|_|_    \__|_    \___/   |_|_|_|   \___|                    
      {======| _|"""""|  _|"""""| _|"""""| _|"""""| _|"""""| _|"""""| _|"""""|                   
     ./o--000' "`-0-0-'  "`-0-0-' "`-0-0-' "`-0-0-' "`-0-0-' "`-0-0-' "`-0-0-'                   
                  _                                                                              
         o O O   | |_      ___                                                                   
        o        |  _|    / _ \                                                                  
       TS__[O]   _\__|    \___/                                                                  
        {======| _|"""""| _|"""""|                                                                 
       ./o--000' "`-0-0-' "`-0-0-'                                                                 
               __      __    _       _         _       ___                                       
         o O O \ \    / /   (_)     | |__     (_)     | _ \    __ _      __       ___       _ _  
        o       \ \/\/ /    | |     | / /     | |     |   /   / _` |    / _|     / -_)     | '_| 
       TS__[O]   \_/\_/    _|_|_    |_\_\    _|_|_    |_|_\   \__,_|    \__|_    \___|    _|_|_  
      {======| _|"""""|  _|"""""| _|"""""| _|"""""| _|"""""| _|"""""| _|"""""| _|"""""| _|"""""| 
     ./o--000' "`-0-0-'  "`-0-0-' "`-0-0-' "`-0-0-' "`-0-0-' "`-0-0-' "`-0-0-' "`-0-0-' "`-0-0-' 


    -----------------------------------------------------------------------------------------------


    )" << "\n";
}


WikiRacer::~WikiRacer() {}

std::vector<std::string> WikiRacer::getWikiGameSolution(const std::string& start_page, const std::string& end_page) {
    std::cout << "Looking for a path between " << start_page << " and " << end_page << " ..." << std::endl;

    std::vector<std::string> solution_path = WikiRacer::getPriorityLadderSolution(start_page, end_page);
    
    /* Print out solution */
    std::cout << "Found solution: [";
    for (auto page_name : solution_path) {
        std::cout << page_name << ", ";
    }
    std::cout << "]" << std::endl;
    
    return solution_path;
}

std::unordered_set<std::string> WikiRacer::getAllLinksOnPage(const std::string& page_name) {
    
    // Grab page's html content
    UrlRequest urlRequest;
    const std::string page_html = urlRequest.getPageHTML(page_name);
    
    // All Wikipedia links start with this HTML pattern:
    const std::string wiki_link_start_pattern = "<a href=\"/wiki/";
    
    // All Wikipedia links end with this HTML pattern:
    const std::string wiki_link_end_pattern = "\"";
    
    std::unordered_set<std::string> page_names;
    auto start_pos = page_html.begin();

    // Crawl through the HTML and search for Wiki links
    while (true) {
        // find position in file where start pattern occurs
        start_pos = std::search(start_pos, page_html.end(), wiki_link_start_pattern.begin(), wiki_link_start_pattern.end());
        if (start_pos == page_html.end()) {
            break;
        }
        start_pos += wiki_link_start_pattern.size();
        auto end_pos = std::find(start_pos, page_html.end(), '\"');

        // Get name of linked page
        std::string page_name = std::string(start_pos, end_pos);

        // Ignore page names containing ":" and "#" as these are links to internal Wikipedia information
        if (page_name.find(":") == std::string::npos && page_name.find("#") == std::string::npos) {
            // Decode URL to plaintext to handle special characters
            page_name = urlRequest.decodeURL(page_name);
            page_names.insert(page_name);
        }
        start_pos = end_pos;
    }
    return page_names;
}

std::vector<std::string> WikiRacer::getPriorityLadderSolution(const std::string& start_page_name, const std::string& end_page_name) {

    /* Store all links on end page */
    const std::unordered_set<std::string> end_page_links = WikiRacer::getAllLinksOnPage(end_page_name);
    
    std::unordered_map<std::string, std::unordered_set<std::string>> visited_pages;
    std::vector<std::string> solution;
    
    std::string current_page = start_page_name;
    visited_pages[start_page_name] = WikiRacer::getAllLinksOnPage(start_page_name);

    /* Execute priotiy-ladder algorithm */
    while (true) {

        solution.push_back(current_page);
        std::cout << "\tVisiting page: " << current_page << "\n" << std::endl;

        /* 
        Set up priority queue.
        Items in the queue are pairs of (page_name, num_links_in_common_with_end_page)
        We define a custom comparator function for the queue to use. This function
        compares the second item in the pairs.
        */

        /* Comparator function for p-queue */
        auto cmp = [](const std::pair<std::string, int>& left, const std::pair<std::string, int>& right) { return (left.second < right.second); };

        /* Priority queue with custom comparator function */
        std::priority_queue<std::pair<std::string, int>, std::vector<std::pair<std::string, int>>, decltype(cmp)> queue(cmp);

        /* Process each candidate page that current page links to */
        std::unordered_set<std::string> current_page_links = visited_pages[current_page];

        /* If solution is in current page's links, we're done! */
        if (current_page_links.count(end_page_name)) {
            solution.push_back(end_page_name);
            break;
        }

        std::vector<std::string> current_pages(current_page_links.size());
        std::copy(current_page_links.begin(), current_page_links.end(), current_pages.begin());

#ifdef PARALLELIZE_THREADS
if (PARALLELIZE_THREADS > 4) {
    throw std::runtime_error("\n\nERROR: Please do not specify more than 4 threads with PARALLELIZE_THREADS. This will cause too many page requests per second.\n\n");
}
omp_set_num_threads(PARALLELIZE_THREADS);
#pragma omp parallel for
#endif
        for (size_t i = 0; i < current_pages.size(); ++i) {

            std::string candidate_page = current_pages[i];

            /* Process candidate page if it hasn't already been visited */
            if (!std::count(solution.begin(), solution.end(), candidate_page)) {

                /* Get all links on candidate page */
                std::unordered_set<std::string> candidate_page_links;

                if (visited_pages.find(candidate_page) == visited_pages.end()) {
                    /* Get links by reading HTML */
                    candidate_page_links = WikiRacer::getAllLinksOnPage(candidate_page);
                    
                    /* Stash links for use later */
                    visited_pages[candidate_page] = candidate_page_links;
                } else {
                    /* Use already stashed links*/
                    candidate_page_links = visited_pages[candidate_page];
                }
                
                /* Count number of links candidate page has in common with end page */
                int num_pages_in_common = WikiRacer::countNumCommonLinks(candidate_page_links, end_page_links);

                std::cout << "\t\tProcessed (" << queue.size()+1 << "\tof " << current_page_links.size() << ") [" << num_pages_in_common << "\tof " << end_page_links.size() << " pages similar]:\t" << candidate_page << std::endl;

                /* Push the candidate page to the priority queue */
                queue.push(std::make_pair(candidate_page, num_pages_in_common));
            }
        }

        /* Select the page with highest priority (highest numer of links in common with end page)*/
        current_page = queue.top().first;
    }
    return solution;
}

int WikiRacer::countNumCommonLinks(const std::unordered_set<std::string>& links_1, const std::unordered_set<std::string>& links_2) {
    int num = 0;
    for (std::string link : links_1) {
        if (links_2.count(link)) {
            num++;
        }
    }
    return num;
}
