#ifndef HTML_PARSER_HPP
#define HTML_PARSER_HPP

#include <gumbo.h>
#include <string>

namespace granada
{
    namespace utils
    {
        using HTML = const std::string;
        class HtmlParser
        {
        private:
            /* data */
            static void searchTitleElement(GumboNode *, std::string & title);
            static void searchDivByClass(GumboNode* node, const std::string& class_name, int& index, int target_index);

        public:
            static void getTitle(HTML &text, std::string & title);
            static void getDivContent(HTML &text, const std::string& class_name, int target_index);
        };

    } // namespace utils

} // namespace granada

#endif