#ifndef WIKIRACER_H
#define WIKIRACER_H

#include <string>
#include <vector>
#include <unordered_set>


class WikiRacer {

    public:
        WikiRacer();
        ~WikiRacer();

        /**
        Finds a solution to the WikiGame for the start and end pages specified. 
        A valid solution is a path from the start Wiki page to the end page using only Wikipedia links.

        @param start_page The name of the Wiki page to start at. Must be the name given by the end of the URL of the page.
                          Example: "Kalman_filter" (URL: https://en.wikipedia.org/wiki/Kalman_filter)
                          Example: "Simon_St%C3%A5lenhag" (URL: https://en.wikipedia.org/wiki/Simon_St%C3%A5lenhag)
        @param end_page The name of the Wiki page to end at.
        */
        std::vector<std::string> getWikiGameSolution(const std::string& start_page, const std::string& end_page);

    private:

        /**
        Gets a set of all wikipedia pages linked to by the given page.

        @param page_name The name of the Wiki page. Must be the name given by the end of the URL of the page.
        */
        std::unordered_set<std::string> getAllLinksOnPage(const std::string& page_name);

        /**
        Finds a path from the start to end page using the PriorityLadder algorithm.
        
        @param start_page_name The name of the Wiki page to start at. Must be the name given by the end of the URL of the page.
        @param end_page_name The name of the Wiki page to end at. Must be the name given by the end of the URL of the page.
        */
        std::vector<std::string> getPriorityLadderSolution(const std::string& start_page_name, const std::string& end_page_name);
        
        /**
        Counts the numeber of Wikipedia links two pages have in common.

        @param links_1 a set containing all links from page 1.
        @param links_2 a set containing all links from page 2.
        */
        int countNumCommonLinks(const std::unordered_set<std::string>& links_1, const std::unordered_set<std::string>& links_2);
};

#endif // WIKIRACER_H

