#ifndef URLREQUEST_H
#define URLREQUEST_H

#include <memory>
#include <string>
#include "curl/curl.h"

/**
This is an RAII wrapper around some basic CURL functionality to query webpages.
Followed this great example here: https://gist.github.com/Jacajack/af22073e723aeadb76fd5ab3814a0938
*/
class UrlRequest {
    
    public:
        UrlRequest();

        /**
        Gets the raw HTML of a wikipedia page given its page name.
        Great example here: https://gist.github.com/alghanmi/c5d7b761b2c9ab199157

        @param page_name The name of the Wiki page. Must be the name given by the end of the URL of the page.
        */
        std::string getPageHTML(const std::string page_name);

        /** 
        Converts a URL-encoded string to plain-text string to handle special characters
        Example: "Simon_St%C3%A5lenhag" -> "Simon_Stålenhag"

        @param url_string URL encoded string to decode.
        */
        std::string decodeURL(const std::string& url_string);

    private:

        /**
        Callback used to write HTML page contents to a string.
        */
        static size_t _writeCallback(void *contents, size_t size, size_t nmemb, void *userp);

        /**
        Smart pointer to Curl object for performing webpage queries.
        */
        std::unique_ptr<CURL, void(*)(CURL*)> _curl;
};

#endif // URLREQUEST_H