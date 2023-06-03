#include "UrlRequest.h"
#include "curl/curl.h"

#include <cstddef>
#include <iostream>


UrlRequest::UrlRequest() : _curl(curl_easy_init(), curl_easy_cleanup) {
    // Note: intializer above also defines the destructor for the smart pointer with the second argument
    if (!_curl) {
        throw std::runtime_error("ERROR initializing CURL");
    }
}

std::string UrlRequest::getPageHTML(const std::string page_name) {
    CURLcode result;
    std::string page_url = "https://en.wikipedia.org/wiki/" + page_name;
    std::string read_buffer;

    /* Set options */
    curl_easy_setopt(_curl.get(), CURLOPT_URL, page_url.c_str());
    curl_easy_setopt(_curl.get(), CURLOPT_WRITEFUNCTION, UrlRequest::_writeCallback);
    curl_easy_setopt(_curl.get(), CURLOPT_WRITEDATA, &read_buffer);

    /* Perform the request */
    result = curl_easy_perform(_curl.get());

    /* Check for errors */
    if(result != CURLE_OK || read_buffer == "") {
        throw std::runtime_error("\n\n\nERROR: page '" + page_name + "' not found!\nPlease enter a valid Wikipedia page name, which is found at the end of the page's URL.\n\tExample: 'Kalman_filter' (URL: https://en.wikipedia.org/wiki/Kalman_filter)\n\tExample: 'Simon_St%C3%A5lenhag' (URL: https://en.wikipedia.org/wiki/Simon_St%C3%A5lenhag)\n\n\n");
    }
    
    return read_buffer;
}

std::string UrlRequest::decodeURL(const std::string& url_string) {
    int decodelen;
    char *decoded = curl_easy_unescape(_curl.get(), url_string.c_str(), 0, &decodelen);
    if(decoded) {
        std::string s(decoded);
        curl_free(decoded);
        // std::cout << "ENCODED: " << url_string << "\tDECODED: " << s << std::endl;
        return s;
    }
    return "";
}

size_t UrlRequest::_writeCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}
