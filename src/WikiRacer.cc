#include "WikiRacer.h"
#include <cstddef>
#include <iostream>
#include <algorithm>
#include <string>
#include <utility>
#include <vector>
#include <unordered_set>
#include <queue>
#include <unordered_map>
#include "curl/curl.h"


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


WikiRacer::~WikiRacer() {
    
}

std::vector<std::string> WikiRacer::getWikiGameSolution(const std::string& start_page, const std::string& end_page, unsigned int links_per_page_limit) {
    std::cout << "Looking for a path between " << start_page << " and " << end_page << " ..." << std::endl;

    std::vector<std::string> solution_path = WikiRacer::getPriorityLadderSolution(start_page, end_page, links_per_page_limit);
    
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
    const std::string page_html = WikiRacer::getPageHTML(page_name);
    
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
            page_names.insert(page_name);
        }
        start_pos = end_pos;
    }
    return page_names;
}

std::vector<std::string> WikiRacer::getPriorityLadderSolution(const std::string& start_page_name, const std::string& end_page_name, const unsigned int links_per_page_limit) {

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

        size_t counter = 1;
        for (std::string candidate_page : current_page_links) {

            if (links_per_page_limit >= 0 && counter >= links_per_page_limit) {
                break;
            }

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

                std::cout << "\t\tProcessed (" << counter << " of " << current_page_links.size() << ") [" << num_pages_in_common << " of " << end_page_links.size() << " pages similar]:\t\t" << candidate_page << std::endl;
                
                /* Push the candidate page to the priority queue */
                queue.push(std::make_pair(candidate_page, num_pages_in_common));

                counter++;
            }
        }

        /* Select the page with highest priority (highest numer of links in common with end page)*/
        current_page = queue.top().first;
    }
    return solution;
}

std::string WikiRacer::getPageHTML(const std::string page_name) {
    CURL* curl;
    CURLcode result;
    std::string page_url = "https://en.wikipedia.org/wiki/" + page_name;
    std::string read_buffer;

    curl = curl_easy_init();
    if(curl) {
        /* Set options */
        curl_easy_setopt(curl, CURLOPT_URL, page_url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WikiRacer::_writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);

        /* Perform the request */
        result = curl_easy_perform(curl);

        /* Perform cleanup */
        curl_easy_cleanup(curl);

        /* Check for errors */
        if(result != CURLE_OK || read_buffer == "") {
            throw std::runtime_error("\n\n\nERROR: page '" + page_name + "' not found!\nPlease enter a valid Wikipedia page name, which is found at the end of the page's URL.\n\tExample: 'Kalman_filter' (URL: https://en.wikipedia.org/wiki/Kalman_filter)\n\tExample: 'Simon_St%C3%A5lenhag' (URL: https://en.wikipedia.org/wiki/Simon_St%C3%A5lenhag)\n\n\n");
        }
        
        return read_buffer;
    } 
    return "";
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

size_t WikiRacer::_writeCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

